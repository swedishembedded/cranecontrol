#include <memory.h>
#include "system.h"

void system_init(struct system *self)
{
	memset(self, 0, sizeof(*self));
}
