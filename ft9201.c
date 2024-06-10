#include <linux/module.h>
#include <linux/printk.h>
#include <linux/usb.h>

#include "ft9201.h"

MODULE_AUTHOR("Ben Maddocks <bm16ton@gmail.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("FT9201 Fingeprint reader driver");

#define VENDOR_ID 0x2808
#define PRODUCT_ID 0x9338

static struct usb_device_id ft9201_table[] = {
		{USB_DEVICE(VENDOR_ID, PRODUCT_ID)},
		{}
};
MODULE_DEVICE_TABLE(usb, ft9201_table);

#define FT9201_MINOR_BASE	192

#define WRITES_IN_FLIGHT	8

#define USB_CONTROL_OP_TIMEOUT 1000
#define USB_READ_OP_TIMEOUT 1000

struct ft9201_device {
	struct usb_device *udev;
	struct usb_interface *interface;
	struct semaphore	limit_sem;		/* limiting the number of writes in progress */
	__u8			bulk_in_endpointAddr;	/* the address of the bulk in endpoint */
	int			errors;			/* the last request tanked */
	spinlock_t		err_lock;		/* lock for errors */
	struct kref		kref;
	struct mutex		io_mutex;		/* synchronize I/O with disconnect */
	unsigned long		disconnected:1;
	wait_queue_head_t	bulk_in_wait;		/* to wait for an ongoing read */

	struct ft9201_status device_status;

	bool            ongoing_read;           /* a read is going on */
	unsigned char   *read_img_data;
	size_t			img_in_size;		/* the size of the receive buffer */
	size_t			img_in_filled;		/* number of bytes in the buffer */
	size_t			img_in_copied;		/* already copied to user space */
	bool			timetoexit;

};
#define to_ft9201_dev(d) container_of(d, struct ft9201_device, kref)

static int ft9201_open(struct inode *inode, struct file *file);
static int ft9201_release(struct inode *inode, struct file *file);
static long ft9201_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t ft9201_read(struct file *fp, char __user *buf, size_t count, loff_t *f_pos);
static int ft9201_initialize(struct ft9201_device *dev);
static int ft9201_ic_sensor_mode_exit(struct ft9201_device *dev);
static int ft9201_read_image(struct ft9201_device *dev);

static void ft9201_delete(struct kref *kref);

static struct usb_driver ft9201_driver;

static const struct file_operations ft9201_fops = {
		.owner =   THIS_MODULE,
		.llseek =  noop_llseek,
		.open =    ft9201_open,
		.release = ft9201_release,
		.unlocked_ioctl = ft9201_ioctl,
		.read =    ft9201_read,
};

/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static struct usb_class_driver ft9201_class = {
		.name =	        "fpreader%d",
		.fops =	        &ft9201_fops,
		.minor_base =   FT9201_MINOR_BASE,
};

// IN Requests
#define FT9201_REQ_READ_REGISTERS 0x43
#define FT9201_REQ_GET_SUI_VERSION 0x1a

// OUT Requests
#define FT9201_REQ_START_CAPTURE_PROBABLY 0x34
#define FT9201_REQ_CONFIGURE_BULK_TRANSFER_SIZE_PROBABLY 0x35
#define FT9201_REQ_WRITE_REGISTER 0x3b

#define FT9201_REG_MCU_SENSOR_STATUS_INDEX 0x20

#define FT9201_AFE_0X30_SUCCESSFUL_RESPONSE 0xbb

static long ft9201_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long errCode;
	struct ft9201_device *dev = file->private_data;

	pr_info("ft9201 ioctl, cmd: %u\n", cmd);

	errCode = 0;

	switch (cmd) {
		case FT9201_IOCTL_REQ_INITIALIZE:
			errCode = ft9201_initialize(dev);
			if (errCode < 0) {
				dev_err(&dev->interface->dev, "Error initializing device: %ld", errCode);
				return errCode;
			}
			break;

		default:
			return -EINVAL;
	}

	return 0;
}

