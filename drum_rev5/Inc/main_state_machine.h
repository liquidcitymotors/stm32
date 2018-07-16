#ifndef MAIN_STATE_MACHINE
#define MAIN_STATE_MACHINE


#include "stm32f3xx_hal.h"
#include "stm32f3xx.h"
#include "stm32f3xx_it.h"
#include "dsp.h"


/**
 *
 * Main oscillator state machine
 *
 */

void (*main_State)(void);

/**
 *
 * States
 *
 *  the state machine is scarcely more than a function pointer
 *  signals have not needed to be implemented, but the function calls in an int
 *  a DSP buffer overrun switches the state to fillBuffer
 *  the completion of the fillBuffer function initiates a transition to the handleUI state
 *
 *
 */

// get next sample(s) then switch to ui handling state
void main_fillBuffer(void);

// execute the touch sensor state machine and expander button
void main_handleUI(void);

/**
 *
 * Main state machine helper functions
 *
 */

// wrapper for implementing the UI
void implementUI(void);

// pointers and functions to show the mode
// these are probably out of place here
// TODO show sync mode
// TODO show (and implement) morph mode
void (*displaySHMode)(void);
void (*displaySyncMode)(void);
void (*displayXCVMode)(void);
void (*displayMorphMode)(void);

void displayXCV_FM(void);
void displayXCV_PM(void);
void displayMorph_Morph(void);
void displayMorph_PMW(void);
void displaySH_On(void);
void displaySH_Off(void);
void displaySync_Hard(void);
void displaySync_Soft(void);

void (*updateRGB)(controlRateInputs *, audioRateInputs *, int, int);
void updateRGBAudio(controlRateInputs *, audioRateInputs *, int, int);
void updateRGBSubAudio(controlRateInputs *, audioRateInputs *, int, int);
void updateRGBTrigger(controlRateInputs *, audioRateInputs *, int, int);
void updateRGBBlank(controlRateInputs *, audioRateInputs *, int, int);

// Flag word bit packing macros (stale from a different fimware)

uint32_t flagHolder;

#define OSCILLATOR_ACTIVE 	flagHolder & 0b00000000000000000000000000000001
#define LAST_PHASE_STATE 	flagHolder & 0b00000000000000000000000000000010
#define GATE	 			flagHolder & 0b00000000000000000000000000000100
#define RUNTIME_DISPLAY 	flagHolder & 0b00000000000000000000000000001000
#define UPDATE_PRESCALER 	flagHolder & 0b00000000000000000000000000010000
#define DRUM_MODE_FLAG	 	flagHolder & 0b00000000000000000000000000100000
#define DRUM_ATTACK 		flagHolder & 0b00000000000000000000000001000000
#define DRUM_RELEASE	 	flagHolder & 0b00000000000000000000000010000000
#define PITCH_MOD 			flagHolder & 0b00000000000000000000000100000000
#define MORPH_MOD 			flagHolder & 0b00000000000000000000001000000000
#define AMP_MOD 			flagHolder & 0b00000000000000000000010000000000
#define DRUM_OFF 			flagHolder & 0b00000000000000000000100000000000
#define LAST_CYCLE 			flagHolder & 0b00000000000000000001000000000000
#define HOLD_AT_B 			flagHolder & 0b00000000000000000010000000000000
#define PHASE_STATE 		flagHolder & 0b00000000000000000100000000000000
#define TRIGGER_BUTTON	 	flagHolder & 0b00000000000000001000000000000000
#define DETECT_ON	 		flagHolder & 0b00000000000000010000000000000000
#define AUX_MENU	 		flagHolder & 0b00000000000000100000000000000000
#define GATEA		 		flagHolder & 0b00000000000001000000000000000000
#define TRIGA		 		flagHolder & 0b00000000000010000000000000000000
#define DELTAA		 		flagHolder & 0b00000000000100000000000000000000
#define GATEB		 		flagHolder & 0b00000000001000000000000000000000
#define TRIGB		 		flagHolder & 0b00000000010000000000000000000000
#define DELTAB		 		flagHolder & 0b00000000100000000000000000000000
#define BANDLIMIT		 	flagHolder & 0b00000001000000000000000000000000
#define DRUM_SAFETY		 	flagHolder & 0b00000010000000000000000000000000


