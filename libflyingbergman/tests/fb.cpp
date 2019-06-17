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
#include "../src/fb.c"
#include "fb_leds.h"
#include "fb_state.h"

void thread_sched_suspend(void) {
}

void thread_sched_resume(void) {
}

int canopen_set_mode(memory_device_t canopen_mem, canopen_mode_t mode) {
	return 0;
}
int canopen_set_address(memory_device_t canopen_mem, uint8_t address) {
	return 0;
}

int canopen_pdo_rx(memory_device_t canopen_mem, uint8_t node_id,
                   const struct canopen_pdo_config *conf) {
	return 0;
}
int canopen_pdo_tx(memory_device_t canopen_mem, uint8_t node_id,
                   const struct canopen_pdo_config *conf) {
}

int thread_mutex_lock(struct mutex *self) {
	return 0;
}

int thread_mutex_unlock(struct mutex *self) {
	return 0;
}

int drv8302_get_gain(drv8302_t drv) {
	return 10;
}
void drv8302_enable_calibration(drv8302_t dev, bool en){

	}

int printk(const char *fmt, ...){
	printf("%s\n", fmt);
}

}

#include "mock/timestamp.cpp"
#include "mock/regmap.cpp"
#include "mock/thread.cpp"
#include "mock/gpio.cpp"

class FlyingBergmanTest : public ::testing::Test {
	protected:
	struct fb fb;
	void SetUp() {
		//fb_init(&fb);
	}
	void TearDown() {
	}
	void clock(unsigned cycles) {
		for(unsigned c = 0; c < cycles; c++) {
			_fb_update(&fb);
		}
	}
};

TEST_F(FlyingBergmanTest, leds_vs_button_ids) {
	EXPECT_EQ(FB_LED_PRESET_1, FB_SW_PRESET_1);
	EXPECT_EQ(FB_LED_PRESET_2, FB_SW_PRESET_2);
	EXPECT_EQ(FB_LED_PRESET_3, FB_SW_PRESET_3);
	EXPECT_EQ(FB_LED_PRESET_4, FB_SW_PRESET_4);
	EXPECT_EQ(FB_LED_HOME, FB_SW_HOME);
	EXPECT_EQ(FB_LED_COUNT, FB_SWITCH_COUNT);
}
#if 0
TEST_F(FlyingBergmanTest, mode_switch_using_can_address) {
	fb.inputs.can_addr = FB_CANOPEN_MASTER_ADDRESS;
	clock(1);
	EXPECT_EQ(fb.mode, FB_MODE_MASTER);
	fb.inputs.can_addr = FB_CANOPEN_MASTER_ADDRESS + 1;
	clock(1);
	EXPECT_EQ(fb.mode, FB_MODE_SLAVE);
}

TEST_F(FlyingBergmanTest, slave_mode) {
	fb.inputs.can_addr = FB_CANOPEN_MASTER_ADDRESS + 1;
	clock(1);
	EXPECT_EQ(fb.mode, FB_MODE_SLAVE);
}
#endif
int main(int argc, char **argv) {
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
