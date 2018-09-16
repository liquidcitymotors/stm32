/*
 * ViaUI::presets.cpp
 *
 *  Created on: Sep 11, 2018
 *      Author: willmitchell
 */


#include "user_interface.hpp"

/**
 *
 * Preset select menu
 *
 */

void ViaUI::presetMenu(int sig) {

	switch (sig) {

	case ENTRY_SIG:
//		CLEAR_RUNTIME_DISPLAY;
//		uiClearLEDs();
//		uiClearRGB();
		timerReset();
		timerDisable();
		break;

	case SENSOR_EVENT_SIG:

		if (BUTTON3SENSOR== PRESSED) {
			transition(&ViaUI::presetPressedMenu);
			presetNumber = 3;
		} else if (BUTTON1SENSOR == PRESSED) {
			transition(&ViaUI::presetPressedMenu);
			presetNumber = 1;
		} else if (BUTTON4SENSOR == PRESSED) {
			transition(&ViaUI::presetPressedMenu);
			presetNumber = 4;
		} else if (BUTTON6SENSOR == PRESSED) {
			transition(&ViaUI::presetPressedMenu);
			presetNumber = 6;
		} else if (BUTTON2SENSOR == PRESSED) {
			transition(&ViaUI::presetPressedMenu);
			presetNumber = 2;
		} else if (BUTTON5SENSOR == PRESSED) {
			transition(&ViaUI::presetPressedMenu);
			presetNumber = 5;
		}
		break;

		case EXPAND_SW_OFF_SIG:
		transition(&ViaUI::defaultMenu);
		break;

		case EXIT_SIG:
//		CLEAR_RUNTIME_DISPLAY;
//		uiClearLEDs();
//		uiClearRGB();
		break;

		default:
		break;
	}
}

/**
 *
 * Preset selected menu
 * Watches for release while a sensor is pressed from the preset menu.
 * Loads or stores preset according to the length of the press.
 *
 */

void ViaUI::presetPressedMenu(int sig) {
	switch (sig) {
	case ENTRY_SIG:
		timerReset();
		timerSetOverflow(500);
		timerEnable();
		break;

	case SENSOR_EVENT_SIG:
		switch (presetNumber) {
		case 1:
			if (BUTTON1SENSOR== RELEASED) {
				loadFromEEPROM(presetNumber);
				transition(&ViaUI::switchPreset);
			}
			break;
			case 2:
			if (BUTTON4SENSOR == RELEASED) {
				loadFromEEPROM(presetNumber);
				transition(&ViaUI::switchPreset);
			}
			break;
			case 3:
			if (BUTTON3SENSOR == RELEASED) {
				loadFromEEPROM(presetNumber);
				transition(&ViaUI::switchPreset);
			}
			break;
			case 4:
			if (BUTTON4SENSOR == RELEASED) {
				loadFromEEPROM(presetNumber);
				transition(&ViaUI::switchPreset);
			}
			break;
			case 5:
			if (BUTTON5SENSOR == RELEASED) {
				loadFromEEPROM(presetNumber);
				transition(&ViaUI::switchPreset);
			}
			break;
			case 6:
			if (BUTTON6SENSOR == RELEASED) {
				loadFromEEPROM(presetNumber);
				transition(&ViaUI::switchPreset);
			}
			break;
		}
		break;

		case TIMEOUT_SIG:
		storeToEEPROM(presetNumber);
		transition(&ViaUI::newPreset);
		break;

		// if trig button is released, exit menu
		case EXPAND_SW_OFF_SIG:
		transition(&ViaUI::defaultMenu);
		break;

		case EXIT_SIG:
		break;
	}
}

/**
 *
 * Preset storage indicator state
 * Flashes around the white leds 4 times
 *
 */

void ViaUI::newPreset(int sig) {
	static int flashCounter = 0;
	switch (sig) {
	case ENTRY_SIG:
		timerReset();
		timerSetOverflow(500);
		timerEnable();
		break;

	case TIMEOUT_SIG:
		if (flashCounter < 16) {
//			UI_TIMER_ENABLE
//			;
			flashCounter++;
//			uiSetLEDs(flashCounter % 4);
		} else {
			flashCounter = 0;
			transition(&ViaUI::defaultMenu);
		}
	}
}

/**
 *
 * Switch to preset indicator state
 * Flashes around the white leds once
 *
 */

void ViaUI::switchPreset(int sig) {
	static int flashCounter = 0;
	switch (sig) {
	case ENTRY_SIG:
		timerReset();
		timerSetOverflow(500);
		timerEnable();
		break;

	case TIMEOUT_SIG:
		if (flashCounter < 4) {
//			UI_TIMER_ENABLE
//			;
			flashCounter++;
			//uiSetLEDs(flashCounter % 4);
		} else {
			flashCounter = 0;
			transition(&ViaUI::defaultMenu);
		}
	}
}
