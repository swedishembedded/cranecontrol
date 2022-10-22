/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015, Martin K. Schröder, All Rights Reserved
 * Copyright (c) John Ihlein
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#pragma once

struct mpu6000 {
	serial_port_t port;
	pio_dev_t gpio;
	gpio_pin_t cs_pin;
};

void mpu6000_init(struct mpu6000 *self, serial_dev_t port, pio_dev_t gpio, gpio_pin_t cs_pin);
uint8_t mpu6000_probe(struct mpu6000 *self);

void mpu6000_readRawAcc(struct mpu6000 *self, int16_t *ax, int16_t *ay, int16_t *az);
void mpu6000_readRawGyr(struct mpu6000 *self, int16_t *gx, int16_t *gy, int16_t *gz);
void mpu6000_convertAcc(struct mpu6000 *self, int16_t ax, int16_t ay, int16_t az, float *axg,
			float *ayg, float *azg);
void mpu6000_convertGyr(struct mpu6000 *self, int16_t gx, int16_t gy, int16_t gz, float *gxd,
			float *gyd, float *gyz);
/*
void mpu6000_getRawData(struct mpu6000 *self, int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);
void mpu6000_convertData(struct mpu6000 *self, 
	int16_t ax, int16_t ay, int16_t az, 
	int16_t gx, int16_t gy, int16_t gz, 
	float *axg, float *ayg, float *azg, 
	float *gxd, float *gyd, float *gyz
); */
//void mpu6000_getConvAcc(struct mpu6000 *self, double* axg, double* ayg, double* azg);
//void mpu6000_getConvGyr(struct mpu6000 *self, double* gxds, double* gyds, double* gzds);
