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
#include <libfirmware/encoder.h>
#include <libfirmware/memory.h>
#include <libfirmware/timestamp.h>

#include <libfdt/libfdt.h>

#define FB_SWITCH_COUNT 8
#define FB_LED_COUNT 8
#define FB_PRESET_COUNT 4

#define FB_PRESET_BIT_VALID (1 << 0)

struct config {
	struct config_preset {
		uint8_t flags;
		float pitch, yaw;
	} preset[FB_PRESET_COUNT];
};

struct application {
    led_controller_t leds;
	console_t console;
	gpio_device_t sw_gpio;
	analog_device_t sw_leds;
	adc_device_t adc;
	analog_device_t mot_x;
	analog_device_t mot_y;
	gpio_device_t mux;
	encoder_device_t enc1;
	memory_device_t eeprom;

	struct {
		timestamp_t prev_micros;
		float pitch, yaw;
		timestamp_t sw_last_toggle[FB_SWITCH_COUNT];
		struct fb_led_state {
			float intensity;
			int dim;
		} leds[FB_LED_COUNT];
	} state;

	struct {
		float pitch_rad, yaw_rad;
	} actual;

	struct {
		float yaw, pitch;
		float yaw_acc, pitch_acc;
		float yaw_speed, pitch_speed;
		bool sw[FB_SWITCH_COUNT];
		bool sw_toggle[FB_SWITCH_COUNT];
	} controls;

	int mux_chan;
	int16_t mux_adc[8];
};

