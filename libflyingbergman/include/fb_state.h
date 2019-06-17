#pragma once

void _fb_enter_state(struct fb *self,
                            void (*fp)(struct fb *self, float dt));
void _fb_state_operational(struct fb *self, float dt);
void _fb_state_operational_enter(struct fb *self);
void _fb_state_wait_home(struct fb *self, float dt);
void _fb_state_wait_power(struct fb *self, float dt);
void _fb_state_save_preset(struct fb *self, float dt);
