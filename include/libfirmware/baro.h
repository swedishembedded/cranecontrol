#pragma once

#include "driver.h"
#include "types/list.h"

typedef const struct baro_device_ops **baro_device_t;

enum { BARO_HAS_TEMPERATURE = 1, BARO_HAS_PRESSURE = (1 << 1) };

struct baro_reading {
	uint8_t caps;
	float temperature;
	float pressure;
};

struct baro_device_ops {
	int (*read)(baro_device_t dev, struct baro_reading *data);
};

#define baro_read(baro, data) (*(baro))->read(baro, data)

DECLARE_DEVICE_CLASS(baro)
