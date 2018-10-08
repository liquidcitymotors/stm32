
#include "scanner.hpp"

void ViaScanner::ViaScannerUI::initialize(void) {

#ifdef BUILD_VIRTUAL

	button1 = (int32_t *) &this_module.button1Input;
	button2 = (int32_t *) &this_module.button2Input;
	button3 = (int32_t *) &this_module.button3Input;
	button4 = (int32_t *) &this_module.button4Input;
	button5 = (int32_t *) &this_module.button5Input;
	button6 = (int32_t *) &this_module.button6Input;

#endif

#ifdef BUILD_F373

	tsl_user_Init();

	button1 = (int32_t *) &BUTTON1SENSOR;
	button2 = (int32_t *) &BUTTON2SENSOR;
	button3 = (int32_t *) &BUTTON3SENSOR;
	button4 = (int32_t *) &BUTTON4SENSOR;
	button5 = (int32_t *) &BUTTON5SENSOR;
	button6 = (int32_t *) &BUTTON6SENSOR;

	initializeMemory();

#endif

	loadFromEEPROM(0);

#ifdef BUILD_F373

	this_module.handleButton1ModeChange(this_module.scannerUI.button1Mode);
	this_module.handleButton2ModeChange(this_module.scannerUI.button2Mode);
	this_module.handleButton3ModeChange(this_module.scannerUI.button3Mode);
	this_module.handleButton4ModeChange(this_module.scannerUI.button4Mode);
	this_module.handleButton5ModeChange(this_module.scannerUI.button5Mode);
	this_module.handleButton6ModeChange(this_module.scannerUI.button6Mode);

#endif
	state = &ViaUI::defaultMenu;
	transition(&ViaUI::defaultMenu);

}

void scannerTouchLink (void * uiVoid) {
	ViaScanner::ViaScannerUI * ui = (ViaScanner::ViaScannerUI *) uiVoid;
	ui->dispatch(SENSOR_EVENT_SIG);
}

void ViaScanner::ViaScannerUI::defaultEnterMenuCallback(void) {
	this_module.clearLEDs();
	this_module.runtimeDisplay = 1;
}
void ViaScanner::ViaScannerUI::newModeEnterMenuCallback(void) {
	;
}
void ViaScanner::ViaScannerUI::newAuxModeEnterMenuCallback(void) {
	;
}
void ViaScanner::ViaScannerUI::presetEnterMenuCallback(void) {
	;
}

void ViaScanner::ViaScannerUI::button1EnterMenuCallback(void) {
	this_module.runtimeDisplay = 0;
	this_module.clearLEDs();
	this_module.clearRGB();
	this_module.setLEDs(X_TABLE);
	resetTimerMenu();
}
void ViaScanner::ViaScannerUI::button2EnterMenuCallback(void) {
	this_module.runtimeDisplay = 0;
	this_module.clearLEDs();
	this_module.clearRGB();
	this_module.setLEDs(Y_TABLE);
	resetTimerMenu();
}
void ViaScanner::ViaScannerUI::button3EnterMenuCallback(void) {
	this_module.runtimeDisplay = 0;
	this_module.clearLEDs();
	this_module.clearRGB();
	this_module.setLEDs(X_TABLE);
	resetTimerMenu();
}
void ViaScanner::ViaScannerUI::button4EnterMenuCallback(void) {
	this_module.runtimeDisplay = 0;
	this_module.clearLEDs();
	this_module.clearRGB();
	this_module.setLEDs(SYNC_MODE);
	resetTimerMenu();
}
void ViaScanner::ViaScannerUI::button5EnterMenuCallback(void) {
	this_module.runtimeDisplay = 0;
	this_module.clearLEDs();
	this_module.clearRGB();
	this_module.setLEDs(Y_TABLE);
	resetTimerMenu();
}
void ViaScanner::ViaScannerUI::button6EnterMenuCallback(void) {
	this_module.runtimeDisplay = 0;
	this_module.clearLEDs();
	this_module.clearRGB();
	this_module.setLEDs(TERRAIN);
	resetTimerMenu();
}
void ViaScanner::ViaScannerUI::aux1EnterMenuCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}
void ViaScanner::ViaScannerUI::aux2EnterMenuCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}
void ViaScanner::ViaScannerUI::aux3EnterMenuCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}
void ViaScanner::ViaScannerUI::aux4EnterMenuCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}

