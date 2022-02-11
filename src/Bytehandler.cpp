//TODO: come up with a better damn name for this class, dammit
//This class handles segments of a program and endianness. 
#include "Bytehandler.h"


//TODO: Verify that this code is unused before removing.
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

//a class to read words with varying endianness fast. 
//create a function pointer, set the pointer to the correct function depending on a boolean. 
class wordReader {
public:
	wordReader(char* buffer, bool endianness = false) {
		_head = &buffer[0];
		funcPtr = &wordReader::read;
		if (endianness) {
			funcPtr = &wordReader::readEndSwap;
		}
	}

	inline uint32_t readWord() {
		uint32_t temp = funcPtr(_head);
		_head += 4 * sizeof(char);
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
