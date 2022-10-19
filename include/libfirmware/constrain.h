#pragma once

#define _CONSTRAIN(x, a, b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))
static inline uint32_t constrain_u32(uint32_t x, uint32_t a, uint32_t b)
{
	return _CONSTRAIN(x, a, b);
}

static inline int32_t constrain_i32(int32_t x, int32_t a, int32_t b)
{
	return _CONSTRAIN(x, a, b);
}

static inline uint16_t constrain_u16(uint16_t x, uint16_t a, uint16_t b)
{
	return _CONSTRAIN(x, a, b);
}

static inline int16_t constrain_i16(int16_t x, int16_t a, int16_t b)
{
	return _CONSTRAIN(x, a, b);
}

static inline uint8_t constrain_u8(uint8_t x, uint8_t a, uint8_t b)
{
	return _CONSTRAIN(x, a, b);
}

static inline int8_t constrain_i8(int8_t x, int8_t a, int8_t b)
{
	return _CONSTRAIN(x, a, b);
}

static inline float constrain_float(float x, float a, float b)
{
	return _CONSTRAIN(x, a, b);
}
