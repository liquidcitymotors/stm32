#include "tables.h"
#include "main.h"
#include "tsl_user.h"
#include "scales.h"
#include "eeprom.h"
#include "stm32f3xx_hal.h"
#include "stm32f3xx.h"
#include "stm32f3xx_it.h"
#include "interrupt_functions.h"
#include "int64.h"

extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim1;

enum syncTypes syncMode; // {none, true, hardSync, catch}
enum controlSchemes controlScheme; // {root, dutyCycle, FM, phaseMod}
enum sampleHoldModeTypes sampleHoldMode;

uint32_t time2Average;
uint32_t morphKnobAverage;
uint32_t morphCVAverage;

void getSample(uint32_t) __attribute__((section("ccmram")));
void getSampleQuinticSpline(uint32_t) __attribute__((section("ccmram")));
void getSampleLaGrange(uint32_t);
void getPhase(void)  __attribute__((section("ccmram")));
void sampHoldB(void)  __attribute__((section("ccmram")));
void sampHoldA(void)  __attribute__((section("ccmram")));
void getAverages(void)  __attribute__((section("ccmram")));

void dacISR(void) {
	uint32_t storePhase;

	// write the current contour generator value to DAC1, and its inverse to DAC2 (crossfading)
	((*(volatile uint32_t *) DAC1_ADDR) = (4095 - out));
	((*(volatile uint32_t *) DAC2_ADDR) = (out));

	// get averages for T2 and morph CV (move to the ADC interrupt??)
	getAverages();

	// store last "Phase State" (attack or release)
	storePhase = PHASE_STATE;

//		PROFILING_EVENT("DAC Updated");

	// call the function to advance the phase of the contour generator using that increment
	getPhase();

	//PROFILING_EVENT("Phase Acquired");

	// calculate morph amount per sample as a function of the morph knob and CV (move to the interrupt?)
	if ((32767 - morphCVAverage) >= 16384) {
		fixMorph = fix16_lerp(morphKnobAverage, 4095, ((32767 - morphCVAverage) - 16384) << 2);
	} else {
		fixMorph = fix16_lerp(0, morphKnobAverage, (32767 - morphCVAverage) << 2);
	}
//		fixMorph = 1000;

	// call the appropriate interpolation routine per phase in the two part table and declare phase state as such
	if (position < span) {
		CLEAR_PHASE_STATE;
		//getSampleLaGrange(0);
		//getSample(0);
		getSampleQuinticSpline(0);
	}
	if (position >= span) {
		SET_PHASE_STATE;
		//getSampleLaGrange(1);
		//getSample(1);
		getSampleQuinticSpline(1);
	}
//		PROFILING_EVENT("Sampling Complete");

	// if we transition from one phase state to another, enable the transition handler interrupt
	if ((PHASE_STATE) != storePhase) {
		HAL_NVIC_SetPendingIRQ(EXTI15_10_IRQn);
	}
//		PROFILING_STOP();
}

