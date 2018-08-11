
#include "scales.h"
#include "main.h"
#include "stm32f3xx_hal.h"
#include "stm32f3xx.h"
#include "stm32f3xx_it.h"



Scale integerRatios = {
   .grid = integerRatiosGrid,
   .t2Bitshift = 9,
   .oneVoltOct = 0};

Scale pentamodal5primefullspan = {
   .grid = pentamodal5primefullspanGrid,
   .t2Bitshift = 9,
   .oneVoltOct = 0};

Scale harmSubharmfullspan = {
   .grid = harmSubharmfullspanGrid,
   .t2Bitshift = 9,
   .oneVoltOct = 0};

Scale modal5primefullspan = {
   .grid = modal5primefullspanGrid,
   .t2Bitshift = 9,
   .oneVoltOct = 0};

Scale harmSubharm = {
   .grid = harmSubharmGrid,
   .t2Bitshift = 9,
   .oneVoltOct = 1};

Scale harmFolded4 = {
   .grid = harmFolded4Grid,
   .t2Bitshift = 10,
   .oneVoltOct = 1};

Scale pentamodal5prime = {
   .grid = pentamodal5primeGrid,
   .t2Bitshift = 9,
   .oneVoltOct = 1};

Scale triads = {
   .grid = triadsGrid,
   .t2Bitshift = 9,
   .oneVoltOct = 1};

Scale seventhTetradsMinimalProgression = {
   .grid = seventhTetradsMinimalProgressionGrid,
   .t2Bitshift = 8,
   .oneVoltOct = 1};

Scale seventhTetrads = {
   .grid = seventhTetradsGrid,
   .t2Bitshift = 8,
   .oneVoltOct = 1};

Scale modesOfLimitedTransposition = {
   .grid = modesOfLimitedTranspositionGrid,
   .t2Bitshift = 8,
   .oneVoltOct = 1};

Scale majMin9thPentStarling = {
   .grid = majMin9thPentStarlingGrid,
   .t2Bitshift = 8,
   .oneVoltOct = 1};

Scale simpleRhythms = {
   .grid = simpleRhythmsGrid,
   .t2Bitshift = 9,
   .oneVoltOct = 0};

Scale simpleRhythms16 = {
   .grid = simpleRhythms16Grid,
   .t2Bitshift = 9,
   .oneVoltOct = 0};

Scale rhythmDivisionsReset = {
   .grid = rhythmDivisionsResetGrid,
   .t2Bitshift = 9,
   .oneVoltOct = 0};

Scale polyResets = {
   .grid = polyResetsGrid,
   .t2Bitshift = 8,
   .oneVoltOct = 0};

void initializeScales() {

   scaleArray[0][0] = &integerRatios;
   scaleArray[0][1] = &pentamodal5primefullspan;
   scaleArray[0][2] = &harmSubharmfullspan;
   scaleArray[0][3] = &modal5primefullspan;

   scaleArray[1][0] = &harmSubharm;
   scaleArray[1][1] = &harmFolded4;
   scaleArray[1][2] = &pentamodal5prime;
   scaleArray[1][3] = &triads;

   scaleArray[2][0] = &seventhTetradsMinimalProgression;
   scaleArray[2][1] = &seventhTetrads;
   scaleArray[2][2] = &modesOfLimitedTransposition;
   scaleArray[2][3] = &majMin9thPentStarling;

   scaleArray[3][0] = &simpleRhythms;
   scaleArray[3][1] = &simpleRhythms16;
   scaleArray[3][2] = &rhythmDivisionsReset;
   scaleArray[3][3] = &polyResets;
}

//void initializeScales() {
//   scaleGroup[0] = pentamodal5prime;
//   scaleGroup[1] = pentamodal5prime;
//   scaleGroup[2] = pentamodal5prime;
//   scaleGroup[3] = pentamodal5prime;
//   scaleGroup[4] = harmSubharm;
//   scaleGroup[5] = harmFolded4;
//   scaleGroup[6] = pentamodal5prime;
//   scaleGroup[7] = triads;
//   scaleGroup[8] = seventhTetradsMinimalProgression;
//   scaleGroup[9] = seventhTetrads;
//   scaleGroup[10] = modesOfLimitedTransposition;
//   scaleGroup[11] = majMin9thPentStarling;
//   scaleGroup[12] = simpleRhythms;
//   scaleGroup[13] = simpleRhythms16;
//   scaleGroup[14] = rhythmDivisionsReset;
//   scaleGroup[15] = polyResets;
//}

