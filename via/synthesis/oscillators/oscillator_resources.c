/*
 * oscillator_resources.c
 *
 *  Created on: Aug 26, 2018
 *      Author: willmitchell
 */

#include "oscillators.h"


/*
 *
 * Pll with scale grid multiplier
 *
 */


void pllMultiplierParseControls(controlRateInputs * controls, audioRateInputs * input,
		pllMultiplierParameters * parameters) {

	Scale * scale = parameters->scale;

	int noteIndex = __USAT(controls->knob1Value + controls->cv1Value - 2048, 12) >> 5;
	int rootMod = parameters->rootMod[0];
	rootMod = rootMod >> 4;
	int	rootIndex = (__USAT(controls->knob2Value + rootMod, 12)) >> scale->t2Bitshift;

	parameters->fracMultiplier = scale->grid[rootIndex][noteIndex]->fractionalPart;
	parameters->intMultiplier = scale->grid[rootIndex][noteIndex]->integerPart;
	parameters->gcd = scale->grid[rootIndex][noteIndex]->fundamentalDivision;

}
void pllMultiplierDoPLL(pllMultiplierParameters * parameters) {

	static int pllCounter;
	static int lastMultiplier;

	pllCounter++;

	pllCounter *= parameters->pllReset;
	parameters->pllReset = 1;
	parameters->phaseReset = 1;

	int phase = (parameters->phaseSignal + parameters->phaseOffset) & ((1 << 25) - 1);

	// this switch is always 0 unless pllCounter is 0, in which case its sync mode

#define IGNORE 0
#define PLL_OFF 1
#define TRUE_PLL 2
#define HARD_RESET 3

	if (pllCounter >= parameters->gcd) {

		switch (parameters->syncMode) {
			case IGNORE:
				break;
			case PLL_OFF:
				parameters->pllNudge = 0;
				pllCounter = 0;
				break;
			case TRUE_PLL:
				parameters->pllNudge = (((phase >> 24)*WAVETABLE_LENGTH - phase) << 16);
				pllCounter = parameters->gcd;
				break;
			case HARD_RESET:
				parameters->phaseReset = 0;
				pllCounter = 0;
				break;
			default:
				pllCounter = 0;
				break;

		}
	}

	int multKey = parameters->fracMultiplier + parameters->intMultiplier;

	if (lastMultiplier != multKey) {
		EXPAND_LOGIC_HIGH
	}

	lastMultiplier = multKey;

}
void pllMultiplierGenerateFrequency(pllMultiplierParameters * parameters) {

	uint32_t increment = ((((uint64_t)((uint64_t)WAVETABLE_LENGTH << 18) + parameters->pllNudge)) / (parameters->periodCount));
	increment = fix48_mul(increment >> 8, parameters->fracMultiplier) + fix16_mul(increment >> 8, parameters->intMultiplier);
	parameters->increment = __USAT(increment, 24);

}


/*
 *
 * Meta oscillator controller
 *
 */


void metaControllerParseControlsAudio(controlRateInputs * controls, metaControllerParameters * parameters) {

	// time1 is coarse, time2 is fine, release time = attack time

	parameters->timeBase1 = fix16_mul(fix16_mul(expoTable[controls->knob1Value] >> 5,
			expoTable[(controls->knob2Value >> 3) + 200]),
			expoTable[controls->cv1Value] >> 2);

	parameters->dutyCycleBase = 32767;

}

void metaControllerParseControlsEnv(controlRateInputs * controls, metaControllerParameters * parameters) {

	// t1 is attack time (attackIncrements), t2 is release time (releaseIncrements)

	parameters->timeBase1 = expoTable[((4095 - controls->knob1Value) >> 2) * 3] >> 8;
	parameters->timeBase2 = expoTable[((4095 - controls->knob2Value) >> 2) * 3] >> 10;

	parameters->dutyCycleBase = 32767;

}

void metaControllerParseControlsSeq(controlRateInputs * controls, metaControllerParameters * parameters) {

	// t1 is cycle time, t2 is used to feed the duty cycle input for getSamples

	parameters->timeBase1 = fix16_mul(expoTable[4095 - controls->knob1Value] >> 9,
			expoTable[4095 - controls->cv1Value] >> 9);

	parameters->dutyCycleBase = controls->knob2Value << 4;

}


// fill incrementValues 1 and 2 with the attack and release time increments, respectively

void metaControllerGenerateIncrementsAudio(metaControllerParameters * parameters, audioRateInputs * inputs) {

	int fm = -inputs->cv2Samples[0];
	fm += 16384;
	parameters->increment1 = fix16_mul(parameters->timeBase1, fm);
	parameters->increment2 = parameters->increment1;

	parameters->dutyCycle = parameters->dutyCycleBase;

}