void getSample(uint32_t phase) {
	// use the phase position to get the sample to give to our DACs using "biinterpolation"
	// we retrieve 4 nearest-neighbor sample values and 2 fractional arguments (where are we at in between those sample values)
	// think of locating a position on a rectangular surface based upon how far you are between the bottom and top and how far you are between the left and right sides

	uint32_t LnSample;  // index of left neighbor to our position in the wavetable
	uint32_t LnFamily;  // index of left neighbor (wavetable) to our morph value in the family
	uint32_t waveFrac;  // fractional distance between our nearest neighbors in the wavetable
	uint32_t morphFrac; // fractional distance between our nearest neighbors in the family
	uint32_t Lnvalue1;  // sample values used by our two interpolations
	uint32_t Rnvalue1;
	uint32_t Lnvalue2;
	uint32_t Rnvalue2;
	uint32_t interp1;   // results of those two interpolations
	uint32_t interp2;

	// interp1 and interp2 are the interpolated values in the two adjacent wavetables per the playback position
	// out is the interpolation between those given the value of morphFrac

	if (phase == 0) {
		// we do a lot of tricky bitshifting to take advantage of the structure of a 16 bit fixed point number
		// truncate position, that sample n and n+1 are the relevant indices for our wavetables, first within the wavetable then the actual wavetables in the family
		LnSample = (position >> 16);

		// bit shifting to divide by the correct power of two takes a 12 bit number (fixMorph) and returns a quotient in the range of the family size
		LnFamily = fixMorph >> morphBitShiftRight;

		// determine the fractional component of our phase position by masking off the integer part
		waveFrac = 0x0000FFFF & position;

		// calculate the fractional component and shift up to full scale
		morphFrac = (fixMorph - (LnFamily << morphBitShiftRight)) << morphBitShiftLeft;

		// get values from the relevant wavetables
		// this is a funny looking method of referencing elements in a two dimensional array
		// we need to do it like this because our struct contains a pointer to the array being used
		// i feel like this could be optimized if we are loading from flash

//		family = currentFamily.attackFamily + LnFamily;
//		Lnvalue1 = *(*(family) + LnSample);
//		Rnvalue1 = *(*(family) + LnSample + 1);
//		Lnvalue2 = *(*(family + 1) + LnSample);
//		Rnvalue2 = *(*(family + 1) + LnSample + 1);

		// attempt at optimizing using fixed size array on the heap
		Lnvalue1 = attackHoldArray[LnFamily][LnSample];
		Rnvalue1 = attackHoldArray[LnFamily][LnSample + 1];
		Lnvalue2 = attackHoldArray[LnFamily + 1][LnSample];
		Rnvalue2 = attackHoldArray[LnFamily + 1][LnSample + 1];

		// find the interpolated values for the adjacent wavetables using an efficient fixed point linear interpolation
		interp1 = fix16_lerp(Lnvalue1, Lnvalue2, morphFrac);
		interp2 = fix16_lerp(Rnvalue1, Rnvalue2, morphFrac);

		// interpolate between those based upon the fractional part of our phase position
		out = fix16_lerp(interp1, interp2, waveFrac) >> 3;

		// compare the interpolated nearest neighbor samples to determine the sign of rate of change
		// used to generate the "delta" gate output (are we moving towards a, or towards b)
		if (interp1 < interp2) {
			REV2_GATE_HIGH;
			if (DELTAB) {
				BLOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDD_ON;
				}
			}
			if (DELTAA) {
				ALOGIC_LOW;
				if (RUNTIME_DISPLAY) {
					LEDC_OFF;
				}
			}
		} else if (interp2 < interp1) {
			REV2_GATE_LOW;
			if (DELTAB) {
				BLOGIC_LOW
				if (RUNTIME_DISPLAY) {
					LEDD_OFF;
				}
			}
			if (DELTAA) {
				ALOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDC_ON;
				}
			}
		}

		// if the runtime display is on, show our mode - timer compare thresholds are RGB LED PWM values
		if (RUNTIME_DISPLAY) {
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, out);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, fixMorph >> 2);
		}
	} else {

		//					RELEASE
		// this section is similar, but subtly different to implement "release"
		// we reflect position back over span
		LnSample = ((spanx2 - position) >> 16);
		LnFamily = fixMorph >> morphBitShiftRight;

		// here, again, we use that reflected value
		waveFrac = 0x0000FFFF & (spanx2 - position);
		morphFrac = (uint16_t) ((fixMorph - (LnFamily << morphBitShiftRight)) << morphBitShiftLeft);

		// pull the values from our "release family"
//		family = currentFamily.releaseFamily + LnFamily;
//		Lnvalue1 = *(*(family) + LnSample);
//		Rnvalue1 = *(*(family) + LnSample + 1);
//		Lnvalue2 = *(*(family + 1) + LnSample);
//		Rnvalue2 = *(*(family + 1) + LnSample + 1);

		// attempt at optimizing using fixed size array on the heap
		Lnvalue1 = releaseHoldArray[LnFamily][LnSample];
		Rnvalue1 = releaseHoldArray[LnFamily][LnSample + 1];
		Lnvalue2 = releaseHoldArray[LnFamily + 1][LnSample];
		Rnvalue2 = releaseHoldArray[LnFamily + 1][LnSample + 1];

		// find the interpolated values for the adjacent wavetables using an efficient fixed point linear interpolation
		interp1 = fix16_lerp(Lnvalue1, Lnvalue2, morphFrac);
		interp2 = fix16_lerp(Rnvalue1, Rnvalue2, morphFrac);

		// interpolate between those based upon the fractional part of our phase position
		out = fix16_lerp(interp1, interp2, waveFrac) >> 3;

		// compare the interpolated nearest neighbor samples to determine the sign of rate of change
		// used to generate the "delta" gate output (are we moving towards a, or towards b)
		if (interp1 < interp2) {
			REV2_GATE_HIGH;
			if (DELTAB) {
				BLOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDD_ON;
				}
			}
			if (DELTAA) {
				ALOGIC_LOW;
				if (RUNTIME_DISPLAY) {
					LEDC_OFF;
				}
			}
		} else if (interp2 < interp1) {
			REV2_GATE_LOW;
			if (DELTAB) {
				BLOGIC_LOW;
				if (RUNTIME_DISPLAY) {
					LEDD_OFF;
				}
			}
			if (DELTAA) {
				ALOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDC_ON;
				}
			}
		}
		// if the runtime display is on, show our mode - timer compare thresholds are RGB LED PWM values
		if (RUNTIME_DISPLAY) {
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, out);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, fixMorph >> 2);
		}
	}
}

