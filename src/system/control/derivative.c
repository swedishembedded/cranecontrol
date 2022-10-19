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
#include "control/derivative.h"

void derivative_init(struct derivative *self, float pole, float gain)
{
	self->pgain = 1.f - pole;
	self->dgain = self->pgain * gain;
}

float derivative_update(struct derivative *self, float input)
{
	float ret = self->dgain * (input - self->state);
	// x(n + 1) = x(n) - x(n - 1);
	self->state = self->pgain * (input - self->state);
	return ret;
}
