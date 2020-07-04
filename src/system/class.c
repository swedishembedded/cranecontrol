/*
 * Copyright (C) 2017 Martin K. Schröder <mkschreder.uk@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <libfdt/libfdt.h>

#include "driver.h"
#include "adc.h"
#include "analog.h"
#include "baro.h"
#include "display.h"
#include "encoder.h"
#include "i2c.h"
#include "imu.h"
#include "leds.h"
#include "memory.h"
#include "pointer.h"
#include "serial.h"
#include "spi.h"

#include <errno.h>

DEFINE_DEVICE_CLASS(adc)
DEFINE_DEVICE_CLASS(analog)
DEFINE_DEVICE_CLASS(baro)
DEFINE_DEVICE_CLASS(display)
DEFINE_DEVICE_CLASS(encoder)
DEFINE_DEVICE_CLASS(i2c)
DEFINE_DEVICE_CLASS(imu)
DEFINE_DEVICE_CLASS(leds)
DEFINE_DEVICE_CLASS(memory)
DEFINE_DEVICE_CLASS(pointer)
DEFINE_DEVICE_CLASS(serial)
DEFINE_DEVICE_CLASS(spi)