void metaControllerGenerateIncrementsEnv(metaControllerParameters * parameters, audioRateInputs * inputs) {

//	int parameters->increment2Multiplier = -inputs->cv2Samples[0];
//
//	parameters->increment2Multiplier += 32767;
//
//	parameters->increment2Multiplier = expoTable[parameters->increment2Multiplier >> 4] >> 8;

	parameters->increment1 = parameters->timeBase1;
	parameters->increment2 = parameters->timeBase2;

	parameters->dutyCycle = parameters->dutyCycleBase;

}


void metaControllerGenerateIncrementsSeq(metaControllerParameters * parameters, audioRateInputs * inputs) {

	parameters->increment1 = parameters->timeBase1;
	parameters->increment2 = parameters->timeBase2;

	int dutyCycleMod = -inputs->cv2Samples[0];

	parameters->dutyCycle = __USAT(parameters->dutyCycleBase + dutyCycleMod, 16);

}

int metaControllerAdvancePhase(metaControllerParameters * parameters) {

	static int previousPhase;
	static int previousPhaseEvent;
	int phaseWrapper;

	int increment = (*metaControllerIncrementArbiter)(parameters);

	int phase = parameters->phase;

	phase = (phase + __SSAT(increment, 24)) * (parameters->oscillatorOn);

	phaseWrapper = 0;

	// add wavetable length if phase < 0

	phaseWrapper += ((uint32_t)(phase) >> 31) * WAVETABLE_LENGTH;

	// subtract wavetable length if phase > wavetable length

	phaseWrapper -= ((uint32_t)(WAVETABLE_LENGTH - phase) >> 31) * WAVETABLE_LENGTH;

	// apply the wrapping
	// no effect if the phase is in bounds

	phase += phaseWrapper;

	// log a -1 if the max value index of the wavetable is traversed from the left
	// log a 1 if traversed from the right
	// do this by subtracting the sign bit of the last phase from the current phase, both less the max phase index
	// this adds cruft to the wrap indicators, but that is deterministic and can be parsed out

	int atBIndicator = ((uint32_t)(phase - AT_B_PHASE) >> 31) - ((uint32_t)(previousPhase - AT_B_PHASE) >> 31);

	phaseWrapper += atBIndicator;

	// stick the position to WAVETABLE AT_B_PHASE
	phase += (AT_B_PHASE - phase) * (abs(atBIndicator) & parameters->gateSignal);

	parameters->phaseEvent = phaseWrapper;

	// TODO rewrite logic parsing function

	// store the current phase
	previousPhase = phase;
	previousPhaseEvent = phaseWrapper;

	parameters->oscillatorOn = (*metaControllerLoopHandler)(parameters);

	return phase;

}


int handleLoopOn(metaControllerParameters * parameters) {
	return 1;
}

int handleLoopOff(metaControllerParameters * parameters) {

	static int oscillatorActiveState;

	oscillatorActiveState |= !(parameters->triggerSignal);

	oscillatorActiveState &= !(abs(parameters->phaseEvent) >> 24);

	return oscillatorActiveState;

}

/*
 *
 * Meta Controller Increment Arbiter
 * Based on the phase and the logic inputs, handle
 *
 */

int noRetrigAttackState(metaControllerParameters * parameters) {

	switch (parameters->phaseEvent) {

	case (AT_B_FROM_ATTACK):
		metaControllerIncrementArbiter = noRetrigReleaseState;
		return parameters->increment2;

	default:
		return parameters->increment1;

	}
}

int noRetrigReleaseState(metaControllerParameters * parameters) {

	switch (parameters->phaseEvent) {

	case (AT_A_FROM_RELEASE):
		metaControllerIncrementArbiter = noRetrigAttackState;
		return parameters->increment1;

	default:
		return parameters->increment2;

	};
}

/*
 *
 * hardSyncStateMachine
 *
 */

int hardSyncAttackState(metaControllerParameters * parameters) {

	switch (parameters->phaseEvent) {

	case (AT_B_FROM_ATTACK):
		metaControllerIncrementArbiter = hardSyncReleaseState;
		return parameters->increment2;

	default:
		return parameters->increment1;

	}
}

int hardSyncReleaseState(metaControllerParameters * parameters) {

	if (parameters->triggerSignal == 0) {
		metaControllerIncrementArbiter = hardSyncAttackState;
	}

	switch (parameters->phaseEvent) {

	case (AT_A_FROM_RELEASE):
		metaControllerIncrementArbiter = hardSyncAttackState;
		return parameters->increment1;

	default:
		return parameters->increment2;

	};
}

/*
 *
 * envStateMachine
 *
 */

int envAttackState(metaControllerParameters * parameters) {

	switch (parameters->phaseEvent) {

	case (AT_B_FROM_ATTACK):
		metaControllerIncrementArbiter = envReleaseState;
		return parameters->increment2;

	default:
		return parameters->increment1;

	}
}

