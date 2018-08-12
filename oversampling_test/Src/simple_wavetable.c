/*
 * simple_wavetable.c
 *
 *  Created on: Aug 8, 2018
 *      Author: willmitchell
 */

#include "simple_wavetable.h"
#include "via_rev5_hardware_io.h"
#include "tables.h"
#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"
#include "pwm_tables.h"

uint32_t phase;

uint32_t phaseModPWMTables[33][65] = {phaseModPWM_0, phaseModPWM_1, phaseModPWM_2, phaseModPWM_3, phaseModPWM_4, phaseModPWM_5, phaseModPWM_6, phaseModPWM_7, phaseModPWM_8, phaseModPWM_9, phaseModPWM_10, phaseModPWM_11, phaseModPWM_12, phaseModPWM_13, phaseModPWM_14, phaseModPWM_15, phaseModPWM_16, phaseModPWM_17, phaseModPWM_18, phaseModPWM_19, phaseModPWM_20, phaseModPWM_21, phaseModPWM_22, phaseModPWM_23, phaseModPWM_24, phaseModPWM_25, phaseModPWM_26, phaseModPWM_27, phaseModPWM_28, phaseModPWM_29, phaseModPWM_30, phaseModPWM_31, phaseModPWM_32};


void oscillatorInit(void) {

	  for (int i = 0; i < 256; i++) {
		  wavetable1[i] = __USAT(tenor257Atk0[i] >> 3, 12);
	  }

	  for (int i = 0; i < 257; i++) {
		  wavetable1[255 + i] = __USAT(tenor257Rls0[256 - i] >> 3, 12);
	  }

	  wavetable1[512] = tenor257Atk0[0];

	  for (int i = 0; i < 256; i++) {
		  wavetable2[i] = __USAT(sinwavefold_257_Family0[i] >> 3, 12);
	  }

	  for (int i = 0; i < 257; i++) {
		  wavetable2[255 + i] = __USAT(sinwavefold_257_Family0[256 - i] >> 3, 12);
	  }

	  wavetable2[512] = sinwavefold_257_Family0[0];

	  SH_A_TRACK;
	  SH_B_TRACK;

}


void renderLine1(controlRateInputs * controls) {

	GPIOC->BRR = (uint32_t)GPIO_PIN_13;

	uint32_t increment = (((controls->timeBase1) * (cv2[0])));
	uint32_t morph = __USAT(controls->morphBase + cv3[0], 16);
	uint32_t pwm = controls->timeBase2;
	uint32_t pwmIndex = (pwm >> 7);
	uint32_t * pwmTable1 = phaseModPWMTables[pwmIndex];
	uint32_t * pwmTable2 = phaseModPWMTables[pwmIndex + 1];
	uint32_t pwmFrac = (pwm & 0b00000000000000001111111) << 9;

	uint32_t leftSample;
	uint32_t interp1;
	uint32_t interp2;
	uint32_t result;

#define pwmPhaseFrac (phase & 0x3FFFFFF) >> 9
#define phaseFrac (result & 0x7FFFFF) >> 7


	for (int i = 0; i < 16; i++) {
		// wraps at full scale uint32_t
		phase = (phase + increment);
		leftSample = phase >> 26;
		interp1 = fix16_lerp(pwmTable1[leftSample], pwmTable2[leftSample], pwmFrac);
		interp2 = fix16_lerp(pwmTable1[leftSample + 1], pwmTable2[leftSample + 1], pwmFrac);
		result = fix16_lerp(interp1, interp2, pwmPhaseFrac) << 7;
		dacBuffer3[i] = result >> 20;
		result = phase;
		leftSample = result >> 23;
		interp1 = fast_16_16_lerp(wavetable1[leftSample], wavetable2[leftSample], morph);
		interp2 = fast_16_16_lerp(wavetable1[leftSample + 1], wavetable2[leftSample + 1], morph);
		result = fast_16_16_lerp(interp1, interp2, phaseFrac);
		dacBuffer1[i] = 4095 - result;
		dacBuffer2[i] = result;
	}
	GPIOC->BSRR = (uint32_t)GPIO_PIN_13;


}