void getSampleQuinticSpline(uint32_t phase) {

	// adapted from code shared by Josh Scholar on musicDSP.org
	// https://web.archive.org/web/20170705065209/http://www.musicdsp.org/showArchiveComment.php?ArchiveID=60

#define PRECALC1_6 2796202

	// get the 3 adjacent values on either side of the phase pointer and plug them into the quintic spline equation
	// each value must be interpolated between the two families adjacent to the morph pointer

	uint32_t LnSample;  // indicates the nearest neighbor to our position in the wavetable
	uint32_t LnFamily;  // indicates the nearest neighbor (wavetable) to our morph value in the family
	uint32_t waveFrac;  // indicates the fractional distance between our nearest neighbors in the wavetable
	uint32_t morphFrac; // indicates the fractional distance between our nearest neighbors in the family
	uint32_t lFvalue0;  // sample values used by our two interpolations in
	uint32_t rFvalue0;
	uint32_t lFvalue1;
	uint32_t rFvalue1;
	uint32_t lFvalue2;
	uint32_t rFvalue2;
	uint32_t lFvalue3;
	uint32_t rFvalue3;
	uint32_t lFvalue4;
	uint32_t rFvalue4;
	uint32_t lFvalue5;
	uint32_t rFvalue5;
	uint32_t interp0;  // results of those two interpolations
	uint32_t interp1;
	uint32_t interp2;  // results of those two interpolations
	uint32_t interp3;
	uint32_t interp4;
	uint32_t interp5;
	int calc;

	// the above is used to perform our bi-interpolation
	// essentially, interp 1 and interp 2 are the interpolated values in the two adjacent wavetables per the playback position
	// out is the "crossfade" between those according to morphFrac

	if (phase == 0) {
		// we do a lot of tricky bitshifting to take advantage of the structure of a 16 bit fixed point number
		// truncate position then add one to find the relevant indices for our wavetables, first within the wavetable then the actual wavetables in the family
		LnSample = (position >> 16) + 2;

		// bit shifting to divide by the correct power of two takes a 12 bit number (our fixMorph) and returns the a quotient in the range of our family size
		LnFamily = fixMorph >> morphBitShiftRight;

		// determine the fractional part of our phase position by masking off the integer
		waveFrac = 0x0000FFFF & position;

		// we have to calculate the fractional portion and get it up to full scale
		morphFrac = (fixMorph - (LnFamily << morphBitShiftRight)) << morphBitShiftLeft;

		// get values from the relevant wavetables
		lFvalue0 = attackHoldArray[LnFamily][LnSample - 2];
		lFvalue1 = attackHoldArray[LnFamily][LnSample - 1];
		lFvalue2 = attackHoldArray[LnFamily][LnSample];
		lFvalue3 = attackHoldArray[LnFamily][LnSample + 1];
		lFvalue4 = attackHoldArray[LnFamily][LnSample + 2];
		lFvalue5 = attackHoldArray[LnFamily][LnSample + 3];
		rFvalue0 = attackHoldArray[LnFamily + 1][LnSample- 2];
		rFvalue1 = attackHoldArray[LnFamily + 1][LnSample - 1];
		rFvalue2 = attackHoldArray[LnFamily + 1][LnSample];
		rFvalue3 = attackHoldArray[LnFamily + 1][LnSample + 1];
		rFvalue4 = attackHoldArray[LnFamily + 1][LnSample + 2];
		rFvalue5 = attackHoldArray[LnFamily + 1][LnSample + 3];

		// find the interpolated values for the adjacent wavetables using an efficient fixed point linear interpolation
		interp0 = fix16_lerp(lFvalue0, rFvalue0, morphFrac);
		interp1 = fix16_lerp(lFvalue1, rFvalue1, morphFrac);
		interp2 = fix16_lerp(lFvalue2, rFvalue2, morphFrac);
		interp3 = fix16_lerp(lFvalue3, rFvalue3, morphFrac);
		interp4 = fix16_lerp(lFvalue4, rFvalue4, morphFrac);
		interp5 = fix16_lerp(lFvalue5, rFvalue5, morphFrac);

		out = interp2
				+ fix24_mul(699051, fix16_mul(waveFrac, ((interp3-interp1)*16 + (interp0-interp4)*2
						+ fix16_mul(waveFrac, ((interp3+interp1)*16 - interp0 - interp2*30 - interp4
								+ fix16_mul(waveFrac, (interp3*66 - interp2*70 - interp4*33 + interp1*39 + interp5*7 - interp0*9
										+ fix16_mul(waveFrac, ( interp2*126 - interp3*124 + interp4*61 - interp1*64 - interp5*12 + interp0*13
												+ fix16_mul(waveFrac, ((interp3-interp2)*50 + (interp1-interp4)*25 + (interp5-interp0) * 5))
										))
								))
						))
				))
				);

		out = out >> 3;

		if (out > 4095){out = 4095;}
		else if (out < 0){out = 0;}

		// we use the interpolated nearest neighbor samples to determine the sign of rate of change
		// aka, are we moving towrds a, or towards b
		// we use this to generate our gate output
		if (interp2 < interp3) {
			EXPAND_GATE_HIGH;
			REV2_GATE_HIGH;
			if (DELTAB) {
				BLOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDD_ON;
				}
			}
			if (DELTAA) {
				ALOGIC_LOW;
				if (RUNTIME_DISPLAY) {
					LEDC_OFF;
				}
			}
		} else if (interp1 < interp2) {
			EXPAND_GATE_LOW;
			REV2_GATE_LOW;
			if (DELTAB) {
				BLOGIC_LOW;
				if (RUNTIME_DISPLAY) {
					LEDD_OFF;
				}
			}
			if (DELTAA) {
				ALOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDC_ON;
				}
			}
		}

		if (RUNTIME_DISPLAY) { // if the runtime display is on, show our mode
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, out);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, fixMorph >> 2);
		}
	}

	else {

		// this section is similar, but subtly different to implement our "release"
		// notice, we reflect position back over span
		LnSample = ((spanx2 - position) >> 16) + 2;
		LnFamily = fixMorph >> morphBitShiftRight;

		// here, again, we use that reflected value
		waveFrac = 0x0000FFFF & (spanx2 - position);
		morphFrac = (uint16_t) ((fixMorph - (LnFamily << morphBitShiftRight)) << morphBitShiftLeft);

		// pull the values from our "release family"
		//		family = currentFamily.releaseFamily + LnFamily;
		//		Lnvalue1 = *(*(family) + LnSample);
		//		Rnvalue1 = *(*(family) + LnSample + 1);
		//		Lnvalue2 = *(*(family + 1) + LnSample);
		//		Rnvalue2 = *(*(family + 1) + LnSample + 1);

		lFvalue0 = releaseHoldArray[LnFamily][LnSample - 2];
		lFvalue1 = releaseHoldArray[LnFamily][LnSample - 1];
		lFvalue2 = releaseHoldArray[LnFamily][LnSample];
		lFvalue3 = releaseHoldArray[LnFamily][LnSample + 1];
		lFvalue4 = releaseHoldArray[LnFamily][LnSample + 2];
		lFvalue5 = releaseHoldArray[LnFamily][LnSample + 3];
		rFvalue0 = releaseHoldArray[LnFamily + 1][LnSample- 2];
		rFvalue1 = releaseHoldArray[LnFamily + 1][LnSample - 1];
		rFvalue2 = releaseHoldArray[LnFamily + 1][LnSample];
		rFvalue3 = releaseHoldArray[LnFamily + 1][LnSample + 1];
		rFvalue4 = releaseHoldArray[LnFamily + 1][LnSample + 2];
		rFvalue5 = releaseHoldArray[LnFamily + 1][LnSample + 3];

		// find the interpolated values for the adjacent wavetables using an efficient fixed point linear interpolation
		interp0 = fix16_lerp(lFvalue0, rFvalue0, morphFrac);
		interp1 = fix16_lerp(lFvalue1, rFvalue1, morphFrac);
		interp2 = fix16_lerp(lFvalue2, rFvalue2, morphFrac);
		interp3 = fix16_lerp(lFvalue3, rFvalue3, morphFrac);
		interp4 = fix16_lerp(lFvalue4, rFvalue4, morphFrac);
		interp5 = fix16_lerp(lFvalue5, rFvalue5, morphFrac);

		out = interp2
				+ fix24_mul(699051, fix16_mul(waveFrac, ((interp3-interp1)*16 + (interp0-interp4)*2
						+ fix16_mul(waveFrac, ((interp3+interp1)*16 - interp0 - interp2*30 - interp4
								+ fix16_mul(waveFrac, (interp3*66 - interp2*70 - interp4*33 + interp1*39 + interp5*7 - interp0*9
										+ fix16_mul(waveFrac, ( interp2*126 - interp3*124 + interp4*61 - interp1*64 - interp5*12 + interp0*13
												+ fix16_mul(waveFrac, ((interp3-interp2)*50 + (interp1-interp4)*25 + (interp5-interp0) * 5))
										))
								))
						))
				))
				);

		out = out >> 3;

		if (out > 4095){out = 4095;}
		if (out < 0){out = 0;}

		// we use the interpolated nearest neighbor samples to determine the sign of rate of change
		// aka, are we moving towrds a, or towards b
		// we use this to generate our gate output
		if (interp1 < interp2) {
			EXPAND_GATE_HIGH;
			REV2_GATE_HIGH;
			if (DELTAB) {
				BLOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDD_ON;
				}
			}
			if (DELTAA) {
				ALOGIC_LOW;
				if (RUNTIME_DISPLAY) {
					LEDC_OFF;
				}
			}
		} else if (interp1 < interp2) {
			EXPAND_GATE_LOW;
			REV2_GATE_LOW;
			if (DELTAB) {
				BLOGIC_LOW;
				if (RUNTIME_DISPLAY) {
					LEDD_OFF;
				}
			}
			if (DELTAA) {
				ALOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDC_ON;
				}
			}
		}
		if (RUNTIME_DISPLAY) {
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, out);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, fixMorph >> 2);
		}
	}
}

