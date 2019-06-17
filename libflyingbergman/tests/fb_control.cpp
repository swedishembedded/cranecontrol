extern "C" {
#include "fb_control.h"
#include "fb_config.h"

static float _plan_from_pos, _plan_from_vel;
static float _plan_target_pos, _plan_target_vel;
static float _plan_out_pos, _plan_out_vel, _plan_out_acc;
motion_profile_state_t _plan_state;

void motion_profile_init(struct motion_profile *self, float max_acc, float max_speed,
                         float max_dec) {
}

void motion_profile_plan_move(struct motion_profile *self, float current_pos,
                              float current_vel, float target_pos,
                              float target_vel) {
	_plan_from_pos = current_pos;
	_plan_target_pos = target_pos;
	_plan_from_vel = current_vel;
	_plan_target_vel = target_vel;
}

int motion_profile_get_pva(struct motion_profile *self, float time, float *pos_sp,
                           float *vel_sp, float *acc_sp) {
	*pos_sp = _plan_out_pos;
	*vel_sp = _plan_out_vel;
	*acc_sp = _plan_out_acc;
}

bool motion_profile_completed(struct motion_profile *self, float time) {
	return _plan_state == MOTION_PROFILE_COMPLETED;
}

motion_profile_state_t motion_profile_get_state(struct motion_profile *self) {
	return _plan_state;
}
}

class ControllerTest : public ::testing::Test {
	protected:
	struct fb_control ctrl;
	struct fb_config_control conf;
	float sim_pos, sim_vel;
	void SetUp() {
		conf.error_filter = (struct fb_config_filter){
		    .a0 = -1.9112f, .a1 = 0.914976f, .b0 = 0.0019161f, .b1 = 0.00186018f};

		conf.limits.pos_max = 0.385;
		conf.limits.pos_min = -0.560;
		conf.limits.integral_max = 1.f;

		conf.Kff = 1.f;
		conf.Kp = 18.f;
		conf.Ki = 0.0f;
		conf.Kd = 0.2f;

		fb_control_init(&ctrl, &conf);
		sim_pos = 0;
		sim_vel = 0;

		_plan_state = MOTION_PROFILE_UNINITIALIZED;
		_plan_from_pos = 0;
		_plan_from_vel = 0;

		_plan_target_pos = 0;
		_plan_target_vel = 0;
		_plan_out_pos = 0;
		_plan_out_vel = 0;
		_plan_out_acc = 0;
	}
	void TearDown() {
	}
	void clock(unsigned iterations) {
		static const float dt = 0.001;
		for(unsigned i = 0; i < iterations; i++) {
			fb_control_set_input(&ctrl, sim_pos, sim_vel);
			fb_control_clock(&ctrl);
			sim_vel = fb_control_get_output(&ctrl);
			sim_pos += sim_vel * dt;
		}
	}
};

int main(int argc, char **argv) {
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST_F(ControllerTest, plan_start) {
	EXPECT_FLOAT_EQ(_plan_target_pos, 0);
	_plan_target_vel = 1;
	fb_control_set_target(&ctrl, 3);
	clock(1);
	EXPECT_FLOAT_EQ(_plan_target_pos, 3);
	EXPECT_FLOAT_EQ(_plan_target_vel, 0);
}

TEST_F(ControllerTest, plan_start_while_in_progress) {
	fb_control_set_target(&ctrl, 1);
	clock(1);
	EXPECT_FALSE(ctrl.start);
	_plan_target_pos = 2;
	_plan_target_vel = 3;
	fb_control_set_target(&ctrl, 4);
	EXPECT_TRUE(ctrl.start);
	clock(1);
	EXPECT_TRUE(ctrl.start);
	_plan_target_pos = 2;
	EXPECT_FLOAT_EQ(_plan_target_pos, 2);
	EXPECT_FLOAT_EQ(_plan_target_vel, 3);
	EXPECT_TRUE(ctrl.start);
	EXPECT_TRUE(ctrl.moving);
	_plan_state = MOTION_PROFILE_COMPLETED;
	clock(1);
	EXPECT_TRUE(ctrl.moving);
	EXPECT_FALSE(ctrl.start);
	EXPECT_FLOAT_EQ(_plan_target_pos, 4);
	EXPECT_FLOAT_EQ(_plan_target_vel, 0);
}

TEST_F(ControllerTest, plan_move) {
	EXPECT_FLOAT_EQ(fb_control_get_output(&ctrl), 0);
	clock(100);
	EXPECT_FLOAT_EQ(fb_control_get_output(&ctrl), 0);

	conf.Kff = 1;
	fb_control_set_target(&ctrl, 3);

	clock(4900);
	EXPECT_FLOAT_EQ(fb_control_get_output(&ctrl), 0);
}
