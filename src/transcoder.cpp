// disassembler1.cpp : Defines the entry point for the application.
//

#include "transcoder.h"

using namespace std;


//checks whether bit at position pos is 1
#define bitcheck(data, pos) (data & (1 << pos))
#define getbits(data, start, end) (data & ((1 << start) + (1 << end))) >> end

typedef struct ELFfileheader {
	uint32_t flags;

	//program header table offset, entry size and entry count. Probably unused.
	uint32_t phoff;
	uint16_t phentsize;
	uint16_t phnum;

	//section header table offset, entry size, entry count and section name entry index
	uint32_t shoff;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
};

string condition(uint32_t curinstruction);
string dataprocessing(uint32_t curinstruction);
string datatransfer(uint32_t curinstruction);
string branching(uint32_t curinstruction);
string coprocessor(uint32_t curinstruction);
char* headerdecode(char* buffer);

class byteReader {
public:
	//a flag; denotes whether to swap endianness. Defaults 0.
	static bool endian;

	//grab 1 byte
	static uint8_t grab8(char* head) {
		return (uint8_t)head[0];
	}

	//grab 2 bytes
	static uint16_t grab16(char* head) {
		if (endian) {
			//swapped endianness
			return (uint16_t)head[1] << 8 |
				(uint16_t)head[0];
		}
		else {
			return (uint16_t)head[0] << 8 |
				(uint16_t)head[1];
		}
	}

	//grab a word
	static uint32_t grab32(char* head) {
		if (endian) {
			//swapped endianness
			return (uint32_t)head[3] << 24 |
				(uint32_t)head[2] << 16 |
				(uint32_t)head[1] << 8 |
				(uint32_t)head[0];
		}
		else {
			return 	(uint32_t)head[0] << 24 |
				(uint32_t)head[1] << 16 |
				(uint32_t)head[2] << 8 |
				(uint32_t)head[3];
		}
	}
};

//a class to read words with endianness fast; given that on a 1MB program this will run 250,000 times, it might as well be optimised. 
//on initialise, makes a pointer to correct function; avoids doing conditionals every time. 
class wordReader {
public:
	wordReader(char* head, bool endianness = false) {
		_head = head;
		funcPtr = &wordReader::read;
		if (endianness) {
			funcPtr = &wordReader::readEndSwap;
		}
	}
	inline uint32_t readWord() {
		uint32_t temp = funcPtr(_head);
		_head += 4*sizeof(char);
		return temp;
	}
private:
	char* _head;
	uint32_t(*funcPtr)(char*);

	static uint32_t readEndSwap(char* head) {
		return (uint32_t)head[3] << 24 |
			(uint32_t)head[2] << 16 |
			(uint32_t)head[1] << 8 |
			(uint32_t)head[0];
	}
	static uint32_t read(char* head) {
		return 	(uint32_t)head[0] << 24 |
			(uint32_t)head[1] << 16 |
			(uint32_t)head[2] << 8 |
			(uint32_t)head[3];
	}
};

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Invalid argument count");
		return -1;
	}
	const char* infilename = argv[1];
	ifstream file(infilename, std::ios::binary);

	if(!file){
		printf("file unsuccessfully opened");
		return -1;
	}
	streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	//all ARM instructions are 32 bits; therefore, a file that's not a multiple of 4 bytes is always invalid
	if (!(size % 4)) {
		printf("input not a multiple of 4 bytes");
		return -1;
		
	}
	//read the file into buffer
	vector<char> buffer(size);
	if (!file.read(buffer.data(), size)) {
		printf("file read was bad");
		return -1;
	}

	char* head = &buffer[0];
	list<string> output;
	while (head < &buffer[size]) {
		//ARM instruction structure:
		// 4 bits of conditions {31-28}. They, respectively, represent [N]egative, [Z]ero, [C]arry and [V] (signed overflow). 
		// behaviour of 1111 here depends on ARM version; in early versions, it's effectively a NOP, while later it's not. 
		// 3 bits for op1. 
		// 00x are data processing; 01x are load/store (and media in new versions); 
		// 10x are branching and block data transfer (ie, load multiple); 11x are coprocessor instructions
		// the rest, besides bit 4, depend on op1
		// 

		//smoosh 4 bytes together. Preserves endianness. 
		uint32_t curinstruction =
			(uint32_t)head[0] << 24 |
			(uint32_t)head[1] << 16 |
			(uint32_t)head[2] << 8 |
			(uint32_t)head[3];

		//TODO: depoopify this variable naming. It's bad. 
		string cond = condition(curinstruction);

		int op1 = getbits(curinstruction, 27, 26);

		string instruction;
		switch (op1) {
		case 0:
			instruction = dataprocessing(curinstruction);
			break;
		case 1:
			instruction = datatransfer(curinstruction);
			break;
		case 2:
			instruction = branching(curinstruction);
			break;
		case 3:
			instruction = coprocessor(curinstruction);
			break;
		}
		string a = instruction;

	}
	return 0;
}

