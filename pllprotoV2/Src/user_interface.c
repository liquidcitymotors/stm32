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

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

// these enums contain our mode information
enum syncTypes syncMode; // {none, true, hardSync, catch}
enum controlSchemes controlScheme; // {gateLength, knobCV}
enum scaleTypes scaleType; // {rhythms, pitches}
enum sampleHoldModeTypes sampleHoldMode; // {nosampleandhold, a, b, ab, antidecimate, decimate}
enum logicOutATypes logicOutA; // {triggerA, gateA, deltaA, ratioDeltaA, pllClock};
enum logicOutBTypes logicOutB; // {triggerB, gateB, deltaB, ratioDeltaB, pllClock};
enum autoDutyTypes autoDuty; // {autoDutyOn, autoDutyOff};

extern uint16_t VirtAddVarTab[NB_OF_VAR];
extern uint16_t VarDataTab[NB_OF_VAR];

// these logic flags are used to communicate state between the main controlScheme and the interrupts

// these variables are passed between our functions that read the touch sensors and change modes
uint32_t lastDetect;

void handleRelease(uint32_t);
void changeMode(uint32_t);
void showMode(uint32_t);
void familyRGB(void);
void clearLEDs(void);
void switchFamily(void);
void loadSampleArray(Family);

extern uint32_t modeflag;
extern uint32_t detectOn;
extern uint32_t displayNewMode;
uint32_t lastDetect;
int holdState;

void readDetect(void) {
	// check to see if any touch sensors have gone into detect state
	if (MyTKeys[3].p_Data->StateId == TSL_STATEID_DETECT) {
		RESET_DISPLAY_RUNTIME;  // turn off the runtime display
		modeflag = 10; // indicate to the other mode change functions that we have pressed the scaleType button
		detectOn = 1;  // indicate that a touch sensor was in detect state during this acquisition cycle
		clearLEDs();   // wipe the vestiges of our runtime display
		__HAL_TIM_SET_COUNTER(&htim4, 0); //reset the timer that we use for mode change timeout
		showMode(scaleType); //show our current mode
	}
	if (MyTKeys[2].p_Data->StateId == TSL_STATEID_DETECT) {
		RESET_DISPLAY_RUNTIME;
		modeflag = 2;  // indicate to the other mode change functions that we have pressed the trigger mode button
		detectOn = 1;
		clearLEDs();
		__HAL_TIM_SET_COUNTER(&htim4, 0);
		showMode(syncMode);
	}
	if (MyTKeys[1].p_Data->StateId == TSL_STATEID_DETECT) {
		RESET_DISPLAY_RUNTIME;
		modeflag = 3; // indicate to the other mode change functions that we have pressed the controlScheme button
		detectOn = 1;
		clearLEDs();
		__HAL_TIM_SET_COUNTER(&htim4, 0);
		showMode(controlScheme);
	}
	if (MyTKeys[4].p_Data->StateId == TSL_STATEID_DETECT) {
		RESET_DISPLAY_RUNTIME;
		modeflag = 4; // indicate to the other mode change functions that we have pressed the sample and hold mode button
		detectOn = 1;
		clearLEDs();
		__HAL_TIM_SET_COUNTER(&htim4, 0);
		showMode(sampleHoldMode);
	}
	if (MyTKeys[5].p_Data->StateId == TSL_STATEID_DETECT) {
		RESET_DISPLAY_RUNTIME;
		modeflag = 5; // indicate to the other mode change functions that we have pressed the family up button
		detectOn = 1;
		clearLEDs();
		__HAL_TIM_SET_COUNTER(&htim4, 0);
		showMode(familyIndicator);
	}
	if (MyTKeys[0].p_Data->StateId == TSL_STATEID_DETECT) {
		RESET_DISPLAY_RUNTIME;
		modeflag = 6; // indicate to the other mode change functions that we have pressed the family down button
		detectOn = 1;
		clearLEDs();
		__HAL_TIM_SET_COUNTER(&htim4, 0);
		showMode(familyIndicator);
	}
}

