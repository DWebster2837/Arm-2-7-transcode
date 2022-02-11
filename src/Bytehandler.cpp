//TODO: come up with a better damn name for this class, dammit
//This file handles segments of a program and endianness. 
//Goal is to take the file image and ELF table, and produce an iterator
//that can iterate over all the code parts of the file
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
//NOTE: this isn't used, the code is in CodeSegIterator. It's here in case I want it later. 
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

//this class creates an iterator that iterates over all code segments in the image
//and returns it word by word, handling endianness as it goes. 
class CodeSegIterator {
public:
	CodeSegIterator(char* buffer, std::vector<codeSegment> _segments, bool _endian) {
		img = buffer;
		segments = _segments;

		//set function pointer for reading word with or without swapping endianness. 
		funcPtr = &CodeSegIterator::readWord;
		if (_endian) {
			funcPtr = &CodeSegIterator::readWordEndSwap;
		}

		segIter = segments.begin();

	}

	uint32_t next() {
		//index goes up in multiples of 1 word (4 bytes)
		index+=4;

		if(index > (*segIter).p_filesz){
			//move to next segment if reached end of current one 
			segIter++;
			index = 0;
		}

		int imgIndex = (*segIter).p_offset + index;

		return funcPtr(img + imgIndex);
		//TODO: catch if past last segment
	}

	bool hasNext() {
		//if last segment and index is at end of segment
		return (segIter == segments.end()) && (index >= (*segIter).p_filesz);
	}

private:
	char* img;
	int index;
	std::vector<codeSegment> segments;
	std::vector<codeSegment>::iterator segIter;

	//pointer to one of the readWord functions
	uint32_t(*funcPtr)(char*);

	//reads a word from the address of head
	//two options, one with and one without swapping endianness
	static uint32_t readWordEndSwap(char* head) {
		return (uint32_t)head[3] << 24 |
			(uint32_t)head[2] << 16 |
			(uint32_t)head[1] << 8 |
			(uint32_t)head[0];
	}
	static uint32_t readWord(char* head) {
		return 	(uint32_t)head[0] << 24 |
			(uint32_t)head[1] << 16 |
			(uint32_t)head[2] << 8 |
			(uint32_t)head[3];
	}
};
