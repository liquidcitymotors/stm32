/*
 * meta_ui_implementation.cpp
 *
 *  Created on: Sep 12, 2018
 *      Author: willmitchell
 */


#include "meta.hpp"

void ViaMeta::ViaMetaUI::button1EnterMenuCallback(void) {
	uiSetLEDs(SH_MODE);
	uiResetTimerMenu();
}
void ViaMeta::ViaMetaUI::button2EnterMenuCallback(void) {
	uiSetLEDs(TABLE);
	uiResetTimerMenu();
}
void ViaMeta::ViaMetaUI::button3EnterMenuCallback(void) {
	uiSetLEDs(FREQ_MODE);
	uiResetTimerMenu();
}
void ViaMeta::ViaMetaUI::button4EnterMenuCallback(void) {
	if (!LOOP_MODE && !FREQ_MODE) {
		transition(&ViaUI::aux3Menu);
	} else {
		uiSetLEDs(TRIG_MODE);
		uiResetTimerMenu();
	}
}
void ViaMeta::ViaMetaUI::button5EnterMenuCallback(void) {
	uiSetLEDs(TABLE);
	uiResetTimerMenu();
}
void ViaMeta::ViaMetaUI::button6EnterMenuCallback(void) {
	uiSetLEDs(LOOP_MODE);
	uiResetTimerMenu();
}
void ViaMeta::ViaMetaUI::aux1EnterMenuCallback(void) {
	transition(&ViaUI::button5Menu);
}
void ViaMeta::ViaMetaUI::aux2EnterMenuCallback(void) {
	uiClearLEDs();
	uiSetLEDs(LOGIC_A_MODE);
	uiResetTimerMenu();
}
void ViaMeta::ViaMetaUI::aux3EnterMenuCallback(void) {
	uiClearLEDs();
	uiSetLEDs(DRUM_MODE);
	uiResetTimerMenu();
}
void ViaMeta::ViaMetaUI::aux4EnterMenuCallback(void) {
	uiClearLEDs();
	uiSetLEDs(DAC_3_MODE);
	uiResetTimerMenu();
}

void ViaMeta::ViaMetaUI::button1TapCallback(void) {
	SH_MODE = incrementModeAndStore(SH_MODE, BUTTON1_MASK, numButton1Modes);
	handleButton1ModeChange(SH_MODE);
	uiClearLEDs();
	uiSetLEDs(SH_MODE);
	transition(&ViaUI::newModeMenu);
}
void ViaMeta::ViaMetaUI::button2TapCallback(void) {
	TABLE = incrementModeAndStore(TABLE, BUTTON2_MASK, numButton2Modes);
	handleButton2ModeChange(TABLE);
	uiClearLEDs();
	uiSetLEDs(TABLE);
	transition(&ViaUI::newModeMenu);
}
void ViaMeta::ViaMetaUI::button3TapCallback(void) {
	FREQ_MODE = incrementModeAndStore(FREQ_MODE, BUTTON3_MASK, numButton3Modes);
	handleButton3ModeChange(FREQ_MODE);
	uiClearLEDs();
	uiSetLEDs(FREQ_MODE);
	transition(&ViaUI::newModeMenu);
}
void ViaMeta::ViaMetaUI::button4TapCallback(void) {
	TRIG_MODE = incrementModeAndStore(TRIG_MODE, BUTTON4_MASK, numButton4Modes);
	handleButton4ModeChange(TRIG_MODE);
	uiClearLEDs();
	uiSetLEDs(TRIG_MODE);
	transition(&ViaUI::newModeMenu);
}
void ViaMeta::ViaMetaUI::button5TapCallback(void) {
	TABLE = decrementModeAndStore(TABLE, BUTTON5_MASK, numButton5Modes);
	handleButton5ModeChange(TABLE);
	uiClearLEDs();
	uiSetLEDs(TABLE);
	transition(&ViaUI::newModeMenu);
}
void ViaMeta::ViaMetaUI::button6TapCallback(void) {
	LOOP_MODE = incrementModeAndStore(LOOP_MODE, BUTTON6_MASK, numButton6Modes);
	handleButton6ModeChange(LOOP_MODE);
	uiClearLEDs();
	uiSetLEDs(LOOP_MODE);
	transition(&ViaUI::newModeMenu);
}

void ViaMeta::ViaMetaUI::aux1TapCallback(void) {
	transition(&ViaUI::button5Menu);
}

void ViaMeta::ViaMetaUI::aux2TapCallback(void) {
	LOGIC_A_MODE = incrementModeAndStore(LOGIC_A_MODE, AUX_MODE2_MASK, numAux2Modes);
	handleAux2ModeChange(LOGIC_A_MODE);
	uiClearLEDs();
	uiSetLEDs(LOGIC_A_MODE);
	transition(&ViaUI::newAuxModeMenu);
}

void ViaMeta::ViaMetaUI::aux3TapCallback(void) {
	DRUM_MODE = incrementModeAndStore(DRUM_MODE, AUX_MODE3_MASK, numAux3Modes);
	handleAux3ModeChange(DRUM_MODE);
	uiClearLEDs();
	uiSetLEDs(DRUM_MODE);
	transition(&ViaUI::newModeMenu);
}

void ViaMeta::ViaMetaUI::aux4TapCallback(void) {
	DAC_3_MODE = incrementModeAndStore(DAC_3_MODE, AUX_MODE4_MASK, numAux4Modes);
	handleAux4ModeChange(DAC_3_MODE);
	uiClearLEDs();
	uiSetLEDs(DAC_3_MODE);
	transition(&ViaUI::newAuxModeMenu);
}

void ViaMeta::ViaMetaUI::button1HoldCallback(void) {
	transition(&ViaUI::defaultMenu);
}
void ViaMeta::ViaMetaUI::button2HoldCallback(void) {
	transition(&ViaUI::defaultMenu);
}
void ViaMeta::ViaMetaUI::button3HoldCallback(void) {
	transition(&ViaUI::defaultMenu);
}
void ViaMeta::ViaMetaUI::button4HoldCallback(void) {
	transition(&ViaUI::defaultMenu);
}
void ViaMeta::ViaMetaUI::button5HoldCallback(void) {
	transition(&ViaUI::defaultMenu);
}
void ViaMeta::ViaMetaUI::button6HoldCallback(void) {
	transition(&ViaUI::defaultMenu);
}

void ViaMeta::ViaMetaUI::aux1HoldCallback(void) {
	transition(&ViaUI::button5Menu);
}

void ViaMeta::ViaMetaUI::aux2HoldCallback(void) {
	transition(&ViaUI::defaultMenu);
}

void ViaMeta::ViaMetaUI::aux3HoldCallback(void) {
	transition(&ViaUI::defaultMenu);
}

void ViaMeta::ViaMetaUI::aux4HoldCallback(void) {
	transition(&ViaUI::defaultMenu);
}