void readRelease(uint32_t modeFlagHolder) {
	// look for a change to release state on the button that was pressed (passed in with the function argument)
	switch (modeFlagHolder) {

	case 0:
		if (MyTKeys[5].p_Data->StateId == TSL_STATEID_RELEASE) {
			if (MyTKeys[3].p_Data->StateId == TSL_STATEID_RELEASE) {
				detectOn = 0;
			}
			clearLEDs();
			handleRelease(modeFlagHolder);
		}
		break;

	case 1:
		if (MyTKeys[0].p_Data->StateId == TSL_STATEID_RELEASE) {
			if (MyTKeys[3].p_Data->StateId == TSL_STATEID_RELEASE) {
				detectOn = 0;
			}
			clearLEDs();
			handleRelease(modeFlagHolder);
		}
		break;

	case 10:
		if (MyTKeys[3].p_Data->StateId == TSL_STATEID_RELEASE) {
			detectOn = 0; // indicate that we no longer have a touch sensor in detect state
			clearLEDs();  // clear the display that showed the current mode
			handleRelease(modeFlagHolder); // take the appropriate action per the button that had been pressed
		}
		if (MyTKeys[5].p_Data->StateId == TSL_STATEID_DETECT) {
			SET_AUX_MENU;
			modeflag = 0; // indicate to the other mode change functions that we have pressed the logic a button
			detectOn = 1; // indicate that a touch sensor was in detect state during this acquisition cycle
			clearLEDs();  // wipe the vestiges of our runtimme display
			__HAL_TIM_SET_COUNTER(&htim4, 0); // reset the timer that we use for mode change timeout
			showMode(scaleType); // show our current mode
		}
		if (MyTKeys[0].p_Data->StateId == TSL_STATEID_DETECT) {
			SET_AUX_MENU;
			modeflag = 1; // indicate to the other mode change functions that we have pressed the logic b button
			detectOn = 1; // indicate that a touch sensor was in detect state during this acquisition cycle
			clearLEDs();  // wipe the vestiges of our runtime display
			__HAL_TIM_SET_COUNTER(&htim4, 0); // reset the timer that we use for mode change timeout
			showMode(scaleType); // show our current mode
		}
		break;

	case 2:
		if (MyTKeys[3].p_Data->StateId == TSL_STATEID_DETECT) {
			SET_AUX_MENU;
			modeflag = 7; // indicate to the other mode change functions that we have pressed the logic a button
			detectOn = 1; // indicate that a touch sensor was in detect state during this acquisition cycle
			clearLEDs();  // wipe the vestiges of our runtime display
			__HAL_TIM_SET_COUNTER(&htim4, 0); // reset the timer that we use for mode change timeout
			showMode(logicOutA); // show our current mode
		}
		if (MyTKeys[1].p_Data->StateId == TSL_STATEID_DETECT) {
			SET_AUX_MENU;
			modeflag = 8; // indicate to the other mode change functions that we have pressed the logic b button
			detectOn = 1; // indicate that a touch sensor was in detect state during this acquisition cycle
			clearLEDs();  // wipe the vestiges of our runtime display
			__HAL_TIM_SET_COUNTER(&htim4, 0); // reset the timer that we use for mode change timeout
			showMode(logicOutB); // show our current mode
		}
		if (MyTKeys[4].p_Data->StateId == TSL_STATEID_DETECT) {
			SET_AUX_MENU;
			modeflag = 9; // indicate to the other mode change functions that we have pressed the logic b button
			detectOn = 1; // indicate that a touch sensor was in detect state during this acquisition cycle
			clearLEDs();  // wipe the vestiges of our runtime display
			__HAL_TIM_SET_COUNTER(&htim4, 0); // reset the timer that we use for mode change timeout
			showMode(autoDuty); // show our current mode
		}

		if (MyTKeys[2].p_Data->StateId == TSL_STATEID_RELEASE) {
			detectOn = 0;
			if (!(AUX_MENU)) {
				clearLEDs();
			}
			handleRelease(modeFlagHolder);
		}
		break;

	case 3:
		if (MyTKeys[1].p_Data->StateId == TSL_STATEID_RELEASE) {
			detectOn = 0;
			clearLEDs();
			handleRelease(modeFlagHolder);
		}
		break;

	case 4:
		if (MyTKeys[4].p_Data->StateId == TSL_STATEID_RELEASE) {
			detectOn = 0;
			clearLEDs();
			handleRelease(modeFlagHolder);
		}
		break;

	case 5:
		if (MyTKeys[5].p_Data->StateId == TSL_STATEID_RELEASE) {
			detectOn = 0;
			clearLEDs();
			handleRelease(modeFlagHolder);
		}
		break;

	case 6:
		if (MyTKeys[0].p_Data->StateId == TSL_STATEID_RELEASE) {
			detectOn = 0;
			clearLEDs();
			handleRelease(modeFlagHolder);
		}
		break;

	case 7:
		if (MyTKeys[3].p_Data->StateId == TSL_STATEID_RELEASE) {
			if (MyTKeys[2].p_Data->StateId == TSL_STATEID_RELEASE) {
				detectOn = 0;
			}
			clearLEDs();
			handleRelease(modeFlagHolder);
		}
		break;

	case 8:
		if (MyTKeys[1].p_Data->StateId == TSL_STATEID_RELEASE) {
			if (MyTKeys[2].p_Data->StateId == TSL_STATEID_RELEASE) {
				detectOn = 0;
			}
			clearLEDs();
			handleRelease(modeFlagHolder);
		}
		break;

	case 9:
		if (MyTKeys[4].p_Data->StateId == TSL_STATEID_RELEASE) {
			if (MyTKeys[2].p_Data->StateId == TSL_STATEID_RELEASE) {
				detectOn = 0;
			}
			clearLEDs();
			handleRelease(modeFlagHolder);
		}
		break;
	}
}