// calculates an increment then checks trigger mode, making the appropriate changes to playback when the contour generator is triggered
void getPhase(void) {

	static int lastCV;
	int attackTransferHolder;
	int releaseTransferHolder;

	if (controlScheme == phaseMod) {
		position = position + ((time2CV - lastCV) << 11);
		if (position < 0) {position = 0;}
		else if (position > spanx2) {position = spanx2;}
		lastCV = time2CV;
	}

	if (controlScheme == dutyCycle)  {
			holdPosition = attackInc + holdPosition;
			if (holdPosition >= spanx2) {
				holdPosition = holdPosition - spanx2;
			}

			if (holdPosition < 0) {
				holdPosition = holdPosition + spanx2;
			}

			if (time2Average > 4094) {
				time2Average = 4094;

			} else if (time2Average < 1) {
				time2Average = 1;
			}

			if (holdPosition < (fix16_mul(spanx2, (4095 - time2Average) << 4))) {
				attackTransferHolder = (65535 << 11)/(4095 - time2Average); // 1/(T2*2)
				position = fix16_mul(holdPosition, attackTransferHolder);
	//			position = 2 * holdPosition;
			} else {
				releaseTransferHolder = (65535 << 11)/(time2Average); // 1/((1-T2)*2)
				position = fix16_mul(holdPosition, releaseTransferHolder) + spanx2 - fix16_mul(spanx2, releaseTransferHolder);
	//			position = fix16_mul(holdPosition, 43690) + spanx2 - fix16_mul(spanx2, 43690);
			}

	} else {
		if (controlScheme == FM) {
			if (PHASE_STATE) {
				position = position + ((attackInc * (4095 - time2Average)) >> 11);
				if (position < 0) {position = 0;}
				else if (position > spanx2) {position = spanx2;}
			} else {
				position = position + ((releaseInc * time2Average) >> 11);
				if (position < 0) {position = 0;}
				else if (position > spanx2) {position = spanx2;}
			}
		} else {
			if (PHASE_STATE) {
					position = position + attackInc;
			} else {
					position = position + releaseInc;
			}
		}
	}

	// if we have incremented outside of our table, wrap back around to the other side
	if (position >= spanx2) {
		position = position - spanx2;
	}

	if (position < 0) {
		position = position + spanx2;
	}

	lastCV = time2CV;
}





