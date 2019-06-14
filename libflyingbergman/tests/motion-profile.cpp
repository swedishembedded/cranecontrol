/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/

extern "C" {
#include "motion_profile.h"

static uint32_t _usec = 0;
timestamp_t timestamp(){
	return (timestamp_t){ .sec = _usec / 1000000U, .usec = _usec % 1000000U };
}
}

class MotionProfileTest : public ::testing::Test{
protected:
	struct motion_profile prof;
	void SetUp(){
		_usec = 0;
		motion_profile_init(&prof, 1, 1, 1);
	}
	void TearDown(){
	}
};

TEST_F(MotionProfileTest, inital_state){
	float pos = 1, vel = 2, acc = 3;
	EXPECT_TRUE(motion_profile_completed(&prof, 0));
	EXPECT_EQ(motion_profile_get_state(&prof), MOTION_PROFILE_COMPLETED);
	EXPECT_LT(motion_profile_get_pva(&prof, 0, &pos, &vel, &acc), 0);
	// make sure values were not changed
	EXPECT_FLOAT_EQ(pos, 1.0f);
	EXPECT_FLOAT_EQ(vel, 2.0f);
	EXPECT_FLOAT_EQ(acc, 3.0f);
}

TEST_F(MotionProfileTest, start_stop_from_zero_forward){
	float pos = 0.f, vel = 0.f, acc = 0.f;
	motion_profile_init(&prof, 0.5, 1, 0.5);
	motion_profile_plan_move(&prof, 0.f, 0, 3.f, 0);
	EXPECT_EQ(motion_profile_get_pva(&prof, 0, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, 0.f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 1, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, 0.25f);
	EXPECT_FLOAT_EQ(vel, 0.5f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 2, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, 1.f);
	EXPECT_FLOAT_EQ(vel, 1.f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 3, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, 2.f);
	EXPECT_FLOAT_EQ(vel, 1.f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 4, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, 2.75f);
	EXPECT_FLOAT_EQ(vel, 0.5f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 5, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, 3.f);
	EXPECT_FLOAT_EQ(vel, 0.f);
}

TEST_F(MotionProfileTest, start_stop_from_zero_backward){
	float pos = 0.f, vel = 0.f, acc = 0.f;
	motion_profile_init(&prof, 0.5, 1, 0.5);
	motion_profile_plan_move(&prof, 0, 0, -3.f, 0);
	EXPECT_EQ(motion_profile_get_pva(&prof, 0, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, 0.f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 1, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -0.25f);
	EXPECT_FLOAT_EQ(vel, -0.5f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 2, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -1.f);
	EXPECT_FLOAT_EQ(vel, -1.f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 3, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -2.f);
	EXPECT_FLOAT_EQ(vel, -1.f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 4, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -2.75f);
	EXPECT_FLOAT_EQ(vel, -0.5f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 5, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -3.f);
	EXPECT_FLOAT_EQ(vel, -0.f);
}

TEST_F(MotionProfileTest, cross_position_zero){
	float pos = -1.5f, vel = 0.f, acc = 0.f;
	motion_profile_init(&prof, 0.5, 1, 0.5);
	motion_profile_plan_move(&prof, -1.5f, 0, 1.5f, 0);
	EXPECT_EQ(motion_profile_get_pva(&prof, 0, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -1.5f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 1, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -1.5 + 0.25f);
	EXPECT_FLOAT_EQ(vel, 0.5f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 2, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -1.5 + 1.f);
	EXPECT_FLOAT_EQ(vel, 1.f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 3, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -1.5 + 2.f);
	EXPECT_FLOAT_EQ(vel, 1.f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 4, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -1.5 + 2.75f);
	EXPECT_FLOAT_EQ(vel, 0.5f);

	EXPECT_EQ(motion_profile_get_pva(&prof, 5, &pos, &vel, &acc), 0);
	EXPECT_FLOAT_EQ(pos, -1.5 + 3.f);
	EXPECT_FLOAT_EQ(vel, 0.f);
}

int main(int argc, char **argv){
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
