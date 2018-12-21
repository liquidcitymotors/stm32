#include "scanner.hpp"

void ViaScanner::mainRisingEdgeCallback(void) {

	if (scanner.syncMode) {
		inputs.trigInput = 0;
	} else {
		reverseSignal *= -1;
	}

}

void ViaScanner::mainFallingEdgeCallback(void) {

}

void ViaScanner::auxRisingEdgeCallback(void) {
	setSH(1, 1);
	setLEDA(1);
	inputs.auxTrigInput = 1;
}
void ViaScanner::auxFallingEdgeCallback(void) {
	setSH(0, 0);
	setLEDA(0);
	inputs.auxTrigInput = 0;
}

void ViaScanner::buttonPressedCallback(void) {
	;
}
void ViaScanner::buttonReleasedCallback(void) {
	;
}

void ViaScanner::ioProcessCallback(void) {

//	setLogicOut(readIndex, runtimeDisplay);
//	inputs.trigSamples[readIndex] = inputs.trigInput;
//	inputs.trigInput = 1;
//	reverseBuffer[readIndex] = reverseSignal;
//	inputs.auxTrigSamples[readIndex] = inputs.auxTrigInput;
//
//	readIndex++;
//
//	readIndex &= SCANNER_BUFFER_SIZE*2 - 1;

}

void ViaScanner::halfTransferCallback(void) {

	setLogicOutNoLED(0);

	if (runtimeDisplay) {

		#ifdef BUILD_F373

		*ledCOutput |= (__ROR(outputs.logicA[0], 16) >> 11);

		#endif
		#ifdef BUILD_VIRTUAL

		ledCOutput = (uint32_t) scanner.deltaBlend;

		#endif

	}

	scanner.hardSync = inputs.trigInput;
	inputs.trigInput = 1;
	scanner.reverse = reverseSignal;

	scanner.xInput = (int32_t)-inputs.cv2Samples[0];

	scanner.yInput = (int32_t)-inputs.cv3Samples[0];

	scanner.fillBuffer();

	int32_t sample;

	for (int32_t i = 0; i < SCANNER_BUFFER_SIZE; i++) {
		sample = scanner.altitude[i];
		outputs.dac2Samples[i] = sample;
		outputs.dac1Samples[i] = 4095 - sample;
		outputs.dac3Samples[i] = scanner.locationBlend[i];
	}

	outputs.logicA[0] = GET_ALOGIC_MASK(scanner.hemisphereBlend);
	outputs.auxLogic[0] = GET_EXPAND_LOGIC_MASK(scanner.deltaBlend);

}

void ViaScanner::transferCompleteCallback(void) {

	setLogicOutNoLED(0);

	if (runtimeDisplay) {

		#ifdef BUILD_F373

		*ledCOutput |= (__ROR(outputs.logicA[0], 16) >> 11);

		#endif
		#ifdef BUILD_VIRTUAL

		ledCOutput = (uint32_t) scanner.deltaBlend;

		#endif

	}

	scanner.hardSync = inputs.trigInput;
	inputs.trigInput = 1;
	scanner.reverse = reverseSignal;

	scanner.xInput = (int32_t)-inputs.cv2Samples[0];

	scanner.yInput = (int32_t)-inputs.cv3Samples[0];

	scanner.fillBuffer();

	int32_t sample;

	for (int32_t i = 0; i < SCANNER_BUFFER_SIZE; i++) {
		sample = scanner.altitude[i];
		outputs.dac2Samples[i + SCANNER_BUFFER_SIZE] = sample;
		outputs.dac1Samples[i + SCANNER_BUFFER_SIZE] = 4095 - sample;
		outputs.dac3Samples[i + SCANNER_BUFFER_SIZE] = scanner.locationBlend[i];
	}

	outputs.logicA[0] = GET_ALOGIC_MASK(scanner.hemisphereBlend);
	outputs.auxLogic[0] = GET_EXPAND_LOGIC_MASK(scanner.deltaBlend);

}


void ViaScanner::slowConversionCallback(void) {

	controls.update();
	scanner.parseControls(&controls);

	uint32_t redLevel = abs(scanner.xInput) >> 4;
	uint32_t greenLevel = scanner.zIndex >> 7;
	uint32_t blueLevel = abs(scanner.yInput) >> 4;

	updateRGBDisplay(redLevel, greenLevel, blueLevel, runtimeDisplay);

}

