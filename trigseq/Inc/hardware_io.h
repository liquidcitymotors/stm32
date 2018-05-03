#include "stm32f3xx_hal.h"
#include "stm32f3xx.h"
#include "stm32f3xx_it.h"

void (*manageADac)(int);
void (*manageBDac)(int);

void dacAHigh(int);
void dacALow(int);

void dacARise(int);
void dacAFall(int);

void dacBHigh(int);
void dacBLow(int);

void dacBRise(int);
void dacBFall(int);

void resampleA(void);
void resampleB(void);

void handleAHigh(void);
void handleALow(void);

void handleBHigh(void);
void handleBLow(void);

