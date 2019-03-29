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
#include <libfirmware/adc.h>
#include <libfirmware/analog.h>

#include <libfdt/libfdt.h>

struct application {
    led_controller_t leds;
	console_t console;
	gpio_device_t sw_gpio;
	analog_device_t sw_leds;
	adc_device_t adc;
	analog_device_t mot_x;
	analog_device_t mot_y;
	gpio_device_t mux;
	int mux_chan;
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

static void _flyingbergman_indicator_loop(void *ptr){
	struct application *self = (struct application *)ptr;
	while(1){
		led_on(self->leds, 0);
		thread_sleep_ms(100);
		led_off(self->leds, 0);
		thread_sleep_ms(100);
	}
}

static void _flyingbergman_control_loop(void *ptr){
	struct application *self = (struct application *)ptr;

	bool led_state[8] = {0};
	bool sw_state[8] = {0};
	while(1){
		//adc_trigger(self->adc);
		thread_sleep_ms(5);

		for(unsigned int c = 0; c < 8; c++){
			bool on = gpio_read(self->sw_gpio, 8 + c);
			if(on && !sw_state[c] && !led_state[c]){
				printk("sw%d on\n", c);
				analog_write(self->sw_leds, c, 0.5f);
				led_state[c] = true;
			} else if(on && !sw_state[c] && led_state[c]){
				printk("sw%d off\n", c);
				analog_write(self->sw_leds, c, 0.f);
				led_state[c] = false;
			}
			sw_state[c] = on;
		}

		int16_t val = 0;

		adc_read(self->adc, 14, &val);
		float duty = constrain_float((float)(val - 2100) / 2120.0f, -0.45, 0.45);

		//printk("val: %d, duty %d\n", val, (int32_t)(duty * 1000));
		if(self->mux_chan){
			analog_write(self->mot_x, 0, 0.5 - duty);
			analog_write(self->mot_x, 1, 0.5 + duty);
		} else {
			analog_write(self->mot_y, 0, 0.5 - duty);
			analog_write(self->mot_y, 1, 0.5 + duty);
		}

		self->mux_chan = !self->mux_chan;
		gpio_write(self->mux, 0, self->mux_chan);
	}
}

static int _flyingbergman_probe(void *fdt, int fdt_node){
	struct application *self = kzmalloc(sizeof(struct application));

	gpio_device_t sw_gpio = gpio_find_by_ref(fdt, fdt_node, "sw_gpio");
	adc_device_t adc = adc_find_by_ref(fdt, fdt_node, "adc");
	analog_device_t mot_x = analog_find_by_ref(fdt, fdt_node, "mot_x");
	analog_device_t mot_y = analog_find_by_ref(fdt, fdt_node, "mot_y");
	analog_device_t sw_leds = analog_find_by_ref(fdt, fdt_node, "sw_leds");
	console_t console = console_find_by_ref(fdt, fdt_node, "console");
	gpio_device_t mux = gpio_find_by_ref(fdt, fdt_node, "mux");
	led_controller_t leds = leds_find_by_ref(fdt, fdt_node, "leds");

	if(!leds || !sw_leds || !mux || !sw_gpio || !adc || !mot_x || !mot_y){
		printk("fb: need sw_gpio, adc and pwm\n");
		return -1;
	}

	if(!console){
		printk("fb: console error\n");
		return -1;
	}

	console_add_command(console, self, "fb", "Flying Bergman Control", "", _fb_cmd);

	self->leds = leds;
	self->console = console;
	self->sw_gpio = sw_gpio;
	self->sw_leds = sw_leds;
	self->adc = adc;
	self->mot_x = mot_x;
	self->mot_y = mot_y;
	self->mux = mux;

	led_on(self->leds, 0);
	thread_sleep_ms(500);
	led_off(self->leds, 0);
	thread_sleep_ms(500);

	led_on(self->leds, 0);
	led_off(self->leds, 1);
	led_on(self->leds, 2);

	printk("HEAP: %lu of %lu free\n", thread_get_free_heap(), thread_get_total_heap());
	thread_meminfo();

	thread_create(
		  _flyingbergman_control_loop,
		  "fb_ctrl",
		  500,
		  self,
		  4,
		  NULL);

	thread_create(
		  _flyingbergman_indicator_loop,
		  "fb_ind",
		  250,
		  self,
		  1,
		  NULL);

	printk("FB Rev A\n");

	return 0;
}

static int _flyingbergman_remove(void *fdt, int fdt_node){
	// TODO
    return -1;
}

DEVICE_DRIVER(flyingbergman, "app,flyingbergman", _flyingbergman_probe, _flyingbergman_remove)
