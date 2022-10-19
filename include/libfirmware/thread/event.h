#pragma once

struct thread_event {
	void *ev;
};

int thread_evt_init(struct thread_event *self);
int thread_evt_set_from_isr(struct thread_event *self, uint32_t bits, int32_t *wait);
int thread_evt_set(struct thread_event *self, uint32_t bits);
uint32_t thread_evt_wait(struct thread_event *self, uint32_t mask, uint32_t timeout);
