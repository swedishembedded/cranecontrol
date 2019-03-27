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
#include <libfirmware/thread.h>
#include <libfirmware/console.h>
#include <libfirmware/leds.h>
#include <libfirmware/usb.h>
#include <libfirmware/timestamp.h>
#include <libfirmware/gpio.h>

#include <libfdt/libfdt.h>

struct application {
    led_controller_t leds;
    serial_port_t uart;
	console_t console;
	gpio_device_t sw_gpio;
};

static int _fb_cmd(console_t con, void *userptr, int argc, char **argv){
	struct application *self = (struct application*)userptr;
	if(argc == 2 && strcmp(argv[1], "sw") == 0){
		for(int c = 0; c < 8; c++){
			int val = gpio_read(self->sw_gpio, (uint32_t)(8+c));
			console_printf(con, "SW%d: %d\n", c, val);
		}
	}
	return 0;
}

static int _flyingbergman_probe(void *fdt, int fdt_node){
	struct application *self = kzmalloc(sizeof(struct application));

	uint32_t off_period = (uint32_t)fdt_get_int_or_default(fdt, (int)fdt_node, "off_period", 100);
	uint32_t on_period = (uint32_t)fdt_get_int_or_default(fdt, (int)fdt_node, "on_period", 100);

	gpio_device_t sw_gpio = gpio_find_by_ref(fdt, fdt_node, "sw_gpio");
	if(!sw_gpio){
		printk("fb: sw_gpio error\n");
		return -1;
	}

	console_t console = console_find_by_ref(fdt, fdt_node, "console");
	if(!console){
		printk("fb: console error\n");
		return -1;
	}

	console_add_command(console, self, "fb", "Flying Bergman Control", "", _fb_cmd);

	self->leds = leds_find("/leds");
	self->uart = serial_find("/serial/debug");
	self->console = console;
	self->sw_gpio = sw_gpio;

	BUG_ON(!self->leds);

	led_on(self->leds, 0);
	thread_sleep_ms(500);
	led_off(self->leds, 0);
	thread_sleep_ms(500);

	led_on(self->leds, 0);
	led_off(self->leds, 1);
	led_on(self->leds, 2);

	BUG_ON(!self->uart);

	printk("HEAP: %lu of %lu free\n", thread_get_free_heap(), thread_get_total_heap());
	thread_meminfo();

	printk("FB Rev A\n");

	while(1){
		led_on(self->leds, 0);
		thread_sleep_ms(on_period);
		led_off(self->leds, 0);
		thread_sleep_ms(off_period);
	}

	return 0;
}

static int _flyingbergman_remove(void *fdt, int fdt_node){
	// TODO
    return -1;
}

DEVICE_DRIVER(flyingbergman, "app,flyingbergman", _flyingbergman_probe, _flyingbergman_remove)
