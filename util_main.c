#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#include "ft9201.h"
#include <math.h>
#include <wand/magick_wand.h>

void raw_wand(void)
{
    MagickWand *mw = NULL;
	MagickWandGenesis();
	mw=NewMagickWand();
	MagickReadImage(mw,"img.raw");
char *data = (char*)malloc(sizeof(char) * 20480);
MagickConstituteImage(mw,80,64,"rgb",CharPixel,data);
//MagickImportImagePixels(mw,80,64,80,64,"I",CharPixel,data);
//MagickGetImagePixels(mw,80,64,80,64,"I",CharPixel,data);
//MagickQuantizeImage(mw,256,GRAYColorspace,0,MagickFalse,MagickFalse);
//MagickSetImageType(mw, GrayscaleType);
//MagickSetImageDepth(mw, 8);
//MagickSetImageColorspace(mw,GRAYColorspace);
//MagickTransformImageColorspace(mw, GRAYColorspace);
//MagickSetImageDepth(mw, 8);

MagickWriteImage(mw,"finger.png");
	if(mw)mw = DestroyMagickWand(mw);
	MagickWandTerminus();

}



int main(int argc, char *argv[]) {
	unsigned int action = 0;
	printf("FT9201 utility program\n");

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <device_file> [action]\n", argv[0]);
		return -1;
	}


	if (argc == 3) {
		action = (unsigned int) atoi(argv[2]);
	}

	printf("Action: %d\n", action);

	char *device_file_name = argv[1];
	struct ft9201_status device_status;

	printf("Device: %s\n", device_file_name);

	if (action == 0) {
		fprintf(stderr, "status struct: %p\n", &device_status);

		int result;

		printf("Initializing device\n");

	} else if (action == 1) {
		printf("Reading\n");
int fd;
char buff[5120];
    fd = open(device_file_name,O_RDONLY);
    if (fd != -1) {
      read(fd,buff,5120);
      close(fd);
	}
  int sdfd;
  sdfd = open("img.raw",O_WRONLY | O_CREAT);
  if (sdfd == -1) {
    printf("bad open\n");
    exit(-1);
  }
  write(sdfd,buff,(5120));
  close(sdfd);
    }
	raw_wand();

	return 0;
}
