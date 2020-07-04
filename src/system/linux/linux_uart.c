#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "linux/linux_uart.h"
#include "driver.h"
#include "serial.h"
#include "types/types.h"

struct linux_uart {
	struct serial_device dev;
	int fd_read, fd_write;
	int fd_host_read, fd_host_write;
	int def_port;
};

int linux_uart_write(struct linux_uart *self, const void *_frame, size_t size,
                uint32_t tout) {
	if(self->def_port) {
		if(write(1, _frame, size) < 0) {
			perror("write");
		}
	}

	// check for hangup event and do not write to the terminal if the other end has
	// hung up
	struct pollfd pfd = {.fd = self->fd_write, .events = POLLHUP};
	poll(&pfd, 1, 1 /* or other small timeout */);

	if(!(pfd.revents & POLLHUP)) {
		/* There is now a reader on the slave side */
		ssize_t written = 0;
		const uint8_t *frame = (const uint8_t *)_frame;
		while(written < (ssize_t)size) {
			ssize_t ret =
			    write(self->fd_write, frame + written, (size_t)(size - (size_t)written));
			if(ret <= 0)
				return (int)ret;
			written += ret;
		}
		return (int)written;
	}
	return -EOF;
}


int linux_uart_read(struct linux_uart *self, void *frame, size_t max_size,
               uint32_t tout_ms) {
	suseconds_t tv_sec = (suseconds_t)(tout_ms / 1000);
	suseconds_t tv_usec = (suseconds_t)((tout_ms % 1000) * 1000);

	while(1) {
		struct timeval tv;
		fd_set fds;
		tv.tv_sec = tv_sec;
		tv.tv_usec = tv_usec;

		FD_ZERO(&fds);
		FD_SET(self->fd_read, &fds);

		if(tout_ms == THREAD_SLEEP_MAX_DELAY) {
			select(self->fd_read + 1, &fds, NULL, NULL, NULL);
		} else {
			select(self->fd_read + 1, &fds, NULL, NULL, &tv);
		}

		if(FD_ISSET(self->fd_read, &fds)) {
			int r = (int)read(self->fd_read, frame, max_size);
			if(r > 0) {
				return r;
			}
		}
	}

	return 0;
}

/**
 * Initialize a uart object to read and write from an existing fd
 * @param self pointer to the uart object
 * @param fd file descriptor to bind to this uart object (must be a valid file
 * descriptor)
 */
int linux_uart_init_fd(struct linux_uart *self, int fd) {
	memset(self, 0, sizeof(*self));
	self->fd_read = fd;
	self->fd_write = fd;
	return 0;
}
/*
static int linux_uart_init_pipe(struct linux_uart *self) {
	memset(self, 0, sizeof(*self));

	int fds[2];
	memset(fds, -1, sizeof(fds));
	if(pipe(fds) < 0) {
		perror("pipe");
		return -1;
	}
	self->fd_read = fds[0];
	self->fd_host_write = fds[1];
	memset(fds, -1, sizeof(fds));
	if(pipe(fds) < 0) {
		perror("pipe");
		return -1;
	}
	self->fd_host_read = fds[0];
	self->fd_write = fds[1];
	return 0;
}

int linux_uart_init(struct linux_uart *self) {
	return linux_uart_init_pipe(self);
}
*/

static const struct serial_device_ops _linux_serial;

struct linux_uart *linux_uart_new(){
	int fdm = posix_openpt(O_RDWR);
	if(fdm < 0) {
		perror("linux_uart open");
		return 0;
	}

	int rc = grantpt(fdm);
	if(rc != 0) {
		perror("linux_art grantpt");
		return 0;
	}
	rc = unlockpt(fdm);
	if(rc != 0) {
		perror("linux_uart unlockpt");
		return 0;
	}

	// ensure that pollhup is set so we can check it when writing
	close(open(ptsname(fdm), O_RDWR | O_NOCTTY));

	struct linux_uart *self = kzmalloc(sizeof(struct linux_uart));
	linux_uart_init_fd(self, fdm);

	serial_device_init(&self->dev, 0, 0, &_linux_serial);

	return self;
}

const char *linux_uart_get_ptsname(const struct linux_uart *self){
	const char *pts = (const char *)ptsname(self->fd_read);
	if(!pts) return "<invalid>";
	return pts;
}

static int _pipe_write(serial_port_t serial, const void *_frame, size_t size,
                uint32_t tout) {
	struct linux_uart *self = container_of(serial, struct linux_uart, dev.ops);
	return linux_uart_write(self, _frame, size, tout);
}

static int _pipe_read(serial_port_t serial, void *frame, size_t max_size,
               uint32_t tout_ms) {
	struct linux_uart *self = container_of(serial, struct linux_uart, dev.ops);
	return linux_uart_read(self, frame, max_size, tout_ms);
}

static const struct serial_device_ops _linux_serial = {.write = _pipe_write,
                                                       .read = _pipe_read};


serial_device_t linux_uart_as_serial_device(struct linux_uart *self){
	return &self->dev.ops;
}

static int _linux_uart_probe(void *fdt, int fdt_node) {
	int def_port = fdt_get_int_or_default(fdt, (int)fdt_node, "printk_port", 0);

	struct linux_uart *self = linux_uart_new();

	self->def_port = def_port;

	serial_device_init(&self->dev, fdt, fdt_node, &_linux_serial);
	serial_device_register(&self->dev);

	if(def_port) {
		printf("linux_uart (%s): using as printk port\n",
		       fdt_get_name(fdt, fdt_node, NULL));
		serial_set_printk_port(&self->dev.ops);
	}

	printf("linux_uart: start pseudo terminal at %s\n", linux_uart_get_ptsname(self));

	return 0;
}

static int _linux_uart_remove(void *fdt, int fdt_node) {
	// TODO
	return -1;
}

DEVICE_DRIVER(linux_uart, "gnu,linux_uart", _linux_uart_probe, _linux_uart_remove)
