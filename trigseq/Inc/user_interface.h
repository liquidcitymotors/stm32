

void (*State)(int);

// dispatch a signal to current state
void uiDispatch(int);  // dispatch signal to state

uint32_t eepromStatus;

#define BUTTON1SENSOR MyTKeys[4].p_Data->StateId
#define BUTTON2SENSOR MyTKeys[5].p_Data->StateId
#define BUTTON3SENSOR MyTKeys[3].p_Data->StateId
#define BUTTON4SENSOR MyTKeys[2].p_Data->StateId
#define BUTTON5SENSOR MyTKeys[0].p_Data->StateId
#define BUTTON6SENSOR MyTKeys[1].p_Data->StateId

#define PRESSED TSL_STATEID_DETECT
#define RELEASED TSL_STATEID_RELEASE

// how modes are arranged by size and location in modeStateBuffer (formatted for EEPROM storage).
#define SH_A_MASK 		0b00000000000000000000000000000000

#define SH_B_MASK 		0b00000000000000000000000011110000
#define SH_B_SHIFT		4

#define AND_A_MASK 		0b00000000000000000000111100000000
#define AND_A_SHIFT 	8

#define AND_B_MASK 		0b00000000000000001111000000000000
#define AND_B_SHIFT 	12

#define BANK_MASK	 	0b00000000000011110000000000000000
#define BANK_SHIFT	 	16



#define DEFAULTPRESET1 0b0000000000000000
#define DEFAULTPRESET2 0b0000000000000000
#define DEFAULTPRESET3 0b0000000000000000
#define DEFAULTPRESET4 0b0000000000000000
#define DEFAULTPRESET5 0b0000000000000000
#define DEFAULTPRESET6 0b0000000000000000
