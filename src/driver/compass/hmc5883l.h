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

#ifndef HMC5883L_H_
#define HMC5883L_H_

#ifdef __cplusplus
extern "C" {
#endif

//definitions
#define HMC5883L_ADDR (0x1E << 1) //device address

struct hmc5883l {
	i2c_dev_t i2c;
	uint8_t addr;
	float scale;
};

//functions
void hmc5883l_init(struct hmc5883l *self, i2c_dev_t i2c, uint8_t addr);
void hmc5883l_readRawMag(struct hmc5883l *self, int16_t *mxraw, int16_t *myraw, int16_t *mzraw);
void hmc5883l_read_adjusted(struct hmc5883l *self, float *mx, float *my, float *mz);
void hmc5883l_convertMag(struct hmc5883l *self, int16_t mxraw, int16_t myraw, int16_t mzraw,
			 float *mx, float *my, float *mz);
uint32_t hmc5883l_read_id(struct hmc5883l *self);
#ifdef __cplusplus
}
#endif

#endif
