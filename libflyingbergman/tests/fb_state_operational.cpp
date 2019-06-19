/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
extern "C" {
#include "fb.h"
#include "fb_state.h"

static void (*_state)(struct fb *self, float dt);
static float _pitch_target = 0, _yaw_target = 0;
static fb_led_state_t _led_states[FB_LED_COUNT];
static struct { float pitch, yaw; } _presets[FB_PRESET_COUNT];
static bool _leds_reset = false;
static struct fb_control *_pitch_control;
static struct fb_control *_yaw_control;

void _fb_state_save_preset(struct fb *self, float dt) {
}

void _fb_enter_state(struct fb *self, void (*fp)(struct fb *self, float dt)) {
	_state = fp;
}

void fb_control_set_limits(struct fb_control *self, float acc, float vel, float dec) {

}

void fb_control_set_input(struct fb_control *self, float pos, float vel){

}

void fb_control_set_target(struct fb_control *self, float pos) {
	if(self == _pitch_control)
	_pitch_target = pos;
	else if(self == _yaw_control)
		_yaw_target = pos;
}
void fb_leds_set_state(struct fb_leds *self, unsigned led, fb_led_state_t state) {
	_led_states[led] = state;
}

void fb_leds_reset(struct fb_leds *self) {
	_leds_reset = true;
}

void fb_config_set_preset(struct fb_config *self, unsigned preset, float pitch,
                          float yaw) {
	printf("Set preset %d %f %f\n", preset, pitch, yaw);
	_presets[preset].pitch = pitch;
	_presets[preset].yaw = yaw;
}
}

class StateOperationalTest : public ::testing::Test {
	protected:
	struct fb fb;
	void SetUp() {
		memset(&fb, 0, sizeof(fb));
		for(unsigned c = 0; c < FB_LED_COUNT; c++) {
			_led_states[c] = FB_LED_STATE_OFF;
		}
		for(unsigned c = 0; c < FB_PRESET_COUNT; c++) {
			_presets[c].pitch = 0;
			_presets[c].yaw = 0;
		}
		_pitch_target = 0;
		_yaw_target = 0;
		_state = NULL;
		_fb_state_operational_enter(&fb);
		_leds_reset = false;
		_pitch_control = &fb.axis[FB_AXIS_UPDOWN];
		_yaw_control = &fb.axis[FB_AXIS_LEFTRIGHT];
	}
	void TearDown() {
	}

	void clock(unsigned ticks) {
		for(unsigned c = 0; c < ticks; c++) {
			_fb_state_operational(&fb, 0.001f);
		}
	}
};

TEST_F(StateOperationalTest, default_state) {
	fb.measured.pitch = 123;
	fb.measured.yaw = 456;
	for(unsigned c = 0; c < FB_PRESET_COUNT; c++) {
		EXPECT_FLOAT_EQ(_presets[c].pitch, 0) << "Checking preset pitch " << c;
		EXPECT_FLOAT_EQ(_presets[c].yaw, 0) << "Checking preset yaw " << c;
	}
	clock(1);
	for(unsigned c = 0; c < FB_PRESET_COUNT; c++) {
		EXPECT_FLOAT_EQ(_presets[c].pitch, 0) << "Checking preset pitch " << c;
		EXPECT_FLOAT_EQ(_presets[c].yaw, 0) << "Checking preset yaw " << c;
	}
}