static int ft9201_open(struct inode *inode, struct file *file)
{
	struct usb_interface *intf;
	struct ft9201_device *dev;
	
	pr_info("ft9201 open\n");

	intf = usb_find_interface(&ft9201_driver, iminor(inode));
	if (!intf) {
		pr_err("Can't find device for minor %d\n", iminor(inode));
		return -ENODEV;
	}

	dev = usb_get_intfdata(intf);
	if (!dev) {
		return -ENODEV;
	}

	dev->timetoexit = false;
	dev->img_in_copied = 0;
	dev->img_in_filled = 0;
	kref_get(&dev->kref);

	file->private_data = dev;

	return 0;
}

static int ft9201_release(struct inode *inode, struct file *file)
{
	struct ft9201_device *dev = file->private_data;
	pr_info("ft9201 release\n");

	if (dev == NULL) {
		return -ENODEV;
	}

	kref_put(&dev->kref, ft9201_delete);

	return 0;
}

static int ft9201_initialize(struct ft9201_device *dev)
{
	int errCode = 0;
	unsigned char sensor_status = 1;

	dev_info(&dev->interface->dev, "ioctl initialize");

	if (sensor_status != 1) {
		ft9201_ic_sensor_mode_exit(dev);
	}

	dev->device_status.sensor_height = 0x40;
	dev->device_status.sensor_width = 0x50;

	dev_info(&dev->interface->dev, "Image dimensions: %d x %d", dev->device_status.sensor_width, dev->device_status.sensor_height);

	if (errCode == 0) {
		dev->device_status.initialized = 1;
		dev_info(&dev->interface->dev, "Device initialization successful");
	}

	return errCode;
}

static int ft9201_read_image(struct ft9201_device *dev)
{
	int retVal;
	int img_size = 64 * 80;
	int transfer_size = img_size;
	int read_length;
	unsigned char *img_with_header;

	dev->device_status.sensor_width = 0x50;
	dev->device_status.sensor_height = 0x40;
	dev_info(&dev->interface->dev, "Reading image from scanner; dimensions: %dx%d", dev->device_status.sensor_width, dev->device_status.sensor_height);

	dev->img_in_copied = 0;
	dev->img_in_filled = 0;

	if (dev->read_img_data != NULL) {
		kfree(dev->read_img_data);
		dev->img_in_size = 0;
	}

	dev->read_img_data = kzalloc(img_size, GFP_KERNEL);
	if (dev->read_img_data == NULL) {
		return -ENOMEM;
	}
	dev->img_in_size = img_size;

	img_with_header = kzalloc(transfer_size, GFP_KERNEL);
	if (img_with_header == NULL) {
		retVal = -ENOMEM;
		goto out;
	}

	retVal = usb_bulk_msg(
			dev->udev,
			usb_rcvbulkpipe(dev->udev, dev->bulk_in_endpointAddr),
			img_with_header,
			transfer_size,
			&read_length,
			USB_READ_OP_TIMEOUT
	);
	if (retVal < 0) {
		dev_err(&dev->interface->dev, "Error reading data from device: Error %d", retVal);
		goto out;
	}
	dev_info(&dev->interface->dev, "Received %d bytes from device", read_length);
	if (read_length != transfer_size) {
		dev_err(&dev->interface->dev, "Read less than image size");
		retVal = -EINVAL;
		goto out;
	}

	// Ignore added 2 bytes at the start of data
	memcpy(dev->read_img_data, img_with_header, img_size);
	dev->img_in_filled = img_size;


out:
	if (img_with_header != NULL) {
		kfree(img_with_header);
	}

	return retVal;
}

static int ft9201_ic_sensor_mode_exit(struct ft9201_device *dev)
{
	int errCode = 0;
	return errCode;
}

static DECLARE_WAIT_QUEUE_HEAD(ft9201_wq);

static int has_data_remaining(struct ft9201_device *dev)
{
	dev_info(&dev->interface->dev, "Copied: %lu, Filled: %lu", dev->img_in_copied, dev->img_in_filled);
	if ((dev->img_in_copied == 5120) && (dev->img_in_filled == 5120)) {
		dev->timetoexit = true;
	}
	return dev->img_in_copied < dev->img_in_filled;
}

