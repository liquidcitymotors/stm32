/*
 * platform_signals.h
 *
 *  Created on: Aug 19, 2018
 *      Author: willmitchell
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INC_PLATFORM_SIGNALS_H_
#define INC_PLATFORM_SIGNALS_H_


#include "stm32f3xx_hal.h"
#include "stm32f3xx.h"
#include "dsp.h"
#include "via_global_signals.h"
#include <stdio.h>
#include <stdlib.h>

extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac1;
extern DAC_HandleTypeDef hdac2;
extern SDADC_HandleTypeDef hsdadc1;
extern SDADC_HandleTypeDef hsdadc2;

uint32_t controlRateADCReadings[4];

#define knob2 controlRateADCReadings[3]
#define knob3 controlRateADCReadings[1]
#define knob1 controlRateADCReadings[2]
#define cv1 (4095 - controlRateADCReadings[0])

// initialize a copy of each signal group

static inline void via_ioStreamInit(audioRateInputs * audioRateInput,
		audioRateOutputs * audioRateOutput, int bufferSize) {

	audioRateOutput->dac1Samples = (uint32_t*) malloc(2 * bufferSize * sizeof(int));
	audioRateOutput->dac2Samples = (uint32_t*) malloc(2 * bufferSize * sizeof(int));
	audioRateOutput->dac3Samples = (uint32_t*) malloc(2 * bufferSize * sizeof(int));

	audioRateInput->cv2Samples = (int16_t*) malloc(2 * bufferSize * sizeof(int));
	audioRateInput->cv3Samples = (int16_t*) malloc(2 * bufferSize * sizeof(int));
	audioRateInput->cv2VirtualGround = (int16_t*) malloc(2 * bufferSize * sizeof(int));
	audioRateInput->cv3VirtualGround = (int16_t*) malloc(2 * bufferSize * sizeof(int));

	int16_t cv2Offset = HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA0) << 2;
	int16_t cv3Offset = HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA1) << 2;

	for (int i = 0; i < 2 * bufferSize; i++) {
		audioRateInput->cv2VirtualGround[i] = cv2Offset;
		audioRateInput->cv3VirtualGround[i] = cv3Offset;
	}

	// initialize the ADCs and their respective DMA arrays
	HAL_ADC_Start_DMA(&hadc1, controlRateADCReadings, 4);

	if (HAL_SDADC_CalibrationStart(&hsdadc1, SDADC_CALIBRATION_SEQ_1)
			!= HAL_OK) {
		LEDA_ON;
	}

	/* Pool for the end of calibration */
	if (HAL_SDADC_PollForCalibEvent(&hsdadc1, 100) != HAL_OK) {
		LEDB_ON;
	}

	if (HAL_SDADC_CalibrationStart(&hsdadc2, SDADC_CALIBRATION_SEQ_1)
			!= HAL_OK) {
		LEDA_ON;
	}

	/* Pool for the end of calibration */
	if (HAL_SDADC_PollForCalibEvent(&hsdadc2, 100) != HAL_OK) {
		LEDB_ON;
	}

	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, audioRateOutput->dac1Samples,
			2 * bufferSize, DAC_ALIGN_12B_R);
	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, audioRateOutput->dac2Samples,
			2 * bufferSize, DAC_ALIGN_12B_R);
	HAL_DAC_Start_DMA(&hdac2, DAC_CHANNEL_1, audioRateOutput->dac3Samples,
			2 * bufferSize, DAC_ALIGN_12B_R);

	// set the dac sample rate and start the dac timer
	HAL_SDADC_Start_DMA(&hsdadc1, (uint32_t *) audioRateInput->cv2Samples, 2 * bufferSize);
	HAL_SDADC_Start_DMA(&hsdadc2, (uint32_t *) audioRateInput->cv3Samples, 2 * bufferSize);
//	TIM6->ARR = 1439;
//	TIM6->CR1 |= TIM_CR1_CEN;

}

static inline void via_logicStreamInit(audioRateInputs * audioRateInput,
		audioRateOutputs * audioRateOutput, int bufferSize) {

	audioRateOutput->shA = (uint32_t*) malloc(2 * bufferSize * sizeof(int));
	audioRateOutput->shB = (uint32_t*) malloc(2 * bufferSize * sizeof(int));
	audioRateOutput->logicA = (uint32_t*) malloc(2 * bufferSize * sizeof(int));
	audioRateOutput->auxLogic = (uint32_t*) malloc(2 * bufferSize * sizeof(int));

	audioRateInput->trigSamples = (int*) malloc(2 * bufferSize * sizeof(int));
	audioRateInput->auxTrigSamples = (int*) malloc(2 * bufferSize * sizeof(int));

}

// pass a 1 to runtime display to show the state of the logic outs on the leds
// pass a 0 to just set the outputs

static inline void via_setLogicOut(audioRateOutputs * outputs, int writeIndex, int runtimeDisplay) {

	if (runtimeDisplay) {
		setLogicOutputsLEDOn(outputs->logicA[writeIndex],
				outputs->auxLogic[writeIndex], outputs->shA[writeIndex], outputs->shB[writeIndex]);
	} else {
		setLogicOutputsLEDOff(outputs->logicA[writeIndex],
				outputs->auxLogic[writeIndex], outputs->shA[writeIndex], outputs->shB[writeIndex]);
	}

}

static inline void via_updateControlRateInputs(controlRateInputs * controls) {

	// TODO apply SIMD instructions?

	// store the newest value in a ring buffer
	writeBuffer(&controls->cv1Buffer, cv1);
	writeBuffer(&controls->knob1Buffer, knob1);
	writeBuffer(&controls->knob2Buffer, knob2);
	writeBuffer(&controls->knob3Buffer, knob3);

	// implement a running average on the control rate CV controls
	controls->cv1Sum = cv1 + controls->cv1Sum - readBuffer(&controls->cv1Buffer, 31);
	controls->knob1Sum = knob1 + controls->knob1Sum - readBuffer(&controls->knob1Buffer, 31);
	controls->knob2Sum = knob2 + controls->knob2Sum - readBuffer(&controls->knob2Buffer, 31);
	controls->knob3Sum = knob3 + controls->knob3Sum - readBuffer(&controls->knob3Buffer, 31);

	// write the averaged controls to the holding struct
	controls->cv1Value = controls->cv1Sum >> 5;
	controls->knob1Value = controls->knob1Sum >> 5;
	controls->knob2Value = controls->knob2Sum >> 5;
	controls->knob3Value = controls->knob3Sum >> 5;

}

#ifdef __cplusplus
}
#endif

#endif /* INC_PLATFORM_SIGNALS_H_ */