string condition(uint32_t curinstruction) {
	const string conditions[16] = { "EQ", "NE", "CS/HS", 
		"CC/LO", "MI", "PL", "VS", "VC", "HI", "LS", "GE", 
		"LT", "GT", "LE", "AL", "(NV)"};
	//mask and shift condition bits; gives value from 0 to 15. 
	int cond = (curinstruction & (15 << 28)) >> 28;
	//return corresponding condition mnemonic
	return conditions[cond];
}

string dataprocessing(uint32_t curinstruction) {
	bool op = bitcheck(curinstruction, 4);
	//<opcode>{<cond>}{S} <Rd>, <Rn>, <shifter_operand>
	// bit 25 is I, distinguishes between immediate and register shifter operands
	//bits 24-21 are opcode
	// if bits 24-23 are '10', S should be omitted
	//bit 20 is S; flags whether to update condition registers
	//bits 19 through 16 are Rd, destination register
	//bits 15 through 12 are Rn, first source register
	//bits 11 through 0 are the shifter operand; there are 3 layouts for this
	const string opcodes[16] = {"AND", "EOR", "SUB", "RSB", "ADD", "ADC", "SBC", 
		"RSC", "TST", "TEQ", "CMP", "CMN", "ORR", "MOV", "BIC", "MVN"};
	//shifts by value shift (bits 6 to 5)
	//LSL is 00; LSR is 01; ASR is 10; ROR is 11
	//RRX replaces ROR #0; <Rm> replaces LSL #0
	const string shifts[5] = { "LSL", "LSR", "ASR", "ROR", "RRX" };
	
	bool I = bitcheck(curinstruction, 25);
	int Rn = getbits(curinstruction, 19, 16);
	int Rd = getbits(curinstruction, 15, 12);
	int opcode = getbits(curinstruction, 24, 21);


	//deal with the bit shifting (ie, LSL #2)
	string shifter_operand;
	//create <shifter_operand>
	if (I) {
		//#<immediate>
		int immed = getbits(curinstruction, 7, 0);
		shifter_operand = format("#%.2x", immed); //#<immediate>
	}
	else {
		//anything starting with <Rm>
		int Rm = getbits(curinstruction, 3, 0);
		int shift = getbits(curinstruction, 6, 5);

		//shift by register and shift by immediate
		if (bitcheck(curinstruction, 4)) {
			int Rs = getbits(curinstruction, 11, 8);
			shifter_operand = format("R%i, %s R%i", Rm, shifts[shift], Rs); //<Rm>, <shift_type> <shift_immediate>
		}
		else {
			int shift_imm = getbits(curinstruction, 11, 7);

			//handle RRX; replaces ROR #0
			if (shift_imm == 0 && shift == 3) {
				shifter_operand = shifts[4]; //RRX
			}
			else if (shift_imm == 0 && shift == 0) {
				shifter_operand = format("R%i", Rm); //<Rm>
			}
			else {
				shifter_operand = format("R%i, %s #%.2x", Rm, shifts[shift], shift_imm); //<Rm>, <shift_type> <shift_immediate>
			}
		}

	}

	//deal with the opcode
	string opcodestr = opcodes[opcode];
	string cond = condition(curinstruction);
	string S = bitcheck(curinstruction, 20) ? "S" : "";
	//MOV, MVN
	//1101 or 1111; opcode & 1101 works. 
	if (opcode & 13) {
		return format("%s%s%s R%i, %s", opcodestr, cond, S, Rd, shifter_operand);
	}
	//CMP, CMN, TST, TEQ; 10xx. 
	else if((opcode & 8) && !(opcode & 4)){
		return format("%s%s R%i, %s", opcodestr, cond, Rn, shifter_operand);
	}
	//the rest
	else {
		return format("%s%s%s R%i, R%i, %s", opcodestr, cond, S, Rd, Rn, shifter_operand);
	}
}

string datatransfer(uint32_t curinstruction) {
	bool op = bitcheck(curinstruction, 4);
	//LDR|STR{<cond>}{B}{T} <Rd>, <addressing_mode>
	// 
	//<addressing mode>: <Rn>, {#}[+/-][<offset_12>|<Rm>{, <shift> #<shift_imm>{!}}]
	//<Rn> always present
	//# denotes imediate offset/index
	// ! denotes pre-index; if not present, post-indexed where relevant

	//by and large, there's 2 modes: immediate offset/index, and register offset/index
	//register has option to scale by a bit shift
 
	bool P = bitcheck(curinstruction, 24);
	bool U = bitcheck(curinstruction, 23);
	bool B = bitcheck(curinstruction, 22);
	bool W = bitcheck(curinstruction, 21);
	bool L = bitcheck(curinstruction, 20);
	//L, bit 20, denotes STR (L == 0) or LDR (L == 1). 
	string opcode = bitcheck(curinstruction, 20) ? "STR" : "LDR";


	if (!bitcheck(curinstruction, 25)) {
		//immediate offset/index

	}
	else {
		//register offset/index

	}
}

//also block data transfer
string branching(uint32_t curinstruction) {
	bool op = bitcheck(curinstruction, 4);

}

string coprocessor(uint32_t curinstruction) {
	bool op = bitcheck(curinstruction, 4);

}
