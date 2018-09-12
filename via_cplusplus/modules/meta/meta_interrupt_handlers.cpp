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

	signals.meta_parameters->triggerSignal = 0;

	signals.meta_parameters->gateSignal = 1 * signals.meta_parameters->gateOn;

	signals.drum_parameters->trigger = 0;

}

void ViaMeta::mainFallingEdgeCallback(void) {

	signals.meta_parameters->gateSignal = 0;

}

void ViaMeta::auxRisingEdgeCallback(void) {

	signals.meta_parameters->freeze = 0;

}
void ViaMeta::auxFallingEdgeCallback(void)
{
	signals.meta_parameters->freeze = 1;

}

void ViaMeta::buttonPressedCallback(void) {

	signals.meta_parameters->triggerSignal = 0;

	signals.meta_parameters->gateSignal = 1 * signals.meta_parameters->gateOn;

	signals.drum_parameters->trigger = 0;

}
void ViaMeta::buttonReleasedCallback(void) {

	signals.meta_parameters->gateSignal = 0;

}

void ViaMeta::ioProcessCallback(void) {

	// no need

}

void ViaMeta::halfTransferCallback(void) {

	via_setLogicOut(signals.outputs, 0, runtimeDisplay);

	simpleWavetableParameters * wavetable_parameters = signals.wavetable_parameters;
	metaControllerParameters * meta_parameters = signals.meta_parameters;
	audioRateInputs * inputs = signals.inputs;
	audioRateOutputs * outputs = signals.outputs;

	(*metaControllerGenerateIncrements)(meta_parameters, signals.inputs);
	metaControllerAdvancePhase(meta_parameters, &phaseModPWMTables);
	wavetable_parameters->phase = meta_parameters->ghostPhase;
	outputs->dac2Samples[0] = simpleWavetableAdvance(wavetable_parameters, wavetableRead);
	(*meta_drumMode)(signals, 0);
	outputs->auxLogic[0] = EXPAND_LOGIC_LOW_MASK << (16 * wavetable_parameters->delta);
	(*meta_calculateDac3)(signals, 0);
	(*meta_calculateLogicA)(signals, 0);
	(*meta_calculateSH)(signals, 0);

}

void ViaMeta::transferCompleteCallback(void) {

	via_setLogicOut(signals.outputs, 1, runtimeDisplay);

	simpleWavetableParameters * wavetable_parameters = signals.wavetable_parameters;
	metaControllerParameters * meta_parameters = signals.meta_parameters;
	audioRateInputs * inputs = signals.inputs;
	audioRateOutputs * outputs = signals.outputs;

	(*metaControllerGenerateIncrements)(meta_parameters, signals.inputs);
	metaControllerAdvancePhase(meta_parameters, &phaseModPWMTables);
	wavetable_parameters->phase = meta_parameters->ghostPhase;
	outputs->dac2Samples[1] = simpleWavetableAdvance(wavetable_parameters, wavetableRead);
	(*meta_drumMode)(1);
	outputs->auxLogic[1] = EXPAND_LOGIC_LOW_MASK << (16 * wavetable_parameters->delta);
	(*meta_calculateDac3)(1);
	(*meta_calculateLogicA)(1);
	(*meta_calculateSH)(1);

}

void ViaMeta::slowConversionCallback(void) {

	controlRateInputs * controls = signals.controls;

	via_updateControlRateInputs(controls);
	simpleWavetableParseControls(controls, signals.wavetable_parameters);
	(*metaControllerParseControls)(controls, signals.inputs, signals.meta_parameters);
	simpleEnvelopeParseControls (controls, signals.inputs, signals.drum_parameters);

	if (runtimeDisplay) {
		int sample = signals.outputs->dac2Samples[0];
		int lastPhaseValue = signals.wavetable_parameters->phase;
		SET_RED_LED(sample * (lastPhaseValue >> 24));
		SET_BLUE_LED(sample * (!(lastPhaseValue >> 24)));
		SET_GREEN_LED(__USAT((signals.inputs->cv3Samples[0] + controls->knob3Value - 2048), 12) * sample >> 12);
	}

}


