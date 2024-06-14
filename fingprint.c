#include "lodepng.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

void encodeOneStep(const char* filename, const unsigned char* image, unsigned width, unsigned height) {
  /*Encode the image*/
  unsigned error = lodepng_encode_file(filename, image, width, height, LCT_GREY, 8);

  /*if there's an error, display it*/
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
}

int main(int argc, char *argv[]) {
  const char* filename = "test.png";
const char* device_file_name = argc > 1 ? argv[1] : "/dev/fpreader2";
  /*generate some image*/
  unsigned width = 64, height = 80;
  unsigned char* image = malloc(width * height * 8);
int fd;
char buff[5120];
    fd = open(device_file_name,O_RDONLY);
    if (fd != -1) {
      read(fd,buff,5120);
      close(fd);
	}
  int sdfd;
  sdfd = open("img.raw",O_WRONLY | O_CREAT, O_WRONLY);
  if (sdfd == -1) {
    printf("bad open\n");
    exit(-1);
  }
  write(sdfd,buff,(5120));
  memcpy(image, buff, 5120);
  close(sdfd);

  /*run an example*/
  encodeOneStep(filename, image, width, height);

  free(image);
  return 0;
}

