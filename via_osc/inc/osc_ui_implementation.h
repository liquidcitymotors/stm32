#ifndef VIA_UI_IMPLEMENTATION_H
#define VIA_UI_IMPLEMENTATION_H

#include "user_interface.h"

void handleButton1ModeChange(int);
void handleButton2ModeChange(int);
void handleButton3ModeChange(int);
void handleButton4ModeChange(int);
void handleButon5ModeChange(int);
void handleButon6ModeChange(int);

void handleButton1Tap(void);
void handleButton2Tap(void);
void handleButton3Tap(void);
void handleButton4Tap(void);
void handleButton5Tap(void);
void handleButton6Tap(void);
void handleAux1Tap(void);
void handleAux2Tap(void);
void handleAux3Tap(void);
void handleAux4Tap(void);

void handleButton1Hold(void);
void handleButton2Hold(void);
void handleButton3Hold(void);
void handleButton4Hold(void);
void handleButton5Hold(void);
void handleButton6Hold(void);
void handleAux1Hold(void);
void handleAux2Hold(void);
void handleAux3Hold(void);
void handleAux4Hold(void);

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

#endif