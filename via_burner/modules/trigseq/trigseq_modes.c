/*
 * osc_modes.c
 *
 *  Created on: Aug 21, 2018
 *      Author: willmitchell
 */

#include "trigseq.h"

void trigseq_handleButton1ModeChange(int mode) {

	switch (mode) {
	case 0:
		trigseq_signals.parameters->sampleA = 0;
		trigseq_signals.parameters->trackA = 0;
		break;
	case 1:
		trigseq_signals.parameters->sampleA = 1;
		trigseq_signals.parameters->trackA = 0;
		break;
	case 2:
		trigseq_signals.parameters->sampleA = 0;
		trigseq_signals.parameters->trackA = 1;
		break;
	}

}

void trigseq_handleButton2ModeChange(int mode) {

	trigseq_signals.parameters->andA = mode;

	if (mode) {
		trigseq_signals.parameters->outputAEvent = SOFT_GATE_LOW;
	} else {
		trigseq_signals.parameters->outputAEvent = SOFT_GATE_HIGH;
	}

}

void trigseq_handleButton3ModeChange(int mode) {

	trigseq_signals.parameters->currentABank = trigseq_patternBank[mode];

}

void trigseq_handleButton4ModeChange(int mode) {

	switch (mode) {
	case 0:
		trigseq_signals.parameters->sampleB = 0;
		trigseq_signals.parameters->trackB = 0;
		break;
	case 1:
		trigseq_signals.parameters->sampleB = 1;
		trigseq_signals.parameters->trackB = 0;
		break;
	case 2:
		trigseq_signals.parameters->sampleB = 0;
		trigseq_signals.parameters->trackB = 1;
		break;
	}

}

void trigseq_handleButton5ModeChange(int mode) {

	trigseq_signals.parameters->andB = mode;
	if (mode) {
		trigseq_signals.parameters->outputBEvent = SOFT_GATE_LOW;
	} else {
		trigseq_signals.parameters->outputBEvent = SOFT_GATE_HIGH;
	}

}

void trigseq_handleButton6ModeChange(int mode) {

	trigseq_signals.parameters->currentBBank = trigseq_patternBank[mode];

}

void trigseq_handleAux2ModeChange(int mode) {

	trigseq_signals.parameters->auxLogicMode = mode;

}
