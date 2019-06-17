/** :ms-top-comment
 *  _____ _       _             ____
 * |  ___| |_   _(_)_ __   __ _| __ )  ___ _ __ __ _ _ __ ___   __ _ _ __
 * | |_  | | | | | | '_ \ / _` |  _ \ / _ \ '__/ _` | '_ ` _ \ / _` | '_ \
 * |  _| | | |_| | | | | | (_| | |_) |  __/ | | (_| | | | | | | (_| | | | |
 * |_|   |_|\__, |_|_| |_|\__, |____/ \___|_|  \__, |_| |_| |_|\__,_|_| |_|
 *          |___/         |___/                |___/
 **/
#include "fb.h"

//! this function constrains the demanded pitch and yaw output to be compliant with
//! controls for speed and intensity
void fb_output_limited(struct fb *self, float demand_pitch,
                                   float demand_yaw, float pitch_acc, float yaw_acc) {
	float dt = FB_DEFAULT_DT;
	demand_pitch = constrain_float(demand_pitch, -1.f, 1.f);
	demand_yaw = constrain_float(demand_yaw, -1.f, 1.f);

	float pitch = self->output.pitch;
	if(pitch < demand_pitch)
		pitch = constrain_float(pitch + pitch_acc * dt, pitch, demand_pitch);
	else if(pitch > demand_pitch)
		pitch = constrain_float(pitch - pitch_acc * dt, demand_pitch, pitch);

	float yaw = self->output.yaw;
	if(yaw < demand_yaw)
		yaw = constrain_float(yaw + yaw_acc * dt, yaw, demand_yaw);
	else if(yaw > demand_yaw)
		yaw = constrain_float(yaw - yaw_acc * dt, demand_yaw, yaw);

	float pitch_speed = self->measured.pitch_speed / FB_PITCH_MAX_VEL;
	float yaw_speed = self->measured.yaw_speed / FB_YAW_MAX_VEL;

	pitch = constrain_float(pitch, -pitch_speed, pitch_speed);
	yaw = constrain_float(yaw, -yaw_speed, yaw_speed);

	// limit pitch if measured pitch is outside of allowed limit
	if(self->measured.pitch > self->config.axis[FB_AXIS_UPDOWN].limits.pos_max) {
		pitch = constrain_float(pitch, 0, 1.f);
	} else if(self->measured.pitch <
	          self->config.axis[FB_AXIS_UPDOWN].limits.pos_min) {
		pitch = constrain_float(pitch, -1.f, 0);
	}

	// write final output value
	self->output.pitch = pitch;
	self->output.yaw = yaw;
}