void handleRelease(uint32_t pinMode) {
	if (__HAL_TIM_GET_COUNTER(&htim4) < 3000) {
		// if we haven't exceeded the mode change timeout, change the appropriate mode and then display the new mode
		// current value is probably too short
		changeMode(pinMode);
		switch (pinMode) {
		case 0:
			modeflag = 10;
			showMode(scaleType);
			break;
		case 1:
			modeflag = 10;
			showMode(scaleType);
			break;
		case 2:
			showMode(syncMode);
			break;
		case 3:
			showMode(controlScheme);
			break;
		case 4:
			showMode(sampleHoldMode);
			break;
		case 5:
			showMode(familyIndicator);
			break;
		case 6:
			showMode(familyIndicator);
			break;
		case 7:
			modeflag = 2;
			showMode(logicOutA);
			break;
		case 8:
			modeflag = 2;
			showMode(logicOutB);
			break;
		case 9:
			modeflag = 2;
			showMode(autoDuty);
			break;
		case 10:
			modeflag = 2;
			showMode(scaleType);
			break;
		}
		displayNewMode = 1;
		__HAL_TIM_SET_COUNTER(&htim4, 0);
	} else {
		if (AUX_MENU) {
			modeflag = 2;
		}
		clearLEDs();
		SET_DISPLAY_RUNTIME;
	}
}

