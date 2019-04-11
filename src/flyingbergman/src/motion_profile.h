#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <libfirmware/timestamp.h>

typedef enum {
	MOTION_PROFILE_COMPLETED,
	MOTION_PROFILE_ACCELERATING,
	MOTION_PROFILE_CRUISING,
	MOTION_PROFILE_DECELERATING
} motion_profile_state_t;

struct motion_profile {
	float acc;
	float dec;
	float vmax;
	struct timeval start_time;
	float sign;
	float t1, t2, t3;
	float d1, d2, d3;
	float v1, v2, v3;
	float total_time;
	float total_distance;
	float start_position;
	bool move_planned;
	motion_profile_state_t state;
};

void motion_profile_init(struct motion_profile *self, float max_acc, float max_speed, float max_dec);
void motion_profile_plan_move(struct motion_profile *self, const struct timeval *current_time, float current_pos, float current_vel, float target_pos, float target_vel);
int motion_profile_get_pva(struct motion_profile *self, const struct timeval *current_time, float *pos_sp, float *vel_sp, float *acc_sp);
bool motion_profile_completed(struct motion_profile *self, const struct timeval *current_time);
motion_profile_state_t motion_profile_get_state(struct motion_profile *self);
