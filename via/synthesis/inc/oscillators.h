/*
 * oscillators.h
 *
 *  Created on: Aug 18, 2018
 *      Author: willmitchell
 */

#ifndef INC_OSCILLATORS_H_
#define INC_OSCILLATORS_H_

#include "via_platform_binding.h"
#include "tables.h"

/*
 *
 * Oscillators
 *
 */

typedef struct {
	int16_t * fm;
	int16_t * pm;
	int16_t * pwm;
	int16_t * morphMod;
	int frequencyBase;
	int pmBase;
	int pwmBase;
	int morphBase;
	int syncInput;
	int reverseInput;
	int syncMode;
	int shOn;
	int tableSize;
} oversampledWavetableParameters;

void oversampledWavetableParseControls(controlRateInputs * controls, oversampledWavetableParameters * parameters);

uint32_t oversampledWavetableAdvance(oversampledWavetableParameters * parameters, audioRateOutputs * outputs, uint32_t * wavetable, uint32_t * phaseDistTable, uint32_t writePosition, uint32_t bufferSize);

void oversampledWavetableParsePhase(uint32_t phase, oversampledWavetableParameters * parameters, audioRateOutputs *output);

/*
 *
 * Shared resources
 *
 */

#endif /* INC_OSCILLATORS_H_ */