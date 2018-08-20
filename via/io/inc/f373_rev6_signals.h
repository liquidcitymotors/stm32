/*
 * platform_signals.h
 *
 *  Created on: Aug 19, 2018
 *      Author: willmitchell
 */

#ifndef INC_PLATFORM_SIGNALS_H_
#define INC_PLATFORM_SIGNALS_H_

#include "stm32f3xx_hal.h"
#include "stm32f3xx.h"
#include "dsp.h"

extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac1;
extern DAC_HandleTypeDef hdac2;
extern SDADC_HandleTypeDef hsdadc1;
extern SDADC_HandleTypeDef hsdadc2;

// declare a struct to hold the control rate inputs

typedef struct {
	uint32_t knob1Value;
	uint32_t knob2Value;
	uint32_t knob3Value;
	uint32_t cv1Value;
} controlRateInputs;

int controlRateADCReadings[4];

#define knob2 controlRateADCReadings[3]
#define knob3 controlRateADCReadings[1]
#define knob1 controlRateADCReadings[2]
#define cv1 (4095 - controlRateADCReadings[0])

// declare a struct to point to the audio rate inputs

typedef struct {
	int16_t * cv2Samples;
	int16_t * cv3Samples;
	int16_t * cv2VirtualGround;
	int16_t * cv3VirtualGround;
} audioRateInputs;

// declare a struct to point to the audio rate outputs

typedef struct {
	uint32_t * dac1Samples;
	uint32_t * dac2Samples;
	uint32_t * dac3Samples;
	uint32_t * shA;
	uint32_t * shB;
	uint32_t * logicA;
	uint32_t * auxLogic;
} audioRateOutputs;

// allocate a DMA buffer for each oversampled output

// initialize a copy of each signal group

audioRateOutputs audioRateOutput;
audioRateInputs audioRateInput;
controlRateInputs controlRateInput;



static inline void viaSignalInit(audioRateInputs * audioRateInput, audioRateOutputs * audioRateOutput, int bufferSize) {

	audioRateOutput->dac1Samples = malloc(2*bufferSize*sizeof(int));
	audioRateOutput->dac2Samples = malloc(2*bufferSize*sizeof(int));
	audioRateOutput->dac3Samples = malloc(2*bufferSize*sizeof(int));

	audioRateInput->cv2Samples = malloc(2*bufferSize*sizeof(int));
	audioRateInput->cv3Samples = malloc(2*bufferSize*sizeof(int));
	audioRateInput->cv2VirtualGround = malloc(2*bufferSize*sizeof(int));
	audioRateInput->cv3VirtualGround = malloc(2*bufferSize*sizeof(int));

	int16_t cv2Offset = HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA0) << 2;
	int16_t cv3Offset = HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA1) << 2;

	for (int i = 0; i < 2*bufferSize; i++) {
		audioRateInput->cv2VirtualGround[i] = cv2Offset;
		audioRateInput->cv3VirtualGround[i] = cv3Offset;
	}

	// initialize the ADCs and their respective DMA arrays
	HAL_ADC_Start_DMA(&hadc1, controlRateADCReadings, 4);

	if (HAL_SDADC_CalibrationStart(&hsdadc1, SDADC_CALIBRATION_SEQ_1) != HAL_OK)
	{
		LEDA_ON;
	}

	/* Pool for the end of calibration */
	if (HAL_SDADC_PollForCalibEvent(&hsdadc1, 100) != HAL_OK)
	{
		LEDB_ON;
	}


	if (HAL_SDADC_CalibrationStart(&hsdadc2, SDADC_CALIBRATION_SEQ_1) != HAL_OK)
	{
		LEDA_ON;
	}

	/* Pool for the end of calibration */
	if (HAL_SDADC_PollForCalibEvent(&hsdadc2, 100) != HAL_OK)
	{
		LEDB_ON;
	}

	HAL_SDADC_Start_DMA(&hsdadc1, audioRateInput->cv2Samples, 1);
	HAL_SDADC_Start_DMA(&hsdadc2, audioRateInput->cv3Samples, 1);

	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, audioRateOutput->dac1Samples, 2*bufferSize, DAC_ALIGN_12B_R);
	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, audioRateOutput->dac2Samples, 2*bufferSize, DAC_ALIGN_12B_R);
	HAL_DAC_Start_DMA(&hdac2, DAC_CHANNEL_1, audioRateOutput->dac3Samples, 2*bufferSize, DAC_ALIGN_12B_R);

}

static inline void updateControlRateInputs(controlRateInputs * controls) {

	// TODO apply SIMD instructions?

	static int knob1Sum;
	static int knob2Sum;
	static int knob3Sum;
	static int cv1Sum;
	static buffer knob1Buffer;
	static buffer knob2Buffer;
	static buffer knob3Buffer;
	static buffer cv1Buffer;

	// store the newest value in a ring buffer
	writeBuffer(&cv1Buffer, cv1);
	writeBuffer(&knob1Buffer, knob1);
	writeBuffer(&knob2Buffer, knob2);
	writeBuffer(&knob3Buffer, knob3);

	// implement a running average on the control rate CV controls
	cv1Sum = cv1 + cv1Sum - readBuffer(&cv1Buffer, 31);
	knob1Sum = knob1 + knob1Sum - readBuffer(&knob1Buffer, 31);
	knob2Sum = knob2 + knob2Sum - readBuffer(&knob2Buffer, 31);
	knob3Sum = knob3 + knob3Sum - readBuffer(&knob3Buffer, 31);


	// write the averaged controls to the holding struct
	controls->cv1Value = cv1Sum >> 5;
	controls->knob1Value = knob1Sum >> 5;
	controls->knob2Value = knob2Sum >> 5;
	controls->knob3Value = knob3Sum >> 5;

}



#endif /* INC_PLATFORM_SIGNALS_H_ */
