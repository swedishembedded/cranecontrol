#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

#include <libfirmware/serial.h>
#include <libfirmware/constrain.h>
#include <libfirmware/chip.h>
#include <libfirmware/driver.h>
#include <libfirmware/debug.h>
#include <libfirmware/thread/thread.h>
#include <libfirmware/console.h>
#include <libfirmware/leds.h>
#include <libfirmware/usb.h>
#include <libfirmware/types/timestamp.h>


void vApplicationTaskSWIHook(int task){
	(void)task;
	//swo_sendchar(4, '0'+task);
	//dac_write(&chassis.dac1, 1, task * 500);
	//dac_write(&dac1, 2, task * 100);
}

int _close (int fd){
	printk(PRINT_ERROR "close not implemented\n");
	return -1;
}

/*
int _fstat (int fd, struct stat * buf){
	printk("fstat\n");
}
*/

int _isatty (int fd){
	printk(PRINT_ERROR "isatty not implemented\n");
	return -1;
}
off_t _lseek (int fd, off_t offset, int whence){
	printk(PRINT_ERROR "seek not implemented\n");
	return -1;
}
int _open (const char * pathname, int flags){
	printk(PRINT_ERROR "open not implemented\n");
	return -1;
}
ssize_t _read (int fd, void * buf, size_t count){
	printk(PRINT_ERROR "read %d not implemented\n", fd);
	return -1;
}

ssize_t _write (int fd, const void * buf, size_t count){
	ssize_t ret = count;
	if(fd == 1){
		while(count--){
			const char *str = buf;
			printk("%c", *str++);
		}
		return ret;
	} else if(fd == 2){
		while(count--){
			const char *str = buf;
			printk(PRINT_ERROR "%c", *str++);
		}
		return ret;
	}

	printk(PRINT_ERROR "write %d not implemented\n", fd);
	return -1;
}

void * _sbrk (ptrdiff_t increment){
	printk(PRINT_ERROR "sbrk %d not implemented\n", (int)increment);
	return 0;
}


void panic(const char *msg){
	while(1) asm("nop");
}