static int _fb_cmd(console_t con, void *userptr, int argc, char **argv){
	struct application *self = (struct application*)userptr;
	if(argc == 2 && strcmp(argv[1], "sw") == 0){
		for(int c = 0; c < 8; c++){
			int val = gpio_read(self->sw_gpio, (uint32_t)(8+c));
			console_printf(con, "SW%d: %d\n", c, val);
		}
	} else if(argc == 2 && strcmp(argv[1], "stats") == 0){
		console_printf(con, "SW: ");
		for(int c = 0; c < FB_SWITCH_COUNT; c++){
			console_printf(con, "[%2d] %d ", c, self->controls.sw[c]);
		}
		console_printf(con, "\n");

		console_printf(con, "AIN: ");
		for(int c = 0; c < FB_SWITCH_COUNT; c++){
			console_printf(con, "[%2d] %5d ", c, self->mux_adc[c]);
		}
		console_printf(con, "\n");

		console_printf(con, "ADC: ");
		for(unsigned c = 0; c < 16; c++){
			int16_t val = 0;
			adc_read(self->adc, c, &val);
			console_printf(con, "[%2d] %5d ", c, val);
		}
		console_printf(con, "\n");

		console_printf(con, "JOYSTICK: YAW %5d PITCH %5d\n",
				(int32_t)(self->controls.yaw * 1000),
				(int32_t)(self->controls.pitch * 1000)
		);
		console_printf(con, "INTENSITY: YAW %5d PITCH %5d\n",
				(int32_t)(self->controls.yaw_acc * 1000),
				(int32_t)(self->controls.pitch_acc * 1000)
		);
		console_printf(con, "SPEED: YAW %5d PITCH %5d\n",
				(int32_t)(self->controls.yaw_speed * 1000),
				(int32_t)(self->controls.pitch_speed * 1000)
		);
		console_printf(con, "ACTUAL: YAW %5d PITCH %5d\n",
				(int32_t)(self->actual.yaw_rad * 1000),
				(int32_t)(self->actual.pitch_rad * 1000)
		);

	} else if(argc == 2 && strcmp(argv[1], "enc") == 0){
		int32_t val = encoder_read(self->enc1);
		console_printf(con, "ENC1: %d\n", val);
	} else if(argc == 2 && strcmp(argv[1], "ee") == 0){
		console_printf(con, "Writing eeprom\n");
		static uint8_t buf[] = { 1, 2, 3, 4 };
		memory_write(self->eeprom, 0, buf, 4);
		console_printf(con, "Reading eeprom\n");
		static uint8_t rbuf[4];
		memory_read(self->eeprom, 0, rbuf, 4);
		for(size_t c = 0; c < 4; c++){
			console_printf(con, "%d\n", rbuf[c]);
		}
	} else {
		console_printf(con, "Invalid option\n");
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

static void _flyingbergman_read_controls(struct application *self){
	// read next adc mux channel
	adc_read(self->adc, 14, &self->mux_adc[self->mux_chan]);

	float duty = constrain_float((float)(self->mux_adc[self->mux_chan]) / 4096, 0.0f, 1.0f);
	switch(self->mux_chan) {
		case 0: { self->controls.yaw = -1.f + 2.f * duty; } break;
		case 1: { self->controls.pitch = -1.f + 2.f * duty; } break;
		case 2: { self->controls.yaw_acc = constrain_float(2.f * (duty - 0.25), 0.f, 1.f); } break;
		case 3: { self->controls.pitch_acc = constrain_float(2.f * (duty - 0.25), 0.f, 1.f); } break;
		case 4: { self->controls.yaw_speed = constrain_float(2.f * (duty - 0.25), 0.f, 1.f); } break;
		case 5: { self->actual.pitch_rad = constrain_float(2.f * (duty - 0.25), 0.f, 1.f); } break;
		case 7: { self->controls.pitch_speed = constrain_float(2.f * (duty - 0.25), 0.f, 1.f); } break;
	}

	self->mux_chan = (self->mux_chan+1) & 0x7;

	gpio_write(self->mux, 0, self->mux_chan & 1);
	gpio_write(self->mux, 1, (self->mux_chan >> 1) & 1);
	gpio_write(self->mux, 2, (self->mux_chan >> 2) & 1);

	// read switches
	for(unsigned int c = 0; c < 8; c++){
		bool on = gpio_read(self->sw_gpio, 8 + c);
		self->controls.sw_toggle[c] = on != self->controls.sw[c];
		if(self->controls.sw_toggle[c]){
			self->state.sw_last_toggle[c] = micros();
		}
		self->controls.sw[c] = on;
	}

	// read motor positions
	float yrad = M_PI * ((float)(int16_t)(encoder_read(self->enc1) & 0xffff) / (float)(int16_t)(0xffff >> 1));
	self->actual.yaw_rad = yrad;
}

static void _flyingbergman_update_state(struct application *self){
	timestamp_t t = micros();
	float dt = (float)(t - self->state.prev_micros) * 1e-6;
	self->state.prev_micros = t;

	for(unsigned c = 0; c < FB_LED_COUNT; c++){
		struct fb_led_state *led = &self->state.leds[c];
		if(led->dim < 0){
			led->intensity -= 0.5 * dt;
			if(led->intensity < 0.0f){
				led->intensity = 0.0f;
				led->dim = -led->dim;
			}
		} else if(led->dim > 0){
			led->intensity += 0.5 * dt;
			if(led->intensity > 1.0f){
				led->intensity = 1.0f;
				led->dim = -led->dim;
			}
		}

		analog_write(self->sw_leds, c, led->intensity);
	}

	// update pitch and yaw
	float pitch = self->state.pitch;
	if(pitch < self->controls.pitch) pitch += 4.f * self->controls.pitch_acc * dt;
	else if(pitch > self->controls.pitch) pitch -= 4.f * self->controls.pitch_acc * dt;

	float yaw = self->state.yaw;
	if(yaw < self->controls.yaw) yaw += 4.f * self->controls.yaw_acc * dt;
	else if(yaw > self->controls.yaw) yaw -= 4.f * self->controls.yaw_acc * dt;

	self->state.pitch = constrain_float(pitch, -self->controls.pitch_speed, self->controls.pitch_speed);
	self->state.yaw = constrain_float(yaw, -self->controls.yaw_speed, self->controls.yaw_speed);
}

static void _flyingbergman_update_outputs(struct application *self){
	analog_write(self->mot_y, 0, 0.5 - self->state.pitch * 0.5f);
	analog_write(self->mot_y, 1, 0.5 + self->state.pitch * 0.5f);
	analog_write(self->mot_x, 0, 0.5 - self->state.yaw * 0.5f);
	analog_write(self->mot_x, 1, 0.5 + self->state.yaw * 0.5f);
}

static void _flyingbergman_control_loop(void *ptr){
	struct application *self = (struct application *)ptr;

	while(1){
		//adc_trigger(self->adc);
		thread_sleep_ms(5);

		_flyingbergman_read_controls(self);
		_flyingbergman_update_state(self);
		_flyingbergman_update_outputs(self);
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
	encoder_device_t enc1 = encoder_find_by_ref(fdt, fdt_node, "enc1");
	memory_device_t eeprom = memory_find_by_ref(fdt, fdt_node, "eeprom");

	if(!leds || !sw_leds || !mux || !sw_gpio || !adc || !mot_x || !mot_y || !enc1){
		printk("fb: need sw_gpio, adc and pwm\n");
		return -1;
	}

	if(!eeprom){ printk("fb: no eeprom\n"); return -1; }
	if(!console){ printk("fb: console error\n"); return -1; }

	console_add_command(console, self, "fb", "Flying Bergman Control", "", _fb_cmd);

	self->leds = leds;
	self->console = console;
	self->sw_gpio = sw_gpio;
	self->sw_leds = sw_leds;
	self->adc = adc;
	self->mot_x = mot_x;
	self->mot_y = mot_y;
	self->mux = mux;
	self->enc1 = enc1;
	self->eeprom = eeprom;

	for(int c = 0; c < FB_LED_COUNT; c++){
		self->state.leds[c].dim = 1;
	}

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
