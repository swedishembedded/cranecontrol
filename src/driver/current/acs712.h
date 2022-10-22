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

#pragma once

#include <arch/soc.h>

//defined sensitivity
#define ACS712_SENSITIVITY5 0.185
#define ACS712_SENSITIVITY20 0.100
#define ACS712_SENSITIVITY30 0.066

//setup sensitivity
#define ACS712_SENSITIVITY ACS712_SENSITIVITY30

float acs712_read_current(uint8_t adc_chan, float sensitivity, float vcc_volt);
