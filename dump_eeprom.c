#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#define TIMEOUT 100

#define VENDOR  0x1b80
#define PRODUCT 0xe600

#define SIZE 0x1000

typedef unsigned char u8;
typedef unsigned short u16;

struct libusb_device_handle *dev;

u16 get_0xa0(u16 index) {
	u8 temp = 0;
	u8 clk = 0x40;
	u16 n = (index << 8) | (index >> 8);
	u16 res = 0;

	libusb_control_transfer(dev, 0x40, 0, 0, 6, &clk, 1, TIMEOUT);
	libusb_control_transfer(dev, 0x40, 3, 0, 0xa0, (u8*)&n, 2, TIMEOUT);

	libusb_control_transfer(dev, 0xc0, 0, 0, 5, &temp, 1, TIMEOUT);
	libusb_control_transfer(dev, 0xc0, 2, 0, 0xa0, (u8*)&res, 2, TIMEOUT);
	libusb_control_transfer(dev, 0xc0, 0, 0, 5, &temp, 1, TIMEOUT);

	return res;
}

int main(int argc, char **argv) {
	if (libusb_init(NULL) < 0) {
		printf("libusb_init() failed\n");
		return 1;
	}

	dev = libusb_open_device_with_vid_pid(NULL, VENDOR, PRODUCT);
	if (!dev) {
		printf("Could not open USB with ID %04x:%04x\n", VENDOR, PRODUCT);
		return 2;
	}

	u16 *buf = malloc(SIZE);
	u16 *p = buf;

	for (int i = 0; i < SIZE; i += 2)
		*p++ = get_0xa0(i);

	FILE *f = fopen("dump.bin", "wb");
	fwrite(buf, 1, SIZE, f);
	fclose(f);

	free(buf);
	return 0;
}
