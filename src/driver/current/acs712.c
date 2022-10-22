/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015-2022, Martin K. Schröder, All Rights Reserved
 * Copyright (c) Davide Gironi
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

/*
 * get the current
 * current = (V - Vref/2) / sensitivity
 */

#include <arch/soc.h>

#include "acs712.h"

float acs712_read_current(uint8_t adc_chan, float sensitivity, float vcc_volt)
{
	(void)(adc_chan);
	uint16_t adc_value = adc0_read_immediate_ref(adc_chan, ADC_REF_AVCC_CAP_AREF);
	float volt = ((float)(adc_value) / (float)(65535)) * vcc_volt;
	return (volt - (vcc_volt / 2)) / sensitivity;
}