void changeMode(uint32_t mode) {
	if (mode == 0) {
		// toggle up through our 8 scaleType modes
		scaleType = (scaleType + 1) % 7;
		//switchScale(scaleType);
		holdState = (holdState & 0b1111111100111111) | (scaleType << 6);
	}
	else if (mode == 1) {
		// toggle down through our 8 scaleType modes
		scaleType = (scaleType - 1);
		if (scaleType < 0) {scaleType = 0;}
		//switchScale(scaleType);
		holdState = (holdState & 0b1111111100111111) | (scaleType << 6);
	}
	else if (mode == 2) {
		syncMode = (syncMode + 1) % 3;
		holdState = (holdState & 0b1111111111000111) | (syncMode << 3);

	}
	else if (mode == 3) {
		controlScheme = (controlScheme + 1) % 4;
		holdState = (holdState & 0b1111111111111000) | controlScheme;
	}
	else if (mode == 4) {
		sampleHoldMode = (sampleHoldMode + 1) % 6;
		holdState = (holdState & 0b1111100011111111) | (sampleHoldMode << 8);
		SH_A_TRACK;
		SH_B_TRACK;
	}
	else if (mode == 5) {
		// increment our family pointer and swap in the correct family
		familyIndicator = (familyIndicator + 1) % 16;
		switchFamily();
		holdState = (holdState & 0b1000011111111111) | (familyIndicator << 11);
	}
	else if (mode == 6) {
		// wrap back to the end of the array of families if we go back from the first entry
		// otherwise same as above
		if (familyIndicator == 0) {
			familyIndicator = 15;
		} else {
			familyIndicator = (familyIndicator - 1);
		}
		switchFamily();
		holdState = (holdState & 0b1000011111111111) | (familyIndicator << 11);
	}
	else if (mode == 7) {
		logicOutA = (logicOutA + 1) % 5;
		holdLogicOut = (holdLogicOut & 0b1111111111111000) | logicOutA;
		eepromStatus = EE_WriteVariable(VirtAddVarTab[1], holdLogicOut);
	    eepromStatus|= EE_ReadVariable(VirtAddVarTab[1],  &VarDataTab[1]);

		switch (logicOutA) {
		case 0:
			SET_GATEA;
			RESET_DELTAB;
			RESET_DELTAA;
			RESET_RATIO_DELTAA;
			RESET_PLL_DIVA;
			break;
		case 1:
			RESET_GATEA;
			SET_TRIGA;
			RESET_DELTAA;
			RESET_RATIO_DELTAA;
			RESET_PLL_DIVA;
			break;
		case 2:
			RESET_GATEA;
			RESET_TRIGA;
			SET_DELTAA;
			RESET_RATIO_DELTAA;
			RESET_PLL_DIVA;
			break;
		case 3:
			RESET_GATEA;
			RESET_TRIGA;
			RESET_DELTAA;
			SET_RATIO_DELTAA;
			RESET_PLL_DIVA;
			break;
		case 4:
			RESET_GATEA;
			RESET_TRIGA;
			RESET_DELTAA;
			RESET_RATIO_DELTAA;
			SET_PLL_DIVA;
			break;
		}
	}
	else if (mode == 8) {
		logicOutB = (logicOutB + 1) % 5;
		holdLogicOut = (holdLogicOut & 0b1111111111000111) | (logicOutB << 3);
		eepromStatus = EE_WriteVariable(VirtAddVarTab[1], holdLogicOut);
	    eepromStatus|= EE_ReadVariable(VirtAddVarTab[1],  &VarDataTab[1]);
		switch (logicOutB) {
		case 0:
			SET_GATEB;
			RESET_TRIGB;
			RESET_DELTAB;
			RESET_RATIO_DELTAB;
			RESET_PLL_DIVB;
			break;
		case 1:
			RESET_GATEB;
			SET_TRIGB;
			RESET_DELTAA;
			RESET_RATIO_DELTAB;
			RESET_PLL_DIVB;
			break;
		case 2:
			RESET_GATEB;
			RESET_TRIGB;
			SET_DELTAB;
			RESET_RATIO_DELTAB;
			RESET_PLL_DIVB;
			break;
		case 3:
			RESET_GATEB;
			RESET_TRIGB;
			RESET_DELTAB;
			SET_RATIO_DELTAB;
			RESET_PLL_DIVB;
			break;
		case 4:
			RESET_GATEB;
			RESET_TRIGB;
			RESET_DELTAB;
			RESET_RATIO_DELTAB;
			SET_PLL_DIVB;
			break;
		}
	}
	else if (mode == 9) {
		autoDuty = (autoDuty + 1) % 2;
		holdLogicOut = (holdLogicOut & 0b1111111111000111) | (autoDuty << 6);
		if (autoDuty == autoDutyOn) {
			RESET_AUTODUTY;
		} else {
			SET_AUTODUTY;
		}
		eepromStatus = EE_WriteVariable(VirtAddVarTab[1], holdLogicOut);
	    eepromStatus|= EE_ReadVariable(VirtAddVarTab[1],  &VarDataTab[1]);
	}
	eepromStatus = EE_WriteVariable(VirtAddVarTab[0], holdState);
    eepromStatus|= EE_ReadVariable(VirtAddVarTab[0],  &VarDataTab[0]);
}

