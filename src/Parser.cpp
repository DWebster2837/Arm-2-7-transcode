// CMakeProject1.cpp : Defines the entry point for the application.
//

#include "Parser.h"

#define bitcheck(data, pos) (data & (1 << pos))
#define getbits(data, start, end) (data & ((1 << start) + (1 << end))) >> end


using namespace std;

int parse(uint32_t instruction)
{

	//uint32_t instruction = 0xFC000000;
	//Addressing mode 5, option 2; a STR that does nothing, stores 0 sequential words. 

	//Capture NV conditioned instructions
	if ((instruction & 0xF0000000) == 0xF0000000) {
		//replace with NOP
	}

	int op = getbits(instruction, 27, 21);
	//checking against first three bits
	//using < and multiples of 16 filters out the opcode.
	if (op < 16) {
		//Data processing shift
		//Also has some misc instructions (!)
		// bit 25 separates the two tables
		// in table 1 (bit 25 = 1):
		//bit 6 OR 5 denotes a load/store; both 0 means an accumulate multiply or byte swap
		//in table 2:
		//bits 7-4 make an opcode of sorts
	}
	else if (op < 32) {
		//Data processing immediate
	}
	else if (op < 48) {
		//Load/store immediate offset
	}
	else if (op < 64) {
		//Load/store register offset
	}
	else if (op < 80) {
		//Load/store multiple
		//Load/store array of registers - needs its own function for where it does things
	}
	else if (op < 96) {
		//Branch
		//bits 24-0 are the offset
		//bit 25 flags whether it's with link
	}
	else {
		//Everything else - I can safely ignore coprocessor instructions, for example. 
	}

	return 0;
}


//Immed_12 bits 11-0
uint32_t getMode2Immediate(uint32_t instruction) {
	return instruction & 0x00000FFF;
}

//Works for both register offset and scaled register offset
uint32_t getMode2Shifted(uint32_t instruction) {

}