static ssize_t send_read_data(struct ft9201_device *dev, char __user *buf, size_t count)
{
	size_t remaining = dev->img_in_filled - dev->img_in_copied;
	size_t to_copy = remaining;
	if (to_copy > count) {
		to_copy = count;
	}
	dev_info(&dev->interface->dev, "Copied: %lu, to_copy: %lu, full_size: %lu", dev->img_in_copied, to_copy, dev->img_in_size);

	if (dev->img_in_copied + to_copy > dev->img_in_size) {
		return -EINVAL;
	}

	if (copy_to_user(buf, dev->read_img_data + dev->img_in_copied, to_copy)) {
		return -EFAULT;
	}
	dev->img_in_copied += to_copy;
	dev_info(&dev->interface->dev, "Copied total: %lu", dev->img_in_copied);
	return (ssize_t)to_copy;
}

static ssize_t ft9201_read(struct file *fp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct ft9201_device *dev = fp->private_data;
//	struct inode *inode = fp->f_path.dentry->d_inode;

	unsigned long loop_timeout = msecs_to_jiffies(1000);
	int ret;

	if (dev == NULL) {
		pr_err("device is null\n");
		return 0;
	}

	ret = mutex_lock_interruptible(&dev->io_mutex);
	if (ret < 0) {
		pr_info("Interrupted while waiting on IO mutex");
		return ret;
	}

	if (dev->disconnected) {
		ret = -ENODEV;
		goto exit;
	}

int ret2;

	while (true) {
		if (has_data_remaining(dev)) {
			ret = send_read_data(dev, buf, count);
			ret2 = ret;
			break;
		}
/*
		if (ret2 == 5120) {
			mutex_unlock(&dev->io_mutex);
			kfree(dev->read_img_data);
			dev->read_img_data = NULL;
	        ft9201_release(inode, fp);
			break;
		}
*/
//		pr_info("ft9201 reading %d\n", i);


		ret = wait_event_interruptible_timeout(ft9201_wq, 0, loop_timeout);
		if ((ret == -ERESTARTSYS) || (dev->timetoexit == true)){
			// We were interrupted by a signal
			dev->timetoexit = false;
			mutex_unlock(&dev->io_mutex);
			pr_info("Done reading fingerprint closing");
			ret = 0;
			break;
		}

		int retval;
		int poo = 0;
unsigned char local_value[4];


	retval = usb_control_msg_send(
			dev->udev,
			0,
			52,
			0x40,
			0x0003,
			0,
			NULL,
			0,
			5000,
			GFP_KERNEL);

	if (retval) {
		dev_info(&dev->interface->dev, "Error sending control data 1: %d\n", retval);
		return retval;
	}
	retval = usb_control_msg_send(
			dev->udev,
			0,
			111,
			0x40,
			0x0020,
			37248,
			NULL,
			0,
			5000,
			GFP_KERNEL);

	if (retval) {
		dev_info(&dev->interface->dev, "Error sending control data 2: %d\n", retval);
		return retval;
	}

	retval = usb_control_msg_send(
			dev->udev,
			0,
			111,
			0x40,
			0x1400,
			36992,
			NULL,
			0,
			5000,
			GFP_KERNEL);

	if (retval) {
		dev_info(&dev->interface->dev, "Error sending control data 3: %d\n", retval);
		return retval;
	}

	while(poo == 0) {
	retval = usb_control_msg_recv(
			dev->udev,
			0,
			FT9201_REQ_READ_REGISTERS,
			USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			0,
			0,
			&local_value,
			sizeof(local_value),
			5000,
			GFP_KERNEL);

	if (retval) {
		dev_info(&dev->interface->dev, "Error sending data: %d\n", retval);
		return retval;
		}

//	dev_info(&dev->interface->dev, "read registers returned: %d %d %d %d\n", local_value[0], local_value[1], local_value[2], local_value[3]);

	poo = local_value[0];
	}

	ret = ft9201_read_image(dev);
		if (ret < 0) {
			break;
		}

	dev_info(&dev->interface->dev, "ft9201_read copied : %d\n", ret);

/*
	if (has_data_remaining(dev)) {
		retval = send_read_data(dev, buf, count);
//		break;
	}
*/
//	if (dev->timetoexit == true) {
//		mutex_unlock(&dev->io_mutex);
//		kfree(dev->read_img_data);
//		dev->read_img_data = NULL;
//        ft9201_release(inode, fp);
//		return 0;
//		}

	}

exit:
	mutex_unlock(&dev->io_mutex);
	return ret;
}