void showMode(uint32_t currentmode) {
	// if we are switching families, show a color corresponding to that family
	if ((modeflag == 5) || modeflag == 6) {
		familyRGB();
	}
	else {
		switch (currentmode) {
		// represent a 4 bit number with our LEDs
		// NEEDS WORK
		case 0:
			LEDA_ON;
			break;
		case 1:
			LEDC_ON;
			break;
		case 2:
			LEDB_ON;
			break;
		case 3:
			LEDD_ON;
			break;
		case 4:
			LEDA_ON;
			LEDC_ON;
			break;
		case 5:
			LEDB_ON;
			LEDD_ON;
			break;
		}
	}
}

void familyRGB(void) {
	if (familyIndicator < 8) {
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 4095);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
	}
	else {
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 4095);
	}
	switch (familyIndicator % 8) {
	case 0:
		LEDA_ON;
		break;
	case 1:
		LEDC_ON;
		break;
	case 2:
		LEDB_ON;
		break;
	case 3:
		LEDD_ON;
		break;
	case 4:
		LEDA_ON;
		LEDC_ON;
		break;
	case 5:
		LEDB_ON;
		LEDD_ON;
		break;
	case 6:
		LEDA_ON;
		LEDB_ON;
		break;
	case 7:
		LEDC_ON;
		LEDD_ON;
		break;
	}
}

void clearLEDs(void) {
	// pretty self explanatory
	LEDA_OFF;
	LEDB_OFF;
	LEDC_OFF;
	LEDD_OFF;
	// blank the LEDs
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
}

void restoreDisplay() {
	if (__HAL_TIM_GET_COUNTER(&htim4) > 10000) {
		clearLEDs(); // get rid of last mode display
		SET_DISPLAY_RUNTIME;  // turn on the runtime display
		displayNewMode = 0; // a bit of logic used to make sure that we show the mode during the main controlScheme
	}
}

// this sets the flags to be used in the interrupt and also fills the holding array on the heap
void switchFamily(void) {
	position = 0;
	holdState |= familyIndicator << 9;
	currentFamily = familyArray[familyIndicator];
	loadSampleArray(currentFamily);
	span = (currentFamily.tableLength) << 16;
	spanx2 = (currentFamily.tableLength) << 17;

	switch (currentFamily.familySize) {
	// these are values that properly allow us to select a family and interpolation fraction for our morph
	case 3:
		morphBitShiftRight = 11;
		morphBitShiftLeft = 5;
		break;

	case 5:
		morphBitShiftRight = 10;
		morphBitShiftLeft = 6;
		break;

	case 9:
		morphBitShiftRight = 9;
		morphBitShiftLeft = 7;
		break;

	case 17:
		morphBitShiftRight = 8;
		morphBitShiftLeft = 8;
		break;

	case 33:
		morphBitShiftRight = 7;
		morphBitShiftLeft = 9;
		break;

	}
	switch (currentFamily.tableLength) {
	// these are values that properly allow us to select a family and interpolation fraction for our morph
	case 4:
		tableSizeCompensation = 5;
		break;
	case 8:
		tableSizeCompensation = 4;
		break;
	case 16:
		tableSizeCompensation = 3;
		break;
	case 32:
		tableSizeCompensation = 2;
		break;
	case 64:
		tableSizeCompensation = 1;
		break;
	case 128:
		tableSizeCompensation = 0;
	}
}

// shuttles the data from flash to ram and fills the holding array
void loadSampleArray(Family family) {
	uint16_t **currentFamilyPointer;

	for (int i = 0; i < family.familySize; i++) {
		for (int j = 0; j <= family.tableLength + 4; j++) {
			// this just gets the appropriate samples and plops them into the global holding arrays
			currentFamilyPointer = family.attackFamily + i;
			attackHoldArray[i][j] = *(*(currentFamilyPointer) + j);
			currentFamilyPointer = family.releaseFamily + i;
			releaseHoldArray[i][j] = *(*(currentFamilyPointer) + j);
		}
	}
}
