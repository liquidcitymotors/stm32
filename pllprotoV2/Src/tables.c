#include "tables.h"

#include "main.h"

#include "stm32f3xx_hal.h"
#include "stm32f3xx.h"
#include "stm32f3xx_it.h"

// helper function to load the selected family to ram
void loadSampleArray(Family);

// we store our family stuct pointers here
Family familyArray[16];

// helpful variable we can use for the currently selected family struct
Family currentFamily;


// these variables are used to represent the number of entries in a given wavetable stored in the currently selected family
// they are shared by tim6IRQ
extern uint32_t span;
extern int spanx2;
extern int tableSizeCompensation;

// these variables are used to represent the number of wavetables in the currently selected family when performing our morph function
extern uint32_t morphBitShiftRight;
extern uint32_t morphBitShiftLeft;

// fill our family structs with the arrays declared in the header
// length and size are declared here in accordance with the chosen arrays

Family moogShifted =
	{.attackFamily = moogShiftedAttackFamily,
	.releaseFamily = moogShiftedReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family moogNormalized =
	{.attackFamily = moogNormalizedAttackFamily,
	.releaseFamily = moogNormalizedReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family moogSquare =
	{.attackFamily = moogSquareShiftAttackFamily,
	.releaseFamily = moogSquareShiftReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family moogInverted =
	{.attackFamily = moogInvertedAttackFamily,
	.releaseFamily = moogInvertedReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family perlin =
	{.attackFamily = perlinAttackFamily,
	.releaseFamily = perlinReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family sineFold =
	{.attackFamily = sinefoldAttackFamily,
	.releaseFamily = sinefoldReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family bounce =
	{.attackFamily = bounceAttackFamily,
	.releaseFamily = bounceReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family triodd =
	{.attackFamily = trioddAttackFamily,
	.releaseFamily = trioddReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family triOdd =
	{.attackFamily = trioddAttackFamily,
	.releaseFamily = trioddReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family triFudge =
	{.attackFamily = trifudgeAttackFamily,
	.releaseFamily = trifudgeReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family moog1 =
	{.attackFamily = moog1AttackFamily,
	.releaseFamily = moog1ReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family moog2 =
	{.attackFamily = moog2AttackFamily,
	.releaseFamily = moog2ReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family sawBend =
	{.attackFamily = sawBendAttackFamily,
	.releaseFamily = sawBendReleaseFamily,
	.tableLength = 4,
	.familySize = 5};

Family sawBendLinAtk =
	{.attackFamily = allLinear5_5,
	.releaseFamily = sawBendReleaseFamily,
	.tableLength = 4,
	.familySize = 5};

Family exciteBike =
	{.attackFamily = exciteBikeAttackFamily,
	.releaseFamily = exciteBikeReleaseFamily,
	.tableLength = 8,
	.familySize = 9};

Family exciteBikeLinAtk =
	{.attackFamily = allLinear9_9,
	.releaseFamily = exciteBikeReleaseFamily,
	.tableLength = 8,
	.familySize = 9};

Family rand =
	{.attackFamily = randAttackFamily,
	.releaseFamily = randReleaseFamily,
	.tableLength = 8,
	.familySize = 33};

Family gauss =
	{.attackFamily = gaussAttackFamily,
	.releaseFamily = gaussReleaseFamily,
	.tableLength = 8,
	.familySize = 33};

Family gauss_noconform =
	{.attackFamily = gauss_noconformAttackFamily,
	.releaseFamily = gauss_noconformReleaseFamily,
	.tableLength = 8,
	.familySize = 33};

Family gauss_low =
	{.attackFamily = gauss_lowAttackFamily,
	.releaseFamily = gauss_lowReleaseFamily,
	.tableLength = 8,
	.familySize = 33};

Family gauss_low_noconform =
	{.attackFamily = gauss_low_noconformAttackFamily,
	.releaseFamily = gauss_low_noconformReleaseFamily,
	.tableLength = 8,
	.familySize = 33};

Family algerian =
	{.attackFamily = algerianAttackFamily,
	.releaseFamily = algerianReleaseFamily,
	.tableLength = 64,
	.familySize = 5};

Family quartSym =
	{.attackFamily = quartSymAttackFamily,
	.releaseFamily = quartSymReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family quartAsym =
	{.attackFamily = quartAsymAttackFamily,
	.releaseFamily = quartAsymReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family superEllipse1Sym =
	{.attackFamily = superEllipse1SymAttackFamily,
	.releaseFamily = superEllipse1SymReleaseFamily,
	.tableLength = 64,
	.familySize = 5};

Family superEllipse1Asym =
	{.attackFamily = superEllipse1AsymAttackFamily,
	.releaseFamily = superEllipse1AsymReleaseFamily,
	.tableLength = 64,
	.familySize = 5};

Family gammaSym =
	{.attackFamily = gammaSymAttackFamily,
	.releaseFamily = gammaSymReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family gammaAsym =
	{.attackFamily = gammaAsymAttackFamily,
	.releaseFamily = gammaAsymReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family sharpExpoSym =
	{.attackFamily = sharpExpoSymAttackFamily,
	.releaseFamily = sharpExpoSymReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family sharpExpoAsym =
	{.attackFamily = sharpExpoAsymAttackFamily,
	.releaseFamily = sharpExpoAsymReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family sharpLinSym =
	{.attackFamily = sharpLinSymAttackFamily,
	.releaseFamily = sharpLinSymReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family sharpLinAsym =
	{.attackFamily = sharpLinAsymAttackFamily,
	.releaseFamily = sharpLinAsymReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family ascendingAdditiveClamp =
	{.attackFamily = ascendingAdditiveClampAttackFamily,
	.releaseFamily = ascendingAdditiveClampReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

/*
Family summingAdditiveClamp =
	{.attackFamily = summingAdditiveClampAttackFamily,
	.releaseFamily = summingAdditiveClampReleaseFamily,
	.tableLength = 64,
	.familySize = 9};
*/

Family moogImpossibleTri =
	{.attackFamily = moogImpossibleTriAttackFamily,
	.releaseFamily = moogImpossibleTriReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

Family steps =
	{.attackFamily = stepsAttackFamily,
	.releaseFamily = stepsReleaseFamily,
	.tableLength = 64,
	.familySize = 9};

/*

Family perlin130_1 =
	{.attackFamily = perlin130_1_noskewAttackFamily,
	.releaseFamily = perlin130_1_noskewReleaseFamily,
	.tableLength = 128,
	.familySize = 9};

Family perlin130_2 =
	{.attackFamily = perlin130_2_noskewAttackFamily,
	.releaseFamily = perlin130_2_noskewReleaseFamily,
	.tableLength = 128,
	.familySize = 9};

Family perlin130_3 =
	{.attackFamily = perlin130_3_noskewAttackFamily,
	.releaseFamily = perlin130_3_noskewReleaseFamily,
	.tableLength = 128,
	.familySize = 9};

Family perlin130_4 =
	{.attackFamily = perlin130_4_noskewAttackFamily,
	.releaseFamily = perlin130_4_noskewReleaseFamily,
	.tableLength = 128,
	.familySize = 9};

Family perlin130_5 =
	{.attackFamily = perlin130_5_noskewAttackFamily,
	.releaseFamily = perlin130_5_noskewReleaseFamily,
	.tableLength = 128,
	.familySize = 9};
*/

Family impBig =
	{.attackFamily = imp,
	.releaseFamily = imp,
	.tableLength = 128,
	.familySize = 33};

Family impevens =
	{.attackFamily = impshort,
	.releaseFamily = impshort,
	.tableLength = 128,
	.familySize = 17};


Family tenor48 =
	{.attackFamily = tenor48AttackFamily,
	.releaseFamily = tenor48ReleaseFamily,
	.tableLength = 128,
	.familySize = 5};

/*

Family tenor16 =
	{.attackFamily = tenor16AttackFamily,
	.releaseFamily = tenor16ReleaseFamily,
	.tableLength = 128,
	.familySize = 5};

Family tenor12 =
	{.attackFamily = tenor12AttackFamily,
	.releaseFamily = tenor12ReleaseFamily,
	.tableLength = 128,
	.familySize = 5};
*/

Family soprano48 =
	{.attackFamily = soprano48AttackFamily,
	.releaseFamily = soprano48ReleaseFamily,
	.tableLength = 128,
	.familySize = 5};
/*

Family soprano16 =
	{.attackFamily = soprano16AttackFamily,
	.releaseFamily = soprano16ReleaseFamily,
	.tableLength = 128,
	.familySize = 5};

Family soprano12 =
	{.attackFamily = soprano12AttackFamily,
	.releaseFamily = soprano12ReleaseFamily,
	.tableLength = 128,
	.familySize = 5};
*/

Family artificial_1 =
	{.attackFamily = artificial_1_48AttackFamily,
	.releaseFamily = artificial_1_48ReleaseFamily,
	.tableLength = 128,
	.familySize = 5};

Family filterbank_24 =
	{.attackFamily = fbank_24,
	.releaseFamily = fbank_24,
	.tableLength = 128,
	.familySize = 9};

Family filterbank_48 =
	{.attackFamily = fbank_48,
	.releaseFamily = fbank_48,
	.tableLength = 128,
	.familySize = 9};


//Family filterbankTight_24 =
//	{.attackFamily = fbankTight_24,
//	.releaseFamily = fbankTight_24,
//	.tableLength = 128,
//	.familySize = 9};
//
//Family filterbankTight_48 =
//	{.attackFamily = fbankTight_48,
//	.releaseFamily = fbankTight_48,
//	.tableLength = 128,
//	.familySize = 9};


Family skipSaw =
	{.attackFamily = skipsaw,
	.releaseFamily = skipsaw,
	.tableLength = 64,
	.familySize = 5};

Family hopSaw =
	{.attackFamily = hopsaw,
	.releaseFamily = hopsaw,
	.tableLength = 64,
	.familySize = 5};

Family assortedEnvs =
	{.attackFamily = assorted,
	.releaseFamily = assorted,
	.tableLength = 64,
	.familySize = 5};

Family lump3rdDegLinAtk =
	{.attackFamily = allLinear129_5,
	.releaseFamily = lump3rdDeg,
	.tableLength = 128,
	.familySize = 5};

Family lump2ndDegLinAtk =
	{.attackFamily = allLinear129_5,
	.releaseFamily = lump2ndDeg,
	.tableLength = 128,
	.familySize = 5};
Family doubleLump3rdDegLinAtk =
	{.attackFamily = allLinear129_5,
	.releaseFamily = lump3rdDeg,
	.tableLength = 128,
	.familySize = 5};

Family doubleLump2ndDegLinAtk =
	{.attackFamily = allLinear129_5,
	.releaseFamily = lump2ndDeg,
	.tableLength = 128,
	.familySize = 5};

Family threeBounceLinAtk =
	{.attackFamily = allLinear65_3,
	.releaseFamily = threeBounce,
	.tableLength = 64,
	.familySize = 3};

Family threeSineFoldsLinAtk =
	{.attackFamily = allLinear65_3,
	.releaseFamily = threeSineFolds,
	.tableLength = 64,
	.familySize = 3};



// specify the family in our family bank per speed

void fillFamilyArray(void) {

	familyArray[0] = tenor48;
	familyArray[1] = artificial_1;
	familyArray[2] = impevens;
	familyArray[3] = ascendingAdditiveClamp;
	familyArray[4] = skipSaw;
	familyArray[5] = triOdd;
	familyArray[6] = moogImpossibleTri;
	familyArray[7] = moogSquare;

	familyArray[8] = superEllipse1Sym;
	familyArray[9] = superEllipse1Asym;
	familyArray[10] = doubleLump2ndDegLinAtk;
	familyArray[11] = lump2ndDegLinAtk;
	familyArray[12] = steps;
	familyArray[13] = perlin;
	familyArray[14] = threeSineFoldsLinAtk;
	familyArray[15] = threeBounceLinAtk;


	currentFamily = familyArray[0];
	switchFamily();

}

// this sets the flags to be used in the interrupt and also fills the holding array on the heap

void switchFamily(void) {

	holdState |= familyIndicator << 9;

	currentFamily = familyArray[familyIndicator];
	loadSampleArray(currentFamily);

	span = (currentFamily.tableLength) << 16;
	spanx2 = (currentFamily.tableLength) << 17;
	switch (currentFamily.familySize) {
	// these are values that properly allow us to select a family and interpolation fraction for our morph
	case 3:
		morphBitShiftRight = 11;
		morphBitShiftLeft = 5;
		break;

	case 5:
		morphBitShiftRight = 10;
		morphBitShiftLeft = 6;
		break;

	case 9:
		morphBitShiftRight = 9;
		morphBitShiftLeft = 7;
		break;

	case 17:
		morphBitShiftRight = 8;
		morphBitShiftLeft = 8;
		break;

	case 33:
		morphBitShiftRight = 7;
		morphBitShiftLeft = 9;
		break;

	}
	switch (currentFamily.tableLength) {
	// these are values that properly allow us to select a family and interpolation fraction for our morph
	case 4:
		tableSizeCompensation = 5;
		break;

	case 8:
		tableSizeCompensation = 4;
		break;

	case 16:
		tableSizeCompensation = 3;
		break;

	case 32:
		tableSizeCompensation = 2;
		break;

	case 64:
		tableSizeCompensation = 1;
		break;

	case 128:
		tableSizeCompensation = 0;

	}
}

//this actually shuttles the data from flash to ram and fills our holding array

void loadSampleArray(Family family) {

	uint16_t **currentFamilyPointer;

	for (int i = 0; i < family.familySize; i++) {
		for (int j = 0; j <= family.tableLength; j++) {
			// this just gets the appropriate samples and plops them into the global holding arrays
			currentFamilyPointer = family.attackFamily + i;
			attackHoldArray[i][j] = *(*(currentFamilyPointer) + j);

			currentFamilyPointer = family.releaseFamily + i;
			releaseHoldArray[i][j] = *(*(currentFamilyPointer) + j);
		}
	}
}
