
#include "via_rev5_hardware_io.h"
#include "user_interface.h"
#include "sync_dsp.h"
#include "sync_next_sample.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;

void handleConversionSlow(viaStateVariableSet *, controlRateInputs *, audioRateInputs *, audioRateOutputs * outputs);


/**
 *
 * S&H handling functions
 *
 */



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

void handleConversionSlow(viaStateVariableSet * stateVariables, controlRateInputs * controls, audioRateInputs * inputs, audioRateOutputs * outputs) {

	// TODO apply SIMD instructions?

	static uint32_t knob1Sum;
	static uint32_t knob2Sum;
	static uint32_t knob3Sum;
	static uint32_t cv1Sum;
	static buffer knob1Buffer;
	static buffer knob2Buffer;
	static buffer knob3Buffer;
	static buffer cv1Buffer;

	static int pwmCounter;

	int sequencingSignal = stateVariables->slowConversionCounter;

	switch (sequencingSignal) {
	case 1:
		// implement a running average on the control rate CV inputs
		knob1Sum = knob1 + knob1Sum - readBuffer(&knob1Buffer, 31);
		knob2Sum = knob2 + knob2Sum - readBuffer(&knob2Buffer, 31);

		// store the newest value in a ring buffer
		writeBuffer(&knob1Buffer, knob1);
		writeBuffer(&knob2Buffer, knob2);

		// write the averaged inputs to the holding struct
		controls->knob1Value = knob1Sum >> 5;
		controls->knob2Value = knob2Sum >> 5;

		break;

	case 16:
		// implement a running average on the control rate CV inputs
		knob3Sum = knob3 + knob3Sum - readBuffer(&knob3Buffer, 31);
		cv1Sum = cv1 + cv1Sum - readBuffer(&cv1Buffer, 3);

		// store the newest value in a ring buffer
		writeBuffer(&knob3Buffer, knob3);
		writeBuffer(&cv1Buffer, cv1);

		// write the averaged inputs to the holding struct
		controls->knob3Value = knob3Sum >> 5;
		controls->cv1Value = cv1Sum >> 2;

		break;

	case 32:
		// update the runtime display if the UI menu is not turned on
		if (RUNTIME_DISPLAY) {
			updateRGBTest(stateVariables, controls, inputs, outputs);
		}
		break;

	case 48:
		stateVariables->slowConversionCounter = 0;
		break;

	default:
		break;

	}
}

/**
 *
 * Helper function for display handling
 *
 */

void updateRGBTest(viaStateVariableSet * stateVariables, controlRateInputs * controls, audioRateInputs * inputs, audioRateOutputs * outputs) {
	int sample = outputs->samples;
	int lastPhaseValue = stateVariables->phase;
	SET_RED_LED(sample * (lastPhaseValue >> 24));
	SET_BLUE_LED(sample * (!(lastPhaseValue >> 24)));
	SET_GREEN_LED(__USAT((inputs->cv3Input + controls->knob3Value - 2048), 12) * sample >> 12);
}


void updateRGBAudio(controlRateInputs * controls, audioRateInputs * inputBuffer, int lastSample, int lastPhaseValue) {
	SET_BLUE_LED(__USAT((controls->knob1Value + controls->cv1Value + (controls->knob1Value >> 4)) >> 1, 12));
	SET_GREEN_LED(__USAT((inputBuffer->cv3Input + controls->knob3Value - 2048), 12) >> 1);
	SET_RED_LED(4095 - __USAT((controls->knob1Value + controls->cv1Value + (controls->knob1Value >> 4)) >> 1, 12));
}

void updateRGBSubAudio(controlRateInputs * controls, audioRateInputs * inputBuffer, int lastSample, int lastPhaseValue) {
	SET_RED_LED(lastSample * (lastPhaseValue >> 24));
	SET_BLUE_LED(lastSample * (!(lastPhaseValue >> 24)));
	SET_GREEN_LED(__USAT((inputBuffer->cv3Input + controls->knob3Value - 2048), 12) * lastSample >> 12);
}

void updateRGBTrigger(controlRateInputs * controls, audioRateInputs * inputBuffer, int lastSample, int lastPhaseValue) {
	SET_RED_LED(4095);
	SET_BLUE_LED(4095);
	SET_GREEN_LED(4095);
}

void updateRGBBlank(controlRateInputs * controls, audioRateInputs * inputBuffer, int lastSample, int lastPhaseValue) {
	SET_RED_LED(0);
	SET_BLUE_LED(0);
	SET_GREEN_LED(0);
}