#define SET_OSCILLATOR_ACTIVE	flagHolder |= 0b00000000000000000000000000000001
#define SET_LAST_PHASE_STATE 	flagHolder |= 0b00000000000000000000000000000010
#define SET_GATE	 			flagHolder |= 0b00000000000000000000000000000100
#define SET_RUNTIME_DISPLAY 	flagHolder |= 0b00000000000000000000000000001000
#define SET_UPDATE_PRESCALER 	flagHolder |= 0b00000000000000000000000000010000
#define SET_DRUM_MODE	 		flagHolder |= 0b00000000000000000000000000100000
#define SET_DRUM_ATTACK 		flagHolder |= 0b00000000000000000000000001000000
#define SET_DRUM_RELEASE	 	flagHolder |= 0b00000000000000000000000010000000
#define SET_PITCH_MOD 			flagHolder |= 0b00000000000000000000000100000000
#define SET_MORPH_MOD 			flagHolder |= 0b00000000000000000000001000000000
#define SET_AMP_MOD 			flagHolder |= 0b00000000000000000000010000000000
#define SET_DRUM_OFF 			flagHolder |= 0b00000000000000000000100000000000
#define SET_LAST_CYCLE 			flagHolder |= 0b00000000000000000001000000000000
#define SET_HOLD_AT_B 			flagHolder |= 0b00000000000000000010000000000000
#define SET_PHASE_STATE 		flagHolder |= 0b00000000000000000100000000000000
#define SET_TRIGGER_BUTTON	 	flagHolder |= 0b00000000000000001000000000000000
#define SET_DETECT_ON	 		flagHolder |= 0b00000000000000010000000000000000
#define SET_AUX_MENU			flagHolder |= 0b00000000000000100000000000000000
#define SET_GATEA		 		flagHolder |= 0b00000000000001000000000000000000
#define SET_TRIGA		 		flagHolder |= 0b00000000000010000000000000000000
#define SET_DELTAA		 		flagHolder |= 0b00000000000100000000000000000000
#define SET_GATEB		 		flagHolder |= 0b00000000001000000000000000000000
#define SET_TRIGB		 		flagHolder |= 0b00000000010000000000000000000000
#define SET_DELTAB		 		flagHolder |= 0b00000000100000000000000000000000
#define SET_BANDLIMIT		 	flagHolder |= 0b00000001000000000000000000000000
#define SET_DRUM_SAFETY		 	flagHolder |= 0b00000010000000000000000000000000


#define CLEAR_OSCILLATOR_ACTIVE	flagHolder &= 0b11111111111111111111111111111110
#define CLEAR_LAST_PHASE_STATE 	flagHolder &= 0b11111111111111111111111111111101
#define CLEAR_GATE	 			flagHolder &= 0b11111111111111111111111111111011
#define CLEAR_RUNTIME_DISPLAY 	flagHolder &= 0b11111111111111111111111111110111
#define CLEAR_UPDATE_PRESCALER 	flagHolder &= 0b11111111111111111111111111101111
#define CLEAR_DRUM_MODE 		flagHolder &= 0b11111111111111111111111111011111
#define CLEAR_DRUM_ATTACK	 	flagHolder &= 0b11111111111111111111111110111111
#define CLEAR_DRUM_RELEASE 		flagHolder &= 0b11111111111111111111111101111111
#define CLEAR_PITCH_MOD 		flagHolder &= 0b11111111111111111111111011111111
#define CLEAR_MORPH_MOD 		flagHolder &= 0b11111111111111111111110111111111
#define CLEAR_AMP_MOD 			flagHolder &= 0b11111111111111111111101111111111
#define RESET_DRUM_OFF 			flagHolder &= 0b11111111111111111111011111111111
#define CLEAR_LAST_CYCLE 		flagHolder &= 0b11111111111111111110111111111111
#define CLEAR_HOLD_AT_B 		flagHolder &= 0b11111111111111111101111111111111
#define CLEAR_PHASE_STATE		 flagHolder &= 0b11111111111111111011111111111111
#define CLEAR_TRIGGER_BUTTON	flagHolder &= 0b11111111111111110111111111111111
#define CLEAR_DETECT_ON			flagHolder &= 0b11111111111111101111111111111111
#define CLEAR_AUX_MENU			flagHolder &= 0b11111111111111011111111111111111
#define CLEAR_GATEA				flagHolder &= 0b11111111111110111111111111111111
#define CLEAR_TRIGA				flagHolder &= 0b11111111111101111111111111111111
#define CLEAR_DELTAA			flagHolder &= 0b11111111111011111111111111111111
#define CLEAR_GATEB				flagHolder &= 0b11111111110111111111111111111111
#define CLEAR_TRIGB				flagHolder &= 0b11111111101111111111111111111111
#define CLEAR_DELTAB			flagHolder &= 0b11111111011111111111111111111111
#define CLEAR_BANDLIMIT			flagHolder &= 0b11111110111111111111111111111111
#define CLEAR_DRUM_SAFETY		flagHolder &= 0b11111101111111111111111111111111



#endif