int envReleaseState(metaControllerParameters * parameters) {

	if (parameters->triggerSignal == 0) {
		metaControllerIncrementArbiter = envRetriggerState;
		return -parameters->increment1;
	}

	switch (parameters->phaseEvent) {

	case (AT_A_FROM_RELEASE):
		metaControllerIncrementArbiter = envAttackState;
		return parameters->increment1;

	default:
		return parameters->increment2;

	};
}

int envRetriggerState(metaControllerParameters * parameters) {

	switch (parameters->phaseEvent) {

	case (AT_B_FROM_RELEASE):
		metaControllerIncrementArbiter = envReleaseState;
		return parameters->increment2;

	default:
		return -parameters->increment1;

	}
}

/*
 *
 * gateStateMachine loop
 *
 */

int gateAttackState(metaControllerParameters * parameters) {

	int gateWLoopProtection = parameters->gateSignal | parameters->loopMode;

	if (gateWLoopProtection == 0) {
		metaControllerIncrementArbiter = gateReleaseReverseState;
		return -parameters->increment2;
	}

	switch (parameters->phaseEvent) {

	case (AT_B_FROM_ATTACK):
		metaControllerIncrementArbiter = gatedState;
		return 0;

	default:
		return parameters->increment1;

	}
}

int gateReleaseReverseState(metaControllerParameters * parameters) {

	if (parameters->gateSignal == 1) {
		metaControllerIncrementArbiter = gateAttackState;
		return parameters->increment1;
	}

	switch (parameters->phaseEvent) {

	case (AT_A_FROM_ATTACK):
		metaControllerIncrementArbiter = gateAttackState;
		return parameters->increment1;

	default:
		return -parameters->increment2;

	};
}

int gatedState(metaControllerParameters * parameters) {

	if (parameters->gateSignal == 0) {
		metaControllerIncrementArbiter = gateReleaseState;
		return parameters->increment2;
	} else {
		return 0;
	}

}

int gateReleaseState(metaControllerParameters * parameters) {

	if (parameters->gateSignal == 1) {
		metaControllerIncrementArbiter = gateRetriggerState;
		return -parameters->increment1;
	}

	switch (parameters->phaseEvent) {

	case (AT_A_FROM_RELEASE):
		metaControllerIncrementArbiter = gateAttackState;
		return parameters->increment1;

	default:
		return parameters->increment2;

	};
}

int gateRetriggerState(metaControllerParameters * parameters) {

	int gateWLoopProtection = parameters->gateSignal | parameters->loopMode;

	if (gateWLoopProtection == 0) {
		metaControllerIncrementArbiter = gateReleaseState;
		return parameters->increment2;
	}

	switch (parameters->phaseEvent) {

	case (AT_B_FROM_RELEASE):
		metaControllerIncrementArbiter = gateReleaseState;
		return parameters->increment2;

	default:
		return -parameters->increment1;

	}
}

/*
 *
 * pendulumStateMachine
 *
 */

int pendulumRestingState(metaControllerParameters * parameters) {
	if (parameters->triggerSignal == 0) {
		metaControllerIncrementArbiter = pendulumForwardAttackState;
		return parameters->increment1;
	} else {
		return 0;
	}
}

int pendulumForwardAttackState(metaControllerParameters * parameters) {

	if (parameters->triggerSignal == 0 && parameters->oscillatorOn) {
		metaControllerIncrementArbiter = pendulumReverseAttackState;
		return 0;
	}

	switch (parameters->phaseEvent) {

	case (AT_B_FROM_ATTACK):
		metaControllerIncrementArbiter = pendulumForwardReleaseState;
		return parameters->increment2;

	default:
		return parameters->increment1;

	}

}

int pendulumReverseAttackState(metaControllerParameters * parameters) {


	if (parameters->triggerSignal == 0) {
		metaControllerIncrementArbiter = pendulumForwardAttackState;
		return parameters->increment1;
	}

	switch (parameters->phaseEvent) {

	case (AT_A_FROM_ATTACK):
		metaControllerIncrementArbiter = pendulumReverseReleaseState;
		return -parameters->increment2;

	default:
		return -parameters->increment1;

	}

}

int pendulumForwardReleaseState(metaControllerParameters * parameters) {

	if (parameters->triggerSignal == 0) {
		metaControllerIncrementArbiter = pendulumReverseReleaseState;
		return -parameters->increment2;
	}

	switch (parameters->phaseEvent) {

	case (AT_A_FROM_RELEASE):
		metaControllerIncrementArbiter = pendulumForwardAttackState;
		return parameters->increment1;

	default:
		return parameters->increment2;

	}

}

int pendulumReverseReleaseState(metaControllerParameters * parameters) {

	if (parameters->triggerSignal == 0) {
		metaControllerIncrementArbiter = pendulumForwardReleaseState;
		return parameters->increment2;
	}

	switch (parameters->phaseEvent) {

	case (AT_B_FROM_RELEASE):
		metaControllerIncrementArbiter = pendulumReverseAttackState;
		return -parameters->increment1;

	default:
		return -parameters->increment2;

	}

}