static void ft9201_delete(struct kref *kref)
{
	struct ft9201_device *dev = to_ft9201_dev(kref);

	usb_put_intf(dev->interface);
	usb_put_dev(dev->udev);
	if (dev->read_img_data != NULL) {
		kfree(dev->read_img_data);
		dev->read_img_data = NULL;
	}
	kfree(dev);
}

static int ft9201_probe(struct usb_interface *intf, const struct usb_device_id *id) {
	struct usb_device *udev = interface_to_usbdev(intf);
	struct ft9201_device *dev;
	struct usb_endpoint_descriptor *bulk_in, *bulk_out;

	int retval;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		return -ENOMEM;
	}

	kref_init(&dev->kref);
	sema_init(&dev->limit_sem, WRITES_IN_FLIGHT);
	spin_lock_init(&dev->err_lock);
	init_waitqueue_head(&dev->bulk_in_wait);

	dev->udev = usb_get_dev(udev);
	dev->interface = usb_get_intf(intf);

	// bulk_out endpoint is not yet used. It might be used only for upgrading firmware if it's ever implemented
	/* use only the first bulk-in and bulk-out endpoints */
	retval = usb_find_common_endpoints(intf->cur_altsetting, &bulk_in, &bulk_out, NULL, NULL);
	if (retval) {
		dev_err(&intf->dev, "Could not find both bulk-in and bulk-out endpoints\n");
		goto error;
	}

	dev->bulk_in_endpointAddr = bulk_in->bEndpointAddress;

	/* save our data pointer in this interface device */
	usb_set_intfdata(intf, dev);

	/* we can register the device now, as it is ready */
	retval = usb_register_dev(intf, &ft9201_class);
	if (retval) {
		/* something prevented us from registering this driver */
		dev_err(&intf->dev,
				"Not able to get a minor for this device.\n");
		usb_set_intfdata(intf, NULL);
		goto error;
	}

	/* let the user know what node this device is now attached to */
	dev_info(&intf->dev, "USB fpreader device now attached to fpreader%d", intf->minor);

	retval = ft9201_initialize(dev);
	if (retval < 0) {
		dev_err(&dev->interface->dev, "Error initializing device: %d", retval);
	}

	return 0;

error:
	/* this frees allocated memory */
	kref_put(&dev->kref, ft9201_delete);

	return retval;
}

static int ft9201_pre_reset(struct usb_interface *interface) {
	pr_info("Pre reset\n");

	return 0;
}

static int ft9201_post_reset(struct usb_interface *interface) {
	pr_info("Post reset\n");

	return 0;
}

static void ft9201_disconnect(struct usb_interface *interface) {
	struct ft9201_device *dev;
	int minor = interface->minor;

	pr_info("Disconnect");

	dev = usb_get_intfdata(interface);
	pr_info("Disconnect");
	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &ft9201_class);

	/* prevent more I/O from starting */
	mutex_lock(&dev->io_mutex);
	dev->disconnected = 1;
	mutex_unlock(&dev->io_mutex);

	/* decrement our usage count */
	kref_put(&dev->kref, ft9201_delete);

	dev_info(&interface->dev, "USB device fpreader%d now disconnected", minor);
}

static int ft9201_suspend(struct usb_interface *intf, pm_message_t message) {
	pr_info("Suspend");

	return 0;
}

static int ft9201_resume(struct usb_interface *intf) {
	pr_info("Resume");

	return 0;
}

static struct usb_driver ft9201_driver = {
		.name = "ft9201",
		.probe = ft9201_probe,
		.disconnect = ft9201_disconnect,
		.suspend = ft9201_suspend,
		.resume = ft9201_resume,
		.pre_reset = ft9201_pre_reset,
		.post_reset = ft9201_post_reset,
		.id_table = ft9201_table,
};

module_usb_driver(ft9201_driver);
