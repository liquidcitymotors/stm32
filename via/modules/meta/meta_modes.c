/*
 * meta_modes.c
 *
 *  Created on: Aug 29, 2018
 *      Author: willmitchell
 */

#include "meta.h"

void meta_handleButton1ModeChange(int mode) {

	switch (mode) {
	case nosampleandhold:
		meta_calculateSH = meta_calculateSHMode1;
		break;
	case a:
		meta_calculateSH = meta_calculateSHMode2;
		break;
	case b:
		meta_calculateSH = meta_calculateSHMode3;
		break;
	case ab:
		meta_calculateSH = meta_calculateSHMode4;
		break;
	case halfdecimate:
		meta_calculateSH = meta_calculateSHMode5;
		break;
	case meta_decimate:
		meta_calculateSH = meta_calculateSHMode6;
		break;
	}

	SH_A_TRACK;
	SH_B_TRACK;
}

void meta_handleButton2ModeChange(int mode) {

	meta_switchWavetable(meta_wavetableArray[FREQ_MODE][mode], &meta_signals);

}

void meta_handleButton3ModeChange(int mode) {

	switch (mode) {
	case audio:
		if (!LOOP_MODE) {
			metaControllerGenerateIncrements = metaControllerGenerateIncrementsDrum;
			metaControllerParseControls = metaControllerParseControlsDrum;
			meta_drumMode = meta_drumModeOn;
			metaControllerLoopHandler = handleLoopOn;
			meta_signals.meta_parameters->loopMode = 1;
			meta_handleButton4ModeChange(0);
			meta_handleAux3ModeChange(DRUM_MODE);
		} else {
			metaControllerParseControls = metaControllerParseControlsAudio;
			metaControllerGenerateIncrements = metaControllerGenerateIncrementsAudio;
			meta_drumMode = meta_drumModeOff;
		}
		meta_switchWavetable(meta_wavetableArray[mode][TABLE], &meta_signals);
		//updateRGB = updateRGBAudio;
		break;
	case env:
		metaControllerParseControls = metaControllerParseControlsEnv;
		metaControllerGenerateIncrements = metaControllerGenerateIncrementsEnv;
		if (!LOOP_MODE) {
			meta_switchWavetable(meta_wavetableArray[mode][TABLE], &meta_signals);
			meta_signals.meta_parameters->fm = meta_drumFullScale;
			meta_signals.wavetable_parameters->morphScale = meta_drumFullScale;
			meta_drumMode = meta_drumModeOff;
			metaControllerLoopHandler = handleLoopOff;
			meta_signals.meta_parameters->loopMode = 0;
			meta_handleButton4ModeChange(TRIG_MODE);
		}
		//updateRGB = updateRGBSubAudio;
		break;
	case seq:
		metaControllerParseControls = metaControllerParseControlsSeq;
		metaControllerGenerateIncrements = metaControllerGenerateIncrementsSeq;
		meta_switchWavetable(meta_wavetableArray[mode][TABLE], &meta_signals);
		break;
	}

}

void meta_handleButton4ModeChange(int mode) {

	switch (mode) {
	case noretrigger:
		metaControllerIncrementArbiter = noRetrigAttackState;
		break;
	case meta_hardsync:
		metaControllerIncrementArbiter = hardSyncAttackState;
		break;
	case nongatedretrigger:
		metaControllerIncrementArbiter = envAttackState;
		break;
	case gated:
		metaControllerIncrementArbiter = gateAttackState;
		meta_signals.meta_parameters->phase = 0;
		meta_signals.meta_parameters->gateOn = 1;
		break;
	case meta_pendulum:
		metaControllerIncrementArbiter = pendulumForwardAttackState;
		meta_signals.meta_parameters->gateOn = 0;
		break;
	}

}

void meta_handleButton5ModeChange(int mode) {

	meta_switchWavetable(meta_wavetableArray[FREQ_MODE][mode], &meta_signals);

}

void meta_handleButton6ModeChange(int mode) {

	switch (mode) {
	case noloop:
		if (!FREQ_MODE) {
			meta_handleAux3ModeChange(DRUM_MODE);
			meta_handleButton3ModeChange(0);
			meta_handleButton4ModeChange(0);
		} else {
			metaControllerLoopHandler = handleLoopOff;
			meta_signals.meta_parameters->loopMode = 0;
		}
		break;
	case looping:
		if (!FREQ_MODE) {
			meta_signals.meta_parameters->fm = meta_drumFullScale;
			meta_signals.wavetable_parameters->morphScale = meta_drumFullScale;
			meta_handleButton3ModeChange(0);
			meta_handleButton4ModeChange(TRIG_MODE);
		}
		metaControllerLoopHandler = handleLoopOn;
		meta_signals.meta_parameters->loopMode = 1;
		break;
	}

}

void meta_handleAux1ModeChange(int mode) {


}

void meta_handleAux2ModeChange(int mode) {

	switch (mode) {
	case releaseGate:
		meta_calculateLogicA = meta_calculateLogicAReleaseGate;
		break;
	case attackGate:
		meta_calculateLogicA = meta_calculateLogicAAttackGate;
		break;
	}

}

void meta_handleAux3ModeChange(int mode) {

	switch (mode) {
	case pitchMorphAmp:
		meta_signals.meta_parameters->fm = meta_signals.drum_parameters->output;
		meta_signals.wavetable_parameters->morphScale = meta_signals.drum_parameters->output;
		break;
	case morphAmp:
		meta_signals.meta_parameters->fm = meta_drumFullScale;
		meta_signals.wavetable_parameters->morphScale = meta_signals.drum_parameters->output;
		break;
	case pitchAmp:
		meta_signals.meta_parameters->fm = meta_signals.drum_parameters->output;
		meta_signals.wavetable_parameters->morphScale = meta_drumFullScale;
		break;
	case amp:
		meta_signals.meta_parameters->fm = meta_drumFullScale;
		meta_signals.wavetable_parameters->morphScale = meta_drumFullScale;
		break;
	}

}

void meta_handleAux4ModeChange(int mode) {

	switch (mode) {
	case phasor:
		meta_calculateDac3 = meta_calculateDac3Phasor;
		break;
	case contour:
		meta_calculateDac3 = meta_calculateDac3Contour;
		break;
	}

}



