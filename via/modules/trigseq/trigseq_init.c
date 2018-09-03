#include "via_platform_binding.h"
#include "tsl_user.h"
#include "eeprom.h"
#include "user_interface.h"

#include "trigseq.h"

// eeprom storage array
extern uint16_t VirtAddVarTab[NB_OF_VAR] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
		0x8, 0x9, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16 };

void trigseq_init(trigseq_signal_set * signals) {

	signals->controls = &controlRateInput;
	signals->inputs = &audioRateInput;
	signals->outputs = &audioRateOutput;
	signals->parameters = &trigseqParameters;

	via_ioStreamInit(&audioRateInput, &audioRateOutput, TRIGSEQ_BUFFER_SIZE);
	via_logicStreamInit(&audioRateInput, &audioRateOutput, TRIGSEQ_BUFFER_SIZE);


	trigseq_initializeUICallbacks();

	// initialize our touch sensors
	tsl_user_Init();
	uiInitialize();

	trigseq_initializePatterns();

	manageOutputA = outputARise;
	manageOutputB = outputBRise;

	signals->parameters->currentABank = trigseq_patternBank[0];
	signals->parameters->currentBBank = trigseq_patternBank[0];

//	signals->parameters->outputAEvent = SOFT_GATE_HIGH;
//	signals->parameters->outputBEvent = SOFT_GATE_HIGH;

}

