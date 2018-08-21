/*
 * simple_wavetable.h
 *
 *  Created on: Aug 8, 2018
 *      Author: willmitchell
 */

#ifndef WAVETABLE_H
#define WAVETABLE_H

#include "stm32f3xx.h"

#define BUFFER_SIZE 32

int16_t cv2[32];
int16_t cv3[32];
uint32_t adcReadings[64];
uint32_t dacBuffer1[64];
uint32_t dacBuffer2[64];
uint32_t dacBuffer3[64];

uint32_t wavetable1[513];
uint32_t wavetable2[513];


// macros for DMA array (change to array name?)

#define knob2 adcReadings[3]
#define knob3 adcReadings[1]
#define knob1 adcReadings[2]
#define cv1 (4095 - adcReadings[0])

typedef struct {
	uint32_t timeBase1;
	uint32_t timeBase2;
	uint32_t morphBase;
	uint32_t writePosition;
} controlRateInputs;

void handleConversionSlow(controlRateInputs * controls);

controlRateInputs controls;

void oscillatorInit(void);
void renderBuffer(controlRateInputs * controls, uint32_t);
void renderBuffer0(controlRateInputs * controls);
void renderBuffer1(controlRateInputs * controls);

// Circular buffer

typedef struct {
    int buff[32];
    int writeIndex;
}buffer;

// fixed point math functions

static inline int fix16_mul(int in0, int in1) {

	int lsb;
	  int msb;

	  // multiply the inputs, store the top 32 bits in msb and bottom in lsb

	  __asm ("SMULL %[result_1], %[result_2], %[input_1], %[input_2]"
	    : [result_1] "=r" (lsb), [result_2] "=r" (msb)
	    : [input_1] "r" (in0), [input_2] "r" (in1)
	  );

	  // reconstruct the result with a left shift by 16
	  // pack the bottom halfword of msb into the top halfword of the result
	  // top halfword of lsb goes into the bottom halfword of the result

	  return __ROR(__PKHBT(msb, lsb, 0), 16);

}

static inline int fast_fix15_lerp(int in0, int in1, int frac) {


	  __asm ("SMLAWB %[result_1], %[input_1], %[input_2], %[input_3]"
	    : [result_1] "=r" (in0)
	    : [input_1] "r" (in1 - in0), [input_2] "r" (frac), [input_3] "r" (in0 >> 1)
	  );


	return in0 << 1;
}

// lower quality but faster?
static inline int fix15_bilerp(int in0, int in1, int in2, int in3, int frac0, int frac1) {

	in0 = fast_fix15_lerp(in0, in1, frac0);
	in2 = fast_fix15_lerp(in2, in3, frac0);

	return fast_fix15_lerp(in0, in2, frac1);
}

//static inline int fix15_bilerp(int in0, int in1, int in2, int in3, int frac0, int frac1) {
//
//	int invFrac = 32767 - frac0;
//
//	  __asm ("SMULWB %[result_1], %[input_1], %[input_2]"
//	    : [result_1] "=r" (in0)
//	    : [input_1] "r" (in0), [input_2] "r" (invFrac)
//	  );
//
//	  __asm ("SMLAWB %[result_1], %[input_1], %[input_2], %[input_3]"
//	    : [result_1] "=r" (in0)
//	    : [input_1] "r" (in1), [input_2] "r" (frac0), [input_3] "r" (in0)
//	  );
//
//	  __asm ("SMULWB %[result_1], %[input_1], %[input_2]"
//	    : [result_1] "=r" (in2)
//	    : [input_1] "r" (in2), [input_2] "r" (invFrac)
//	  );
//
//	  __asm ("SMLAWB %[result_1], %[input_1], %[input_2], %[input_3]"
//	    : [result_1] "=r" (in2)
//	    : [input_1] "r" (in3), [input_2] "r" (frac0), [input_3] "r" (in2)
//	  );
//
//	  __asm ("SMULWB %[result_1], %[input_1], %[input_2]"
//	    : [result_1] "=r" (in0)
//	    : [input_1] "r" (in0 << 1), [input_2] "r" (32767 - frac1)
//	  );
//
//	  __asm ("SMLAWB %[result_1], %[input_1], %[input_2], %[input_3]"
//	    : [result_1] "=r" (in0)
//	    : [input_1] "r" (in2 << 1), [input_2] "r" (frac1), [input_3] "r" (in0)
//	  );
//
//	return in0 << 1;
//}

static inline int fix24_lerp(int in0, int in1, int frac) {
	return in0 + (((int64_t)(in1 - in0) * frac) >> 24);
}

// this is a decent improvement over the above for the case of 16 bit interpolation points
// no need to cast a 16bit by 16bit multiplication to 64 bit

//static inline int fast_16_16_lerp(int in0, int in1, int frac) {
//	return in0 + fast_16_16_mul(in1 - in0, frac);
//}

static inline int fast_15_16_lerp(int in0, int in1, int frac) {

	__asm ("SMLAWB %[result_1], %[input_1], %[input_2], %[input_3]"
	    : [result_1] "=r" (in0)
	    : [input_1] "r" (frac), [input_2] "r" (in1 - in0), [input_3] "r" (in0)
	  );

	return in0;
}

static inline int fast_15_16_bilerp(int in0, int in1, int in2, int in3, int frac0, int frac1) {

	__asm ("SMLAWB %[result_1], %[input_1], %[input_2], %[input_3]"
	    : [result_1] "=r" (in0)
	    : [input_1] "r" (frac0), [input_2] "r" (in1 - in0), [input_3] "r" (in0)
	  );

	__asm ("SMLAWB %[result_1], %[input_1], %[input_2], %[input_3]"
	    : [result_1] "=r" (in2)
	    : [input_1] "r" (frac0), [input_2] "r" (in3 - in2), [input_3] "r" (in2)
	  );

	__asm ("SMLAWB %[result_1], %[input_1], %[input_2], %[input_3]"
	    : [result_1] "=r" (in0)
	    : [input_1] "r" (frac1), [input_2] "r" (in2 - in0), [input_3] "r" (in0)
	  );

	return in0;
}

static inline int fast_15_16_bilerp_prediff(int in0, int in1, int frac0, int frac1) {

	__asm ("SMLAWT %[result_1], %[input_1], %[input_2], %[input_3]"
	    : [result_1] "=r" (in0)
	    : [input_1] "r" (frac0), [input_2] "r" (in0), [input_3] "r" (in0 & 0xFFFF)
	  );

	__asm ("SMLAWT %[result_1], %[input_1], %[input_2], %[input_3]"
	    : [result_1] "=r" (in1)
	    : [input_1] "r" (frac0), [input_2] "r" (in1), [input_3] "r" (in1 & 0xFFFF)
	  );

	__asm ("SMLAWB %[result_1], %[input_1], %[input_2], %[input_3]"
	    : [result_1] "=r" (in0)
	    : [input_1] "r" (frac1), [input_2] "r" (in1 - in0), [input_3] "r" (in0)
	  );

	return in0;
}


#endif