void ViaScanner::ViaScannerUI::button1TapCallback(void) {
	X_TABLE = incrementModeAndStore(X_TABLE, BUTTON1_MASK, numButton1Modes, BUTTON1_MASK);
	this_module.handleButton1ModeChange(X_TABLE);
	this_module.clearLEDs();
	this_module.setLEDs(X_TABLE);
	transition(&ViaScanner::ViaScannerUI::newModeMenu);
}
void ViaScanner::ViaScannerUI::button2TapCallback(void) {
	Y_TABLE = incrementModeAndStore(Y_TABLE, BUTTON2_MASK, numButton2Modes, BUTTON2_MASK);
	this_module.handleButton2ModeChange(Y_TABLE);
	this_module.clearLEDs();
	this_module.setLEDs(Y_TABLE);
	transition(&ViaScanner::ViaScannerUI::newModeMenu);
}
void ViaScanner::ViaScannerUI::button3TapCallback(void) {
	X_TABLE = decrementModeAndStore(X_TABLE, BUTTON3_MASK, numButton3Modes, BUTTON3_MASK);
	this_module.handleButton3ModeChange(X_TABLE);
	this_module.clearLEDs();
	this_module.setLEDs(X_TABLE);
	transition(&ViaScanner::ViaScannerUI::newModeMenu);
}
void ViaScanner::ViaScannerUI::button4TapCallback(void) {
	SYNC_MODE = incrementModeAndStore(SYNC_MODE, BUTTON4_MASK, numButton4Modes, BUTTON4_MASK);
	this_module.handleButton4ModeChange(SYNC_MODE);
	this_module.clearLEDs();
	this_module.setLEDs(SYNC_MODE);
	transition(&ViaScanner::ViaScannerUI::newModeMenu);
}
void ViaScanner::ViaScannerUI::button5TapCallback(void) {
	Y_TABLE = decrementModeAndStore(Y_TABLE, BUTTON5_MASK, numButton5Modes, BUTTON5_MASK);
	this_module.handleButton5ModeChange(Y_TABLE);
	this_module.clearLEDs();
	this_module.setLEDs(Y_TABLE);
	transition(&ViaScanner::ViaScannerUI::newModeMenu);
}
void ViaScanner::ViaScannerUI::button6TapCallback(void) {
	TERRAIN = incrementModeAndStore(TERRAIN, BUTTON6_MASK,
			numButton6Modes, BUTTON6_MASK);
	this_module.handleButton6ModeChange(TERRAIN);
	this_module.clearLEDs();
	this_module.setLEDs(TERRAIN);
	transition(&ViaScanner::ViaScannerUI::newModeMenu);
}

void ViaScanner::ViaScannerUI::aux1TapCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}

void ViaScanner::ViaScannerUI::aux2TapCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}

void ViaScanner::ViaScannerUI::aux3TapCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}

void ViaScanner::ViaScannerUI::aux4TapCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}

void ViaScanner::ViaScannerUI::button1HoldCallback(void) {
	transition(&ViaScanner::ViaScannerUI::defaultMenu);
}
void ViaScanner::ViaScannerUI::button2HoldCallback(void) {
	transition(&ViaScanner::ViaScannerUI::defaultMenu);
}
void ViaScanner::ViaScannerUI::button3HoldCallback(void) {
	transition(&ViaScanner::ViaScannerUI::defaultMenu);
}
void ViaScanner::ViaScannerUI::button4HoldCallback(void) {
	transition(&ViaScanner::ViaScannerUI::defaultMenu);
}
void ViaScanner::ViaScannerUI::button5HoldCallback(void) {
	transition(&ViaScanner::ViaScannerUI::defaultMenu);
}
void ViaScanner::ViaScannerUI::button6HoldCallback(void) {
	transition(&ViaScanner::ViaScannerUI::defaultMenu);
}
void ViaScanner::ViaScannerUI::aux1HoldCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}
void ViaScanner::ViaScannerUI::aux2HoldCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}
void ViaScanner::ViaScannerUI::aux3HoldCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}
void ViaScanner::ViaScannerUI::aux4HoldCallback(void) {
	transition(&ViaScanner::ViaScannerUI::button5Menu);
}