TEST_F(StateOperationalTest, preset_activation) {
	fb.measured.pitch = 123;
	fb.measured.yaw = 456;
	// check home button preset. activation
	fb.inputs.sw[FB_SW_HOME].pressed = true;
	fb.inputs.sw[FB_SW_HOME].long_pressed = true;
	clock(1);
	EXPECT_FLOAT_EQ(_presets[FB_PRESET_HOME].pitch, 0);
	EXPECT_FLOAT_EQ(_presets[FB_PRESET_HOME].yaw, fb.measured.yaw);
	fb.inputs.sw[FB_SW_HOME].pressed = false;
	fb.inputs.sw[FB_SW_HOME].long_pressed = false;
	// check preset button activations
	for(unsigned c = 0; c < FB_PRESET_COUNT; c++) {
		struct fb_switch_state *sw = NULL;
		switch(c) {
			case FB_PRESET_HOME:
				continue;
			case FB_PRESET_1:
				sw = &fb.inputs.sw[FB_SW_PRESET_1];
				break;
			case FB_PRESET_2:
				sw = &fb.inputs.sw[FB_SW_PRESET_2];
				break;
			case FB_PRESET_3:
				sw = &fb.inputs.sw[FB_SW_PRESET_3];
				break;
			case FB_PRESET_4:
				sw = &fb.inputs.sw[FB_SW_PRESET_4];
				break;
			default:
				EXPECT_TRUE(false) << "Unhandled preset! (" << c << ")";
		}
		if(c == FB_PRESET_HOME)
			continue;
		sw->pressed = true;
		sw->long_pressed = true;
		_presets[c].pitch = 0;
		_presets[c].yaw = 0;
		clock(1);
		EXPECT_FLOAT_EQ(_presets[c].pitch, fb.measured.pitch)
		    << "Checking preset " << c << " pitch";
		EXPECT_FLOAT_EQ(_presets[c].yaw, fb.measured.yaw)
		    << "Checking preset " << c << " yaw";
		sw->pressed = false;
		sw->long_pressed = false;
	}
}

TEST_F(StateOperationalTest, preset_load) {
	// clang-format off
	fb.config.presets[1].pitch	= 0.110;
	fb.config.presets[1].yaw	= 0.111;
	fb.config.presets[1].valid	= true;
	fb.config.presets[2].pitch	= 0.220;
	fb.config.presets[2].yaw	= 0.221;
	fb.config.presets[2].valid	= true;
	fb.config.presets[3].pitch	= 0.330;
	fb.config.presets[3].yaw	= 0.331;
	fb.config.presets[3].valid	= true;
	fb.config.presets[4].pitch	= 0.440;
	fb.config.presets[4].yaw	= 0.441;
	fb.config.presets[4].valid	= true;
	// clang-format on
	EXPECT_FLOAT_EQ(_presets[1].pitch, 0);
	EXPECT_FLOAT_EQ(_presets[1].yaw, 0);
	fb.inputs.sw[FB_SW_PRESET_1].pressed = false;
	fb.inputs.sw[FB_SW_PRESET_1].toggled = true;
	clock(1);
	EXPECT_FLOAT_EQ(_pitch_target, 0.110);
	EXPECT_FLOAT_EQ(_yaw_target, 0.111);

	EXPECT_FLOAT_EQ(_presets[2].pitch, 0);
	EXPECT_FLOAT_EQ(_presets[2].yaw, 0);
	fb.inputs.sw[FB_SW_PRESET_2].pressed = false;
	fb.inputs.sw[FB_SW_PRESET_2].toggled = true;
	clock(1);
	EXPECT_FLOAT_EQ(_pitch_target, 0.220);
	EXPECT_FLOAT_EQ(_yaw_target, 0.221);

	EXPECT_FLOAT_EQ(_presets[3].pitch, 0);
	EXPECT_FLOAT_EQ(_presets[3].yaw, 0);
	fb.inputs.sw[FB_SW_PRESET_3].pressed = false;
	fb.inputs.sw[FB_SW_PRESET_3].toggled = true;
	clock(1);
	EXPECT_FLOAT_EQ(_pitch_target, 0.330);
	EXPECT_FLOAT_EQ(_yaw_target, 0.331);

	EXPECT_FLOAT_EQ(_presets[4].pitch, 0);
	EXPECT_FLOAT_EQ(_presets[4].yaw, 0);
	fb.inputs.sw[FB_SW_PRESET_4].pressed = false;
	fb.inputs.sw[FB_SW_PRESET_4].toggled = true;
	clock(1);
	EXPECT_FLOAT_EQ(_pitch_target, 0.440);
	EXPECT_FLOAT_EQ(_yaw_target, 0.441);
}

int main(int argc, char **argv) {
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
