#include "kernel.h"
#include "thread/event.h"

int thread_evt_init(struct thread_event *self)
{
	if ((self->ev = xEventGroupCreate()) == NULL)
		return -1;
	return 0;
}

int thread_evt_set_from_isr(struct thread_event *self, uint32_t bits, int32_t *wake)
{
	if (xEventGroupSetBitsFromISR(self->ev, bits, wake) == pdPASS) {
		return 0;
	}
	return -1;
}

int thread_evt_set(struct thread_event *self, uint32_t bits)
{
	if (xEventGroupSetBits(self->ev, bits) == pdPASS) {
		return 0;
	}
	return -1;
}

uint32_t thread_evt_wait(struct thread_event *self, uint32_t mask, uint32_t timeout)
{
	return xEventGroupWaitBits(self->ev, mask, 1, 0, timeout);
}
