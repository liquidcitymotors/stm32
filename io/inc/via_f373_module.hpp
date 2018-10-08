/*
 * via_f373_module.hpp
 *
 *  Created on: Sep 18, 2018
 *      Author: willmitchell
 */

#ifndef INC_VIA_F373_MODULE_HPP_
#define INC_VIA_F373_MODULE_HPP_

#include "dsp.hpp"
#include "f373_rev6_io.hpp"
#include "via_global_signals.hpp"

class ViaModule {
public:

	volatile uint32_t * aLogicOutput;
	volatile uint32_t * auxLogicOutput;
	volatile uint32_t * shAOutput;
	volatile uint32_t * shBOutput;

	volatile uint32_t * redLevel;
	volatile uint32_t * greenLevel;
	volatile uint32_t * blueLevel;

	volatile uint32_t * ledAOutput;
	volatile uint32_t * ledBOutput;
	volatile uint32_t * ledCOutput;
	volatile uint32_t * ledDOutput;

	int32_t * button1Input;
	int32_t * button2Input;
	int32_t * button3Input;
	int32_t * button4Input;
	int32_t * button5Input;
	int32_t * button6Input;

	ViaControls controls;

	int32_t bufferSize;
	ViaInputStreams inputs;
	ViaOutputStreams outputs;

