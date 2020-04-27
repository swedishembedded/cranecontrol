#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

#include <libfirmware/serial.h>
#include <libfirmware/math.h>
#include <libfirmware/chip.h>
#include <libfirmware/driver.h>
#include <libfirmware/debug.h>
#include <libfirmware/thread/thread.h>
#include <libfirmware/console.h>
#include <libfirmware/leds.h>
#include <libfirmware/usb.h>
#include <libfirmware/types/timestamp.h>

static void _init_drivers(void *ptr){
	probe_device_drivers(_devicetree);

	thread_suspend();
}

int main(void){
	thread_create(
		  _init_drivers,
		  "init",
		  450,
		  NULL,
		  2,
		  NULL);

	thread_start_scheduler();

	return 0;
}

