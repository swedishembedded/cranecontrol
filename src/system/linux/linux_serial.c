#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linux/linux_serial.h"
#include "serial.h"

#include <termios.h>
#include <memory.h>
#include <sys/fcntl.h>
#include <unistd.h>

struct linux_serial {
	struct termios oldtio;
	int fd;
	struct serial_device dev;
};

int linux_serial_connect(struct linux_serial *self, const char *serial_dev,
                             unsigned baud) {

	int fd = open(serial_dev, O_RDWR | O_NOCTTY);
	if(fd < 0) {
		perror("open");
		return -1;
	}

	tcgetattr(fd, &self->oldtio); /* save current serial port settings */

	struct termios tty;
	memset(&tty, 0, sizeof(tty)); /* clear struct for new port settings */

	cfsetospeed(&tty, (speed_t)B115200);
	cfsetispeed(&tty, (speed_t)B115200);

	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 1;

	// tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS; /* no HW flow control? */
	tty.c_cflag |= CLOCAL | CREAD;

	tty.c_iflag = IGNPAR;

	cfmakeraw(&tty);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &tty);

	self->fd = fd;

	return 0;
}

int linux_serial_disconnect(struct linux_serial *self) {
	if(self->fd < 0) {
		return -1;
	}
	tcsetattr(self->fd, TCSANOW, &self->oldtio);
	return close(self->fd);
}

int linux_serial_write(struct linux_serial *self, const void *data, size_t size, unsigned tout) {
	printf("linux_serial: write %ld\n", size);
	if(self->fd < 0) {
		return -1;
	}
	// TODO: implement timeout
	return (int)write(self->fd, data, size);
}

int linux_serial_read(struct linux_serial *self, void *data, size_t size,
                             unsigned timeout_us) {
	if(self->fd < 0) {
		return -1;
	}
	fd_set set;
	FD_ZERO(&set);
	FD_SET(self->fd, &set);
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timeout_us;
	int r = select(self->fd + 1, &set, NULL, NULL, &tv);
	if(r < 0)
		return r;
	if(r == 0) {
		//printf("linux_serial: read timeout!\n");
		return -ETIMEDOUT;
	}
	ssize_t count = read(self->fd, data, size);
	printf("linux_serial: read %ld\n", count);
	return (int)count;
}

int linux_serial_flush(struct linux_serial *self) {
	return tcflush(self->fd, TCIFLUSH);
}

static int _serial_write(serial_port_t serial, const void *_frame, size_t size,
                uint32_t tout) {
	if(!serial) return -1;
	struct linux_serial *self = container_of(serial, struct linux_serial, dev.ops);
	return linux_serial_write(self, _frame, size, tout);
}

static int _serial_read(serial_port_t serial, void *frame, size_t max_size,
               uint32_t tout_ms) {
	if(!serial) return -1;
	struct linux_serial *self = container_of(serial, struct linux_serial, dev.ops);
	return linux_serial_read(self, frame, max_size, tout_ms);
}

static const struct serial_device_ops _serial_ops = {
	.write = _serial_write,
	.read = _serial_read
};

struct linux_serial *linux_serial_new() {
	struct linux_serial *self = kzmalloc(sizeof(struct linux_serial));
	serial_device_init(&self->dev, 0, 0,  &_serial_ops);
	self->fd = -1;
	return self;
}

void linux_serial_delete(struct linux_serial *self){
	kfree(self);
}

serial_device_t linux_serial_as_serial_device(struct linux_serial *self){
	return &self->dev.ops;
}

static int _linux_serial_probe(void *fdt, int fdt_node) {
	int def_port = fdt_get_int_or_default(fdt, (int)fdt_node, "printk_port", 0);
	const char *tty = fdt_get_string_or_default(fdt, (int)fdt_node, "tty", "/dev/ttyUSB0");
	unsigned int baud = (unsigned int)fdt_get_int_or_default(fdt, (int)fdt_node, "baud", 0);

	struct linux_serial *self = linux_serial_new();

	serial_device_init(&self->dev, fdt, fdt_node, &_serial_ops);
	serial_device_register(&self->dev);

	if(def_port) {
		printf("linux_serial (%s): using as printk port\n",
		       fdt_get_name(fdt, fdt_node, NULL));
		serial_set_printk_port(&self->dev.ops);
	}

	if(linux_serial_connect(self, tty, baud) == 0){
		printf("linux_serial: connected to %s at %d\n", tty, baud);
	} else {
		printf("linux_serial: failed to connect to %s\n", tty);
	}

	return 0;
}

static int _linux_serial_remove(void *fdt, int fdt_node) {
	// TODO
	return -1;
}

DEVICE_DRIVER(linux_serial, "gnu,linux_serial", _linux_serial_probe, _linux_serial_remove)