int fix16_mul(int in0, int in1) {
	// taken from the fixmathlib library
	int64_t result = (uint64_t) in0 * in1;
	return result >> 16;
}

int fix24_mul(int in0, int in1) {
	// taken from the fixmathlib library
	int64_t result = (uint64_t) in0 * in1;
	return result >> 24;
}

int fix48_mul(uint32_t in0, uint32_t in1) {
	// taken from the fixmathlib library
	int64_t result = (uint64_t) in0 * in1;
	return result >> 48;
}

int fix16_lerp(int in0, int in1, uint16_t inFract) {
	// taken from the fixmathlib library
	int64_t tempOut = int64_mul_i32_i32(in0, (((int32_t) 1 << 16) - inFract));
	tempOut = int64_add(tempOut, int64_mul_i32_i32(in1, inFract));
	tempOut = int64_shift(tempOut, -16);
	return (int) int64_lo(tempOut);
}

void EXTI15_10_IRQHandler(void) {
	// Handler for the rising edge at our expander aux logic input
	if(__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_11)) {
		generateFrequency();
		SET_EXTPLL;
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
	} else {

		if (!(PHASE_STATE)) {
			EXPAND_GATE_HIGH;
			REV2_GATE_LOW;

			if (TRIGA) {
				ALOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDC_ON;
				}
				__HAL_TIM_SET_COUNTER(&htim15, 0);
				__HAL_TIM_ENABLE(&htim15);
			} else if (GATEA) {
				ALOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDC_ON;
				}
			}
			if (GATEB) {
				BLOGIC_LOW;
				if (RUNTIME_DISPLAY) {
					LEDD_OFF;
				}
			}
			sampHoldA();

			if (RUNTIME_DISPLAY) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
			}

			if ((NOCLOCK) && periodCount > 0) {
				generateFrequency();
			}
		} else {

			REV2_GATE_HIGH;
			EXPAND_GATE_LOW;

			if (TRIGB) {
				BLOGIC_HIGH;
				__HAL_TIM_SET_COUNTER(&htim15, 0);
				__HAL_TIM_ENABLE(&htim15);
				if (RUNTIME_DISPLAY) {
					LEDD_ON;
				}
			} else if (GATEB) {
				BLOGIC_HIGH;
				if (RUNTIME_DISPLAY) {
					LEDD_ON;
				}
			}
			if (GATEA) {
				ALOGIC_LOW;
				if (RUNTIME_DISPLAY) {
					LEDC_OFF;
				}
			}
			sampHoldB();

			if (RUNTIME_DISPLAY) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
			}
		}
	}

}

