/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#include "fb.h"

static int _fb_probe(void *fdt, int fdt_node) {
	struct fb *self = (struct fb *)kzmalloc(sizeof(struct fb));

	BUG_ON(!self);

	gpio_device_t sw_gpio = gpio_find_by_ref(fdt, fdt_node, "sw_gpio");
	gpio_device_t gpio_ex = gpio_find_by_ref(fdt, fdt_node, "gpio_ex");
	adc_device_t adc = adc_find_by_ref(fdt, fdt_node, "adc");
	analog_device_t mot_x = analog_find_by_ref(fdt, fdt_node, "mot_x");
	analog_device_t mot_y = analog_find_by_ref(fdt, fdt_node, "mot_y");
	analog_device_t sw_leds = analog_find_by_ref(fdt, fdt_node, "sw_leds");
	console_device_t console = console_find_by_ref(fdt, fdt_node, "console");
	gpio_device_t mux = gpio_find_by_ref(fdt, fdt_node, "mux");
	led_controller_t leds = leds_find_by_ref(fdt, fdt_node, "leds");
	encoder_device_t enc1 = encoder_find_by_ref(fdt, fdt_node, "enc1");
	encoder_device_t enc2 = encoder_find_by_ref(fdt, fdt_node, "enc2");
	memory_device_t eeprom = memory_find_by_ref(fdt, fdt_node, "eeprom");
	analog_device_t oc_pot = analog_find_by_ref(fdt, fdt_node, "oc_pot");
	can_device_t can1 = can_find_by_ref(fdt, fdt_node, "can1");
	memory_device_t can1_mem = memory_find_by_ref(fdt, fdt_node, "can1");
	can_device_t can2 = can_find_by_ref(fdt, fdt_node, "can2");
	memory_device_t can2_mem = memory_find_by_ref(fdt, fdt_node, "can2");
	regmap_device_t regmap = regmap_find_by_ref(fdt, fdt_node, "regmap");
	memory_device_t canopen_mem = memory_find_by_ref(fdt, fdt_node, "canopen");
	gpio_device_t enc1_gpio = gpio_find_by_ref(fdt, fdt_node, "enc1_gpio");
	gpio_device_t enc2_gpio = gpio_find_by_ref(fdt, fdt_node, "enc2_gpio");
	drv8302_t drv_pitch = memory_find_by_ref(fdt, fdt_node, "drv_pitch");
	drv8302_t drv_yaw = memory_find_by_ref(fdt, fdt_node, "drv_yaw");
	events_device_t events = events_find_by_ref(fdt, fdt_node, "events");

	if(!leds || !sw_leds || !mux || !sw_gpio || !adc || !mot_x || !mot_y) {
		printk("fb: need sw_gpio, adc and pwm\n");
		return -1;
	}

	if(!eeprom) {
		printk(PRINT_ERROR "fb: eeprom missing\n");
		return -1;
	}
	if(!console) {
		printk(PRINT_ERROR "fb: console missing\n");
		return -1;
	}
	if(!oc_pot) {
		printk(PRINT_ERROR "fb: oc_pot missing\n");
		return -1;
	}
	if(!can1) {
		printk(PRINT_ERROR "fb: can1 missing\n");
		return -1;
	}
	if(!can2) {
		printk(PRINT_ERROR "fb: can2 missing\n");
		return -1;
	}
	if(!regmap) {
		printk(PRINT_ERROR "fb: regmap missing\n");
		return -1;
	}
	if(!canopen_mem) {
		printk(PRINT_ERROR "fb: canopen missing\n");
		return -1;
	}
	if(!enc1) {
		printk(PRINT_ERROR "fb: enc1 missing\n");
		return -1;
	}
	if(!enc2) {
		printk(PRINT_ERROR "fb: enc2 missing\n");
		return -1;
	}
	if(!enc1_gpio) {
		printk(PRINT_ERROR "fb: enc1_gpio missing\n");
		return -1;
	}
	if(!enc2_gpio) {
		printk(PRINT_ERROR "fb: enc2_gpio missing\n");
		return -1;
	}
	if(!gpio_ex) {
		printk(PRINT_ERROR "fb: gpio_ex missing\n");
		return -1;
	}
	if(!drv_pitch) {
		printk(PRINT_ERROR "fb: drv_pitch missing\n");
		return -1;
	}
	if(!drv_yaw) {
		printk(PRINT_ERROR "fb: drv_yaw missing\n");
		return -1;
	}

	self->leds = leds;
	self->console = console;
	self->sw_gpio = sw_gpio;
	self->gpio_ex = gpio_ex;
	self->sw_leds = sw_leds;
	self->adc = adc;
	self->mot_x = mot_x;
	self->mot_y = mot_y;
	self->mux = mux;
	self->enc1 = enc1;
	self->enc2 = enc2;
	self->eeprom = eeprom;
	self->oc_pot = oc_pot;
	self->can1 = can1;
	self->can1_mem = can1_mem;
	self->can2 = can2;
	self->can2_mem = can2_mem;
	self->regmap = regmap;
	self->canopen_mem = canopen_mem;
	self->enc1_gpio = enc1_gpio;
	self->enc2_gpio = enc2_gpio;
	self->drv_pitch = drv_pitch;
	self->drv_yaw = drv_yaw;
	self->events = events;
	self->debug_gpio = gpio_find_by_ref(fdt, fdt_node, "debug_gpio");
	// self->canopen = canopen;

	fb_init(self);

	return 0;
}

static int _fb_remove(void *fdt, int fdt_node) {
	// TODO
	return -1;
}

DEVICE_DRIVER(flyingbergman, "app,flyingbergman", _fb_probe, _fb_remove)
