/*
 * meta_interrupt_handlers.cpp
 *
 *  Created on: Sep 12, 2018
 *      Author: willmitchell
 */


/*
 * meta_interrupt_handlers.c
 *
 *  Created on: Aug 29, 2018
 *      Author: willmitchell
 */

#include "meta.hpp"

const uint32_t phaseModPWMTables[33][65] = {phaseModPWM_0, phaseModPWM_1, phaseModPWM_2, phaseModPWM_3, phaseModPWM_4, phaseModPWM_5, phaseModPWM_6, phaseModPWM_7, phaseModPWM_8, phaseModPWM_9, phaseModPWM_10, phaseModPWM_11, phaseModPWM_12, phaseModPWM_13, phaseModPWM_14, phaseModPWM_15, phaseModPWM_16, phaseModPWM_17, phaseModPWM_18, phaseModPWM_19, phaseModPWM_20, phaseModPWM_21, phaseModPWM_22, phaseModPWM_23, phaseModPWM_24, phaseModPWM_25, phaseModPWM_26, phaseModPWM_27, phaseModPWM_28, phaseModPWM_29, phaseModPWM_30, phaseModPWM_31, phaseModPWM_32};

void ViaMeta::mainRisingEdgeCallback(void) {

	metaController.triggerSignal = 0;

	metaController.gateSignal = 1 * metaController.gateOn;

	drumEnvelope.trigger = 0;

}

void ViaMeta::mainFallingEdgeCallback(void) {

	metaController.gateSignal = 0;

}

void ViaMeta::auxRisingEdgeCallback(void) {

	metaController.freeze = 0;

}
void ViaMeta::auxFallingEdgeCallback(void)
{
	metaController.freeze = 1;

}

void ViaMeta::buttonPressedCallback(void) {

	metaController.triggerSignal = 0;

	metaController.gateSignal = 1 * metaController.gateOn;

	drumEnvelope.trigger = 0;

}
void ViaMeta::buttonReleasedCallback(void) {

	metaController.gateSignal = 0;

}

void ViaMeta::ioProcessCallback(void) {

	// no need

}

void ViaMeta::halfTransferCallback(void) {

	system.setLogicOut(0, runtimeDisplay);


	metaController.generateIncrementsExternal(&system.inputs);
	metaController.advancePhase((uint32_t *) phaseModPWMTables);
	metaWavetable.phase = metaController.ghostPhase;
	system.outputs.dac2Samples[0] = metaWavetable.advance((uint32_t *) wavetableRead);
	(this->*drumMode)(0);
	system.outputs.auxLogic[0] = EXPAND_LOGIC_LOW_MASK << (16 * metaWavetable.delta);
	(this->*calculateDac3)(0);
	(this->*calculateLogicA)(0);
	(this->*calculateSH)(0);

}

void ViaMeta::transferCompleteCallback(void) {

	system.setLogicOut(1, runtimeDisplay);

	metaController.generateIncrementsExternal(&system.inputs);
	metaController.advancePhase((uint32_t *) phaseModPWMTables);
	metaWavetable.phase = metaController.ghostPhase;
	system.outputs.dac2Samples[1] = metaWavetable.advance((uint32_t *) wavetableRead);
	(this->*drumMode)(1);
	system.outputs.auxLogic[1] = EXPAND_LOGIC_LOW_MASK << (16 * metaWavetable.delta);
	(this->*calculateDac3)(1);
	(this->*calculateLogicA)(1);
	(this->*calculateSH)(1);

}

void ViaMeta::slowConversionCallback(void) {


	system.controls.update();
	metaWavetable.parseControls(&system.controls);
	metaController.parseControlsExternal(&system.controls, &system.inputs);
	drumEnvelope.parseControls(&system.controls, &system.inputs);

	if (runtimeDisplay) {
		int sample = system.outputs.dac2Samples[0];
		int lastPhaseValue = metaWavetable.phase;
		SET_RED_LED(sample * (lastPhaseValue >> 24));
		SET_BLUE_LED(sample * (!(lastPhaseValue >> 24)));
		SET_GREEN_LED((__USAT((system.inputs.cv3Samples[0] + system.controls.knob3Value - 2048), 12)
				* sample) >> 12);
	}

}