void sampHoldB(void) {
	switch (sampleHoldMode) {

	case a:
		SH_A_TRACK;
		if (RUNTIME_DISPLAY) {
			LEDA_ON;
		}
		break;

		// case b: b remains sampled
	case ab:
		SH_A_TRACK;
		if (RUNTIME_DISPLAY) {
			LEDA_OFF;
		}
		// b remains sampled
		break;

	case antidecimate:
		SH_B_SAMPLE;
		SH_A_TRACK;
		if (RUNTIME_DISPLAY) {
			LEDB_OFF;
			LEDA_ON;
		}
		break;

	case decimate:
		SH_A_TRACK;
		__HAL_TIM_SET_COUNTER(&htim7, 0);
		__HAL_TIM_ENABLE(&htim7);
		if (RUNTIME_DISPLAY) {
					LEDA_OFF;
					LEDB_OFF;
				}
		break;

	default:
		break;
	}
}

void sampHoldA(void) {
	switch (sampleHoldMode) {

	case a:
		SH_A_SAMPLE;
		if (RUNTIME_DISPLAY) {
			LEDA_OFF;
		}
		break;

	case b:
		SH_B_TRACK;
		__HAL_TIM_SET_COUNTER(&htim8, 0);
		__HAL_TIM_ENABLE(&htim8);
		if (RUNTIME_DISPLAY) {
			LEDB_OFF;
		}
		break;

	case ab:
		SH_A_SAMPLE;
		SH_B_TRACK;
		if (RUNTIME_DISPLAY) {
			LEDB_OFF;
			LEDA_ON;
		}
		__HAL_TIM_SET_COUNTER(&htim8, 0);
		__HAL_TIM_ENABLE(&htim8);
		break;

	case antidecimate:
		SH_A_SAMPLE;
		SH_B_TRACK;
		if (RUNTIME_DISPLAY) {
			LEDA_OFF;
			LEDB_ON;
		}
		break;

	case decimate:
		SH_B_TRACK;
		__HAL_TIM_SET_COUNTER(&htim7, 0);
		__HAL_TIM_ENABLE(&htim7);
		if (RUNTIME_DISPLAY) {
			LEDA_OFF;
			LEDB_OFF;
		}
		break;

	default:
		break;
	}
}

void write(buffer* buffer, int value) {
	buffer->buff[(buffer->writeIndex++) & BUFF_SIZE_MASK] = value;
}

int readn(buffer* buffer, int Xn) {
	return buffer->buff[(buffer->writeIndex + (~Xn)) & BUFF_SIZE_MASK];
}

void getAverages(void) {

	static buffer time2CVBuffer;
	static buffer morphCVBuffer;
	static buffer morphKnobBuffer;
	static uint32_t morphKnobSum;
	static uint32_t morphCVSum;
	static uint32_t time2Sum;

	write(&time2CVBuffer, time2CV);
	time2Sum = time2Sum + time2CV- readn(&time2CVBuffer, 31);
	morphKnobSum = morphKnobSum + morphKnob - readn(&morphKnobBuffer, 255);
	morphCVSum = (morphCVSum + morphCV - readn(&morphCVBuffer, 31));

	morphKnobAverage = morphKnobSum >> 8;
	morphCVAverage = morphCVSum >> 2;
	time2Average = time2Sum >> 5;

	write(&morphCVBuffer, morphCV);
	write(&morphKnobBuffer, morphKnob);
}