void renderLine2(controlRateInputs * controls) {

	uint32_t increment = (((controls->timeBase1) * (cv2[0])));
	uint32_t morph = __USAT(controls->morphBase + cv3[0], 16);
	uint32_t pwm = controls->timeBase2;
	uint32_t pwmIndex = (pwm >> 7);
	uint32_t * pwmTable1 = phaseModPWMTables[pwmIndex];
	uint32_t * pwmTable2 = phaseModPWMTables[pwmIndex + 1];
	uint32_t pwmFrac = (pwm & 0b00000000000000001111111) << 9;

	uint32_t leftSample;
	uint32_t interp1;
	uint32_t interp2;
	uint32_t result;

#define pwmPhaseFrac (phase & 0x3FFFFFF) >> 9
#define phaseFrac (result & 0x7FFFFF) >> 7

	GPIOC->BRR = (uint32_t)GPIO_PIN_13;

	for (int i = 16; i < 32; i++) {
		// wraps at full scale uint32_t
		phase = (phase + increment);
		leftSample = phase >> 26;
		interp1 = fix16_lerp(pwmTable1[leftSample], pwmTable2[leftSample], pwmFrac);
		interp2 = fix16_lerp(pwmTable1[leftSample + 1], pwmTable2[leftSample + 1], pwmFrac);
		result = fix16_lerp(interp1, interp2, pwmPhaseFrac) << 7;
		dacBuffer3[i] = result >> 20;
		result = phase;
		leftSample = result >> 23;
		interp1 = fast_16_16_lerp(wavetable1[leftSample], wavetable2[leftSample], morph);
		interp2 = fast_16_16_lerp(wavetable1[leftSample + 1], wavetable2[leftSample + 1], morph);
		result = fast_16_16_lerp(interp1, interp2, phaseFrac);
		dacBuffer1[i] = 4095 - result;
		dacBuffer2[i] = result;
	}
	GPIOC->BSRR = (uint32_t)GPIO_PIN_13;

}


/**
 *
 * ADC Averaging
 *
 */

void writeBuffer(buffer* buffer, int value) {
	buffer->buff[(buffer->writeIndex++) & 31] = value;
}

int readBuffer(buffer* buffer, int Xn) {
	return buffer->buff[(buffer->writeIndex + (~Xn)) & 31];
}

void handleConversionSlow(controlRateInputs * controls) {

	// TODO apply SIMD instructions?

	static uint32_t knob1Sum;
	static uint32_t knob2Sum;
	static uint32_t knob3Sum;
	static uint32_t cv1Sum;
	static buffer knob1Buffer;
	static buffer knob2Buffer;
	static buffer knob3Buffer;
	static buffer cv1Buffer;
	uint32_t knob1Value;
	uint32_t knob2Value;
	uint32_t knob3Value;
	uint32_t cv1Value;


		// implement a running average on the control rate CV controls
		knob1Sum = knob1 + knob1Sum - readBuffer(&knob1Buffer, 31);
		knob2Sum = knob2 + knob2Sum - readBuffer(&knob2Buffer, 31);

		// store the newest value in a ring buffer
		writeBuffer(&knob1Buffer, knob1);
		writeBuffer(&knob2Buffer, knob2);

		// write the averaged controls to the holding struct
		knob1Value = knob1Sum >> 5;
		knob2Value = knob2Sum >> 5;

		// implement a running average on the control rate CV controls
		knob3Sum = knob3 + knob3Sum - readBuffer(&knob3Buffer, 31);
		cv1Sum = cv1 + cv1Sum - readBuffer(&cv1Buffer, 31);

		// store the newest value in a ring buffer
		writeBuffer(&knob3Buffer, knob3);
		writeBuffer(&cv1Buffer, cv1);

		// write the averaged controls to the holding struct
		knob3Value = knob3Sum >> 5;
		cv1Value = cv1Sum >> 5;

		controls->timeBase1 = knob1Value;
		controls->timeBase2 = knob2Value;
		controls->morphBase = (knob3Value & 0xFFF0) << 4;

}

