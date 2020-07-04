/** :ms-top-comment
 *  _     ___ ____  _____ ___ ____  __  ____        ___    ____  _____
 * | |   |_ _| __ )|  ___|_ _|  _ \|  \/  \ \      / / \  |  _ \| ____|
 * | |    | ||  _ \| |_   | || |_) | |\/| |\ \ /\ / / _ \ | |_) |  _|
 * | |___ | || |_) |  _|  | ||  _ <| |  | | \ V  V / ___ \|  _ <| |___
 * |_____|___|____/|_|   |___|_| \_\_|  |_|  \_/\_/_/   \_\_| \_\_____|
 *
 * Copyright (c) 2020, Martin K. Schröder, All Rights Reserved
 *
 * This library is distributed under LGPLv2
 *
 * Commercial licensing: http://swedishembedded.com/code
 * Contact: info@swedishembedded.com
 **/
#include "control/integrator.h"

void integrator_init(struct integrator *self, float gain, float min, float max){
	self->state = 0;
	self->gain = gain;
	self->min = min;
	self->max = max;
}

float integrator_update(struct integrator *self, float input){
	self->state += self->gain * input;
	if(self->state > self->max) self->state = self->max;
	if(self->state < self->min) self->state = self->min;
	return self->state;
}