	void ioStreamInit() {

//		int16_t cv2Offset = HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA0) << 2;
//		int16_t cv3Offset = HAL_FLASHEx_OBGetUserData(OB_DATA_ADDRESS_DATA1) << 2;

//		for (int32_t i = 0; i < 2 * bufferSize; i++) {
//			inputs.cv2VirtualGround[i] = cv2Offset;
//			inputs.cv3VirtualGround[i] = cv3Offset;
//		}

		// initialize the ADCs and their respective DMA arrays
		HAL_ADC_Start_DMA(&hadc1, controls.controlRateInputs, 4);

		if (HAL_SDADC_CalibrationStart(&hsdadc1, SDADC_CALIBRATION_SEQ_1)
				!= HAL_OK) {
		}

		/* Pool for the end of calibration */
		if (HAL_SDADC_PollForCalibEvent(&hsdadc1, 1000) != HAL_OK) {
		}

		if (HAL_SDADC_CalibrationStart(&hsdadc2, SDADC_CALIBRATION_SEQ_1)
				!= HAL_OK) {
		}

		/* Pool for the end of calibration */
		if (HAL_SDADC_PollForCalibEvent(&hsdadc2, 1000) != HAL_OK) {
		}

		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, outputs.dac1Samples,
				2 * bufferSize, DAC_ALIGN_12B_R);
		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, outputs.dac2Samples,
				2 * bufferSize, DAC_ALIGN_12B_R);
		HAL_DAC_Start_DMA(&hdac2, DAC_CHANNEL_1, outputs.dac3Samples,
				2 * bufferSize, DAC_ALIGN_12B_R);

		// set the dac sample rate and start the dac timer
		HAL_SDADC_Start_DMA(&hsdadc1, (uint32_t *) inputs.cv2Samples, 2 * bufferSize);
		HAL_SDADC_Start_DMA(&hsdadc2, (uint32_t *) inputs.cv3Samples, 2 * bufferSize);

	}

	void initializeAuxOutputs(void) {

		aLogicOutput = &(GPIOC->BSRR);
		auxLogicOutput = &(GPIOA->BSRR);
		shAOutput = &(GPIOB->BSRR);
		shBOutput = &(GPIOB->BSRR);

		// tim3 channel 2
		redLevel = &TIM3->CCR1 + 1;
		// tim4 channel 4
		greenLevel = &TIM4->CCR1 + 3;
		// tim5 channel 1
		blueLevel = &TIM5->CCR1;

		ledAOutput = &(GPIOF->BSRR);
		ledBOutput = &(GPIOC->BSRR);
		ledCOutput = &(GPIOA->BSRR);
		ledDOutput = &(GPIOB->BSRR);

	}

	// 16 samples from the hue space as RGB

	//rgb hueSpace[16] = {{4095, 2457, 0}, {4095, 3992, 0}, {2661, 4095, 0}, {1126, 4095, 0}, {0, 4095, 409}, {0, 4095, 1945}, {0, 4095, 3480}, {0, 3173, 4095}, {0, 1638, 4095}, {0, 102, 4095}, {1433, 0, 4095}, {2968, 0, 4095}, {4095, 0, 3685}, {4095, 0, 2149}, {4095, 0, 614}, {4095, 921, 0}};

	//rgb hueSpace[16] = {{4095, 0, 0}, {4095, 1535, 0}, {4095, 3071, 0}, {3583, 4095, 0}, {2047, 4095, 0}, {511, 4095, 0}, {0, 4095, 1023}, {0, 4095, 2559}, {0, 4095, 4095}, {0, 2559, 4095}, {0, 1023, 4095}, {511, 0, 4095}, {2047, 0, 4095}, {3583, 0, 4095}, {4095, 0, 3071}, {4095, 0, 1535}};

	rgb hueSpace[16] = {{4095, 0, 0}, {4095, 1228, 0}, {4095, 2457, 0}, {4095, 3685, 0}, {2047, 4095, 0}, {819, 4095, 0}, {0, 4095, 409}, {0, 4095, 1638}, {0, 4095, 4095}, {0, 2866, 4095}, {0, 1638, 4095}, {0, 409, 4095}, {2047, 0, 4095}, {3276, 0, 4095}, {4095, 0, 3685}, {4095, 0, 2456}};


	/*
	 *
	 * Logic output handling
	 *
	 */

	inline void setLogicA(int32_t high) {
		*aLogicOutput = GET_ALOGIC_MASK(high);
	}

	inline void setAuxLogic(int32_t high) {
		*auxLogicOutput = GET_EXPAND_LOGIC_MASK(high);
	}

	inline void setSH(int32_t sampleA, int32_t sampleB) {

		uint32_t mask = GET_SH_A_MASK(sampleA);
		mask |= GET_SH_B_MASK(sampleB);

		*shAOutput = mask;

	}

	inline void setLogicOutputsLEDOn(uint32_t logicA, uint32_t auxLogic,
			uint32_t shA, uint32_t shB) {

		// LEDA_HIGH_MASK -> SH_A_SAMPLE_MASK >> 16 >> 1 (pin 8 to pin 7, F)
		// LEDB_HIGH_MASK -> SH_B_SAMPLE_MASK >> 16 << 5 (pin 9 to pin 14, C)
		// LEDC_HIGH_MASK -> ALOGIC_HIGH_MASK >> 16 >> 11 (pin 13 to pin 2, A)
		// LEDD_HIGH_MASK -> BLOGIC_HIGH_MASK >> 16 >> 13 (pin 15 to pin 2, B)

	#define LEDA_MASK (__ROR(shA, 16) >> 1)
	#define LEDB_MASK (__ROR(shB, 16) << 5)
	#define LEDC_MASK (__ROR(logicA, 16) >> 11)

		//combine the mask variables for a shared GPIO group with a bitwise or
		*aLogicOutput = (logicA | LEDB_MASK);

		*auxLogicOutput = (auxLogic | LEDC_MASK);

		*shAOutput = (shA | shB);

		*ledAOutput = LEDA_MASK;

	}

	inline void setLogicOutputsLEDOff(uint32_t logicA, uint32_t auxLogic,
			uint32_t shA, uint32_t shB) {

		//combine the mask variables for a shared GPIO group with a bitwise or
		*aLogicOutput = (logicA);

		*auxLogicOutput = (auxLogic);

		*shAOutput = (shA | shB);

	}

	inline void setLogicOut(int32_t writeIndex, int32_t runtimeDisplay) {

		int32_t logicA = outputs.logicA[writeIndex];
		int32_t auxLogic = outputs.auxLogic[writeIndex];
		int32_t shA = outputs.shA[writeIndex];
		int32_t shB = outputs.shB[writeIndex];

		if (runtimeDisplay) {
			setLogicOutputsLEDOn(logicA, auxLogic, shA, shB);
		} else {
			setLogicOutputsLEDOff(logicA, auxLogic, shA, shB);
		}

	}

	inline void setLogicOutBoolean(int32_t writeIndex, int32_t runtimeDisplay) {

		int32_t logicA = GET_ALOGIC_MASK(outputs.logicA[writeIndex]);
		int32_t auxLogic = GET_EXPAND_LOGIC_MASK(outputs.auxLogic[writeIndex]);
		int32_t shA = GET_SH_A_MASK(outputs.shA[writeIndex]);
		int32_t shB = GET_SH_B_MASK(outputs.shB[writeIndex]);

		if (runtimeDisplay) {
			setLogicOutputsLEDOn(logicA, auxLogic, shA, shB);
		} else {
			setLogicOutputsLEDOff(logicA, auxLogic, shA, shB);
		}

	}

	/*
	 *
	 * LED handling
	 *
	 */

	inline void setLEDA(int32_t on) {
		*ledAOutput = ((uint32_t) GPIO_PIN_7) << (16 * (!on));
	}

	inline void setLEDB(int32_t on) {
		*ledBOutput = ((uint32_t) GPIO_PIN_14) << (16 * (!on));
	}

	inline void setLEDC(int32_t on) {
		*ledCOutput = ((uint32_t) GPIO_PIN_2) << (16 * (!on));
	}

	inline void setLEDD(int32_t on) {
		*ledDOutput = ((uint32_t) GPIO_PIN_2) << (16 * (!on));
	}

	inline void setRedLED(int32_t level) {
		*redLevel = level;
	}

	inline void setGreenLED(int32_t level) {
		*greenLevel = level;
	}

	inline void setBlueLED(int32_t level) {
		*blueLevel = level;
	}

	inline void updateRGBDisplay(int32_t red, int32_t green, int32_t blue, int32_t runtimeDisplay) {
		if (runtimeDisplay) {
			setRedLED(red);
			setGreenLED(green);
			setBlueLED(blue);
		}
	}

	void setRGB(rgb color) {
		setRedLED(color.r);
		setGreenLED(color.g);
		setBlueLED(color.b);
	}

	void setRGBScaled(rgb color, int32_t scale) {
		setRedLED((color.r * scale) >> 12);
		setGreenLED((color.g * scale) >> 12);
		setBlueLED((color.b * scale) >> 12);
	}

	void clearRGB() {
		setRGB({0, 0, 0});
	}

	void setLEDs(int32_t digit) {
		switch (digit) {
		case 0:
			setLEDA(1);
			setLEDB(0);
			setLEDC(0);
			setLEDD(0);
			break;
		case 1:
			setLEDA(0);
			setLEDB(0);
			setLEDC(1);
			setLEDD(0);
			break;
		case 2:
			setLEDA(0);
			setLEDB(1);
			setLEDC(0);
			setLEDD(0);
			break;
		case 3:
			setLEDA(0);
			setLEDB(0);
			setLEDC(0);
			setLEDD(1);
			break;
		case 4:
			setLEDA(1);
			setLEDB(0);
			setLEDC(1);
			setLEDD(0);
			break;
		case 5:
			setLEDA(0);
			setLEDB(1);
			setLEDC(0);
			setLEDD(1);
			break;
		case 6:
			setLEDA(1);
			setLEDB(1);
			setLEDC(0);
			setLEDD(0);
			break;
		case 7:
			setLEDA(0);
			setLEDB(0);
			setLEDC(1);
			setLEDD(1);
			break;
		}
	}

	void clearLEDs() {
		setLEDA(0);
		setLEDB(0);
		setLEDC(0);
		setLEDD(0);
	}

};



#endif /* INC_VIA_F373_MODULE_HPP_ */
