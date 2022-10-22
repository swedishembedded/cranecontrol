/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015-2022, Martin K. Schröder, All Rights Reserved
 * Copyright (c) 2011 Davide Gironi
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#ifdef FS300A

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "fs300a.h"

/*
 * TODO: figure out how to read it without using timers directly
 *
 
volatile uint16_t fs300a_flowpulsecount = 0;
volatile uint16_t fs300a_timer0counter = 0;

ISR(TIMER0_OVF_vect) {
	//we are here every TIMER0_MSREPEATMS
	static uint8_t input_state;
	static uint16_t tflowpulsecount;
	uint8_t i;
	uint8_t current_state;
	fs300a_timer0counter++;
	//every 1sec get the pulse for seconds, hertz
	if(fs300a_timer0counter>=TIMER0_FS300ARESET) {
		fs300a_timer0counter = 0;
		fs300a_flowpulsecount = tflowpulsecount/2; //get the rise edge only
		tflowpulsecount = 0;
	}
	current_state = (FS300A_PIN & (1<<FS300A_INPUT1))>>FS300A_INPUT1; //get current state
	i = input_state ^ ~current_state; //is it changed? (using xor)
	input_state = ~current_state; //set last input state as current state
	tflowpulsecount += i; //increment count if state is changed
}

void fs300a_init(void) {
	//set timer0
	TCCR0B = TIMER0_PRESCALER;
	TIMSK0 = 1<<TOIE0; // enable timer interrupt
	//input
	FS300A_DDR &= ~(1 << FS300A_INPUT1);
}

uint16_t fs300a_getflow(void) {
	//(Pulse frequency x 60) / FS300A_CONVERSIONSENSITIVITY = flow rate
	return ((fs300a_flowpulsecount*60)/FS300A_CONVERSIONSENSITIVITY);
}
*/

#endif
