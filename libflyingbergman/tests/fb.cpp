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
#include "fb_leds.h"
}

class FlyingBergmanTest : public ::testing::Test{
protected:
	void SetUp(){
	}
	void TearDown(){
	}
};

TEST_F(FlyingBergmanTest, leds_vs_button_ids){
	EXPECT_EQ(FB_LED_PRESET_1, FB_SW_PRESET_1);
	EXPECT_EQ(FB_LED_PRESET_2, FB_SW_PRESET_2);
	EXPECT_EQ(FB_LED_PRESET_3, FB_SW_PRESET_3);
	EXPECT_EQ(FB_LED_PRESET_4, FB_SW_PRESET_4);
	EXPECT_EQ(FB_LED_HOME, FB_SW_HOME);
	EXPECT_EQ(FB_LED_COUNT, FB_SWITCH_COUNT);
}

int main(int argc, char **argv) {
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
