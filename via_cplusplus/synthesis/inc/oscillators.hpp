/*
 * oscillators.h
 *
 *  Created on: Aug 18, 2018
 *      Author: willmitchell
 */

#ifndef INC_OSCILLATORS_HPP_
#define INC_OSCILLATORS_HPP_

#include <via_platform_binding.hpp>
#include "tables.hpp"

#define WAVETABLE_LENGTH 33554432
#define NEGATIVE_WAVETABLE_LENGTH -33554432 // wavetable length in 16 bit fixed point (512 << 16)
#define AT_B_PHASE 16777216 // wavetable midpoint in 16 bit fixed point (256 << 16)

// phase events
#define NO_EVENT 0
#define AT_A_FROM_RELEASE -WAVETABLE_LENGTH + 1
#define AT_A_FROM_ATTACK WAVETABLE_LENGTH - 1
#define AT_B_FROM_ATTACK -1
#define AT_B_FROM_RELEASE 1

/*
 *
 * Oscillators
 *
 */

// simplest wavetable, provide a phase and a morph

class SimpleWavetable {

public:

	int morphBase = 0;
	int16_t * morphMod;
	int16_t * morphScale;
	int phase = 0;
	uint32_t tableSize = 0;

	// results
	int delta = 0;

	void parseControls(ViaControls * controls);
	int advance(uint32_t * wavetable);

};

// cheap version of that with bilinear interpolation

class CheapWavetable {

public:

	int morphBase = 0;
	int16_t * morphMod;
	int phase = 0;
	uint32_t tableSize = 0;

	void parseControls(ViaControls * controls);
	int advance(uint32_t * wavetable);

};


/*
 *
 * Shared resources
 *
 */

// meta oscillator controller

class MetaController {

	int previousGhostPhase = 0;
	int previousPhase = 0;

public:

	int timeBase1 = 0;
	int timeBase2 = 0;
	int dutyCycleBase = 0;
	int triggerSignal = 0;
	int gateSignal = 0;
	int freeze = 0;
	int gateOn = 0;
	uint32_t loopMode = 0;

	int increment1 = 0;
	int increment2 = 0;
	int dutyCycle = 0;
	int lastPhase = 0;
	int oscillatorOn = 0;
	int16_t * fm;

	int phase = 0;
	int ghostPhase = 0;
	int phaseEvent = 0;

	void parseControlsExternal(ViaControls * controls, ViaInputStreams * inputs);

	void (MetaController::*parseControls)(ViaControls * controls, ViaInputStreams * inputs);

	void parseControlsAudio(ViaControls * controls, ViaInputStreams * inputs);
	void parseControlsDrum(ViaControls * controls, ViaInputStreams * inputs);
	void parseControlsEnv(ViaControls * controls, ViaInputStreams * inputs);
	void parseControlsSeq(ViaControls * controls, ViaInputStreams * inputs);

	void generateIncrementsExternal(ViaInputStreams * inputs);

	void (MetaController::*generateIncrements)(ViaInputStreams * inputs);

	void generateIncrementsAudio(ViaInputStreams * inputs);
	void generateIncrementsDrum(ViaInputStreams * inputs);
	void generateIncrementsEnv(ViaInputStreams * inputs);
	void generateIncrementsSeq(ViaInputStreams * inputs);

	int advancePhase(uint32_t * phaseDistTable);

	int (MetaController::*incrementArbiter)(void);

	int noRetrigAttackState(void);
	int noRetrigReleaseState(void);

	int hardSyncAttackState(void);
	int hardSyncReleaseState(void);

	int envAttackState(void);
	int envReleaseState(void);
	int envRetriggerState(void);

	int gateAttackState(void);
	int gateReleaseReverseState(void);
	int gatedState(void);
	int gateReleaseState(void);
	int gateRetriggerState(void);

	int pendulumRestingState(void);
	int pendulumForwardAttackState(void);
	int pendulumForwardReleaseState(void);
	int pendulumReverseAttackState(void);
	int pendulumReverseReleaseState(void);

	void (MetaController::*loopHandler)(void);

	void handleLoopOff(void);
	void handleLoopOn(void);

};


// just the envelope

class SimpleEnvelope {

	int previousPhase;

public:

	uint32_t attack = 0;
	uint32_t release = 0;
	uint32_t increment = 0;
	uint32_t morph = 0;
	uint32_t phase = 0;
	int phaseEvent = 0;
	uint32_t trigger;

	uint32_t * output;

	void parseControls (ViaControls * controls, ViaInputStreams * inputs);
	void advance (ViaInputStreams * inputs, uint32_t * wavetable);

	int (SimpleEnvelope::*incrementArbiter)(void);

	int attackState(void);
	int releaseState(void);
	int retriggerState(void);
	int restingState(void);

};

#endif /* INC_OSCILLATORS_HPP_ */
