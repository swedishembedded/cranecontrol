#include <stdio.h>
#include <stdlib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

extern "C" {
#include "control/motion_profile.h"
}

class MotionProfileTest : public ::testing::Test {
protected:
    virtual void SetUp() {
    }
	struct motion_profile curve;
};

TEST_F(MotionProfileTest, TestPositivePlan){
	motion_profile_init(&curve, 2.f, 4.0f, 2.f);
	motion_profile_plan_move(&curve, 0.f, 0.f, 12.f, 0.0f);
	
	printf("plan 0: %f %f\n", curve.t1, curve.d1);
	printf("plan 1: %f %f\n", curve.t2, curve.d2);
	printf("plan 2: %f %f\n", curve.t3, curve.d3);

	struct {
		float time;
		float vel;
		float pos;
		float acc;
	} desired[] = {
		{ .time = 0, .vel = 0.0f, .pos = 0.0, .acc = 2 },
		{ .time = 0.1, .vel = 0.2f, .pos = 0.01, .acc = 2 },
		{ .time = 1, .vel = 2.f, .pos = 1.f, .acc = 2 },
		{ .time = 2, .vel = 4.f, .pos = 4.f, .acc = 0 },
		{ .time = 3, .vel = 4.f, .pos = 8.f, .acc = -2 },
		{ .time = 4, .vel = 2.0, .pos = 11.0, .acc = -2 },
		{ .time = 5, .vel = 0.0, .pos = 12.0, .acc = 0 }
	};
	for(size_t c = 0; c < sizeof(desired) / sizeof(desired[0]); c++){
		float pos, vel, acc;
		motion_profile_get_pva(&curve, desired[c].time, &pos, &vel, &acc);
		printf("test %d: pos: %f, vel: %f, acc: %f\n", (int)c, pos, vel, acc);
		EXPECT_NEAR(desired[c].pos, pos, 0.0001);
		EXPECT_NEAR(desired[c].vel, vel, 0.0001);
		EXPECT_NEAR(desired[c].acc, acc, 0.0001);
	}
}

TEST_F(MotionProfileTest, TestNegativePlan){
	motion_profile_init(&curve, 2.f, 4.f, 2.f);
	motion_profile_plan_move(&curve, 0.0, 0.f, -12.f, 0.0f);

	struct {
		float time;
		float vel;
		float pos;
		float acc;
	} negmoves[] = {
		{ .time = 0, .vel = 0.0f, .pos = 0.0, .acc = -2 },
		{ .time = 0.1, .vel = -0.2f, .pos = -0.01, .acc = -2 },
		{ .time = 1, .vel = -2.f, .pos = -1.f, .acc = -2 },
		{ .time = 2, .vel = -4.f, .pos = -4.f, .acc = 0 },
		{ .time = 3, .vel = -4.f, .pos = -8.f, .acc = 2 },
		{ .time = 4, .vel = -2.0, .pos = -11.0, .acc = 2 },
		{ .time = 5, .vel = -0.0, .pos = -12.0, .acc = 0 }
	};
	
	printf("plan 0: %f %f\n", curve.t1, curve.d1);
	printf("plan 1: %f %f\n", curve.t2, curve.d2);
	printf("plan 2: %f %f\n", curve.t3, curve.d3);
	for(size_t c = 0; c < sizeof(negmoves) / sizeof(negmoves[0]); c++){
		float pos, vel, acc;
		motion_profile_get_pva(&curve, negmoves[c].time, &pos, &vel, &acc);
		printf("test %d: pos: %f, vel: %f, acc: %f\n", (int)c, pos, vel, acc);
		EXPECT_NEAR(negmoves[c].pos, pos, 0.0001);
		EXPECT_NEAR(negmoves[c].vel, vel, 0.0001);
		EXPECT_NEAR(negmoves[c].acc, acc, 0.0001);
	}
}

int main(int argc, char **argv) {
        ::testing::InitGoogleMock(&argc, argv);
        return RUN_ALL_TESTS();
}

