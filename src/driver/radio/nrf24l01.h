/**
 * SPDX-License-Identifier: GPLv3
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

/*
	Contributions:
	* Davide Gironi - developing original driver
	* This library is based upon nRF24L01 avr lib by Stefan Engelke
    http://www.tinkerer.eu/AVRLib/nRF24L01
  * and arduino library 2011 by J. Coliz
    http://maniacbug.github.com/RF24
*/
#ifndef _NRF24L01_H_
#define _NRF24L01_H_

#ifdef __cplusplus
extern "C" {
#endif

struct nrf24l01 {
	serial_dev_t spi;
	gpio_pin_t cs_pin;
	gpio_pin_t ce_pin;
};

#define NRF24L01_MAX_CHANNEL 128

//power setup
#define NRF24L01_RF24_PA_MIN 1
#define NRF24L01_RF24_PA_LOW 2
#define NRF24L01_RF24_PA_HIGH 3
#define NRF24L01_RF24_PA_MAX 4
#define NRF24L01_RF24_PA NRF24L01_RF24_PA_MAX

//speed setup
#define NRF24L01_RF24_SPEED_250KBPS 1
#define NRF24L01_RF24_SPEED_1MBPS 2
#define NRF24L01_RF24_SPEED_2MBPS 3
#define NRF24L01_RF24_SPEED NRF24L01_RF24_SPEED_1MBPS

//crc setup
#define NRF24L01_RF24_CRC_DISABLED 1
#define NRF24L01_RF24_CRC_8 2
#define NRF24L01_RF24_CRC_16 3
#define NRF24L01_RF24_CRC NRF24L01_RF24_CRC_16

//transmission channel
#define NRF24L01_CH 54

//payload lenght
#define NRF24L01_PAYLOAD 32

//auto ack enabled
#define NRF24L01_ACK 0

//auto retransmit delay and count
#define NRF24L01_RETR 0xff //1500uS, 15 times

//enable / disable pipe
#define NRF24L01_ENABLEDP0 1 //pipe 0
#define NRF24L01_ENABLEDP1 1 //pipe 1
#define NRF24L01_ENABLEDP2 0 //pipe 2
#define NRF24L01_ENABLEDP3 0 //pipe 3
#define NRF24L01_ENABLEDP4 0 //pipe 4
#define NRF24L01_ENABLEDP5 0 //pipe 5

//address size
#define NRF24L01_ADDRSIZE 5

//pipe address
/*
#define NRF24L01_ADDRP0 {0xE8, 0xE8, 0xF0, 0xF0, 0xE2} //pipe 0, 5 byte address
#define NRF24L01_ADDRP1 {0xC1, 0xC2, 0xC2, 0xC2, 0xC2} //pipe 1, 5 byte address
#define NRF24L01_ADDRP2 {0xC1, 0xC2, 0xC2, 0xC2, 0xC3} //pipe 2, 5 byte address
#define NRF24L01_ADDRP3 {0xC1, 0xC2, 0xC2, 0xC2, 0xC4} //pipe 3, 5 byte address
#define NRF24L01_ADDRP4 {0xC1, 0xC2, 0xC2, 0xC2, 0xC5} //pipe 4, 5 byte address
#define NRF24L01_ADDRP5 {0xC1, 0xC2, 0xC2, 0xC2, 0xC6} //pipe 5, 5 byte address
#define NRF24L01_ADDRTX {0xE8, 0xE8, 0xF0, 0xF0, 0xE2} //tx default address
*/
//enable print info function
#define NRF24L01_PRINTENABLE 0

extern void nrf24l01_init(struct nrf24l01 *nrf, serial_dev_t spi, gpio_pin_t cs, gpio_pin_t ce);
extern uint8_t nrf24l01_getstatus(struct nrf24l01 *nrf);
extern uint8_t nrf24l01_readready(struct nrf24l01 *nrf, uint8_t *pipe);
extern void nrf24l01_read(struct nrf24l01 *nrf, uint8_t *data);
extern uint8_t nrf24l01_write(struct nrf24l01 *nrf, uint8_t *data);
extern void nrf24l01_setrxaddr(struct nrf24l01 *nrf, uint8_t channel, uint8_t *addr);
extern void nrf24l01_settxaddr(struct nrf24l01 *nrf, uint8_t *addr);
void nrf24l01_powerdown(struct nrf24l01 *nrf);
void nrf24l01_scan(struct nrf24l01 *nrf, uint8_t iterations, uint8_t result[NRF24L01_MAX_CHANNEL]);

#ifdef __cplusplus
}
#endif

#endif
