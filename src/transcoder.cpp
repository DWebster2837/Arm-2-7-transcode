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
