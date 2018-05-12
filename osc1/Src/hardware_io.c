#include "stm32f3xx_hal.h"
#include "stm32f3xx.h"
#include "stm32f3xx_it.h"
#include "hardware_io.h"

uint32_t dacALevel;
uint32_t dacBLevel;

extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim1;

enum {
	DAC_GATE_HIGH,
	DAC_GATE_LOW,
	DAC_EXECUTE,
};

//////////////////////////
// dac handling functions
//////////////////////////

void dacAHigh(int event) {
	switch (event) {
		case DAC_EXECUTE:
			((*(volatile uint32_t *) DAC1_ADDR) = 4095);
			if (RUNTIME_DISPLAY) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 4095);
			}
			break;
		case DAC_GATE_LOW:
			dacALevel = 4095;
			manageADac = dacAFall;
			break;
		default:
			break;
	}
}
void dacALow(int event){
	switch (event) {
		case DAC_EXECUTE:
			((*(volatile uint32_t *) DAC1_ADDR) = 0);
			if (RUNTIME_DISPLAY) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
			}
			break;
		case DAC_GATE_HIGH:
			dacALevel = 0;
			manageADac = dacARise;
			break;
		default:
			break;
	}
}

void dacBHigh(int event) {
	switch (event) {
		case DAC_EXECUTE:
			((*(volatile uint32_t *) DAC2_ADDR) = 4095);
			if (RUNTIME_DISPLAY) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 4095);
			}
			break;
		case DAC_GATE_LOW:
			dacBLevel = 4095;
			manageBDac = dacBFall;
			break;
		default:
			break;
	}
}
void dacBLow(int event){
	switch (event) {
		case DAC_EXECUTE:
			((*(volatile uint32_t *) DAC2_ADDR) = 0);
			if (RUNTIME_DISPLAY) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
			}
			break;
		case DAC_GATE_HIGH:
			dacBLevel = 0;
			manageBDac = dacBRise;
			break;
		default:
			break;
	}
}

void dacARise(int event){
	switch (event) {
		case DAC_EXECUTE:
			dacALevel = dacALevel + 100;
			if (dacALevel >= 4095) {
				dacALevel = 4095;
				manageADac = dacAHigh;
			}
			((*(volatile uint32_t *) DAC1_ADDR) = dacALevel);
			if (RUNTIME_DISPLAY) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, dacALevel);
			}
			break;
		case DAC_GATE_LOW:
			manageADac = dacAFall;
			break;
		default:
			break;
	}
}
void dacAFall(int event){
	switch (event) {
		case DAC_EXECUTE:
			dacALevel = dacALevel - 5;
			if (dacALevel <= 0) {
				dacALevel = 0;
				manageADac = dacALow;
			}
			((*(volatile uint32_t *) DAC1_ADDR) = dacALevel);
			if (RUNTIME_DISPLAY) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, dacALevel);
			}
			break;
		case DAC_GATE_HIGH:
			manageADac = dacARise;
			break;
		default:
			break;
	}
}


void dacBRise(int event){
	switch (event) {
		case DAC_EXECUTE:
			dacBLevel = dacBLevel + 50;
			if (dacBLevel >= 4095) {
				dacBLevel = 4095;
				manageBDac = dacBHigh;
			}
			((*(volatile uint32_t *) DAC2_ADDR) = dacBLevel);
			if (RUNTIME_DISPLAY) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, dacBLevel);
			}
			break;
		case DAC_GATE_LOW:
			manageBDac = dacBFall;
			break;
		default:
			break;
	}
}
void dacBFall(int event){
	switch (event) {
		case DAC_EXECUTE:
			dacBLevel = dacBLevel - 5;
			if (dacBLevel <= 0) {
				dacBLevel = 0;
				manageBDac = dacBLow;
			}
			((*(volatile uint32_t *) DAC2_ADDR) = dacBLevel);
			if (RUNTIME_DISPLAY) {
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, dacBLevel);
			}
			break;
		case DAC_GATE_HIGH:
			manageBDac = dacBRise;
	}
}

//////////////////////////
// s&h handling functions
//////////////////////////

void resampleA(void){
	SH_A_TRACK;
	__HAL_TIM_SET_COUNTER(&htim7, 0);
	__HAL_TIM_ENABLE(&htim7);
}
void resampleB(void){
	SH_B_TRACK;
	__HAL_TIM_SET_COUNTER(&htim8, 0);
	__HAL_TIM_ENABLE(&htim8);
}

////////////////////////////////////
// sequence logic handling functions
////////////////////////////////////

void handleAHigh(void){
	ALOGIC_HIGH;
	if (RUNTIME_DISPLAY) {
		LEDC_ON;
	}
	if (AND_A) {
		(*manageADac)(DAC_GATE_HIGH);
	}
	if (SAMPLE_A) {
		resampleA();
	}
}
void handleALow(void){
	ALOGIC_LOW;
	if (RUNTIME_DISPLAY) {
		LEDC_OFF;
	}
	if (AND_A) {
		(*manageADac)(DAC_GATE_LOW);
	}
	if (TRACK_A) {
		SH_A_TRACK;
		if (RUNTIME_DISPLAY) {
			LEDA_OFF;
		}
	}
}

void handleBHigh(void){
	BLOGIC_HIGH;
	if (RUNTIME_DISPLAY) {
		LEDD_ON;
	}
	if (AND_B) {
		(*manageBDac)(DAC_GATE_HIGH);
	}
	if (SAMPLE_B) {
		resampleB();
	}
}
void handleBLow(void){
	BLOGIC_LOW;
	if (RUNTIME_DISPLAY) {
		LEDD_OFF;
	}
	if (AND_B) {
		(*manageBDac)(DAC_GATE_LOW);
	}
	if (TRACK_B) {
		SH_B_TRACK;
		if (RUNTIME_DISPLAY) {
			LEDB_OFF;
		}
	}
}

void handleAuxHigh(void) {
	EXPAND_GATE_HIGH;
	if (RUNTIME_DISPLAY) {
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 1000);
	}
}

void handleAuxLow(void) {
	EXPAND_GATE_LOW;
	if (RUNTIME_DISPLAY) {
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
	}
}
