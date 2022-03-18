#pragma once
#include <cstdint>
#include <vector>
struct Token {
	uint32_t fromAddress;
	uint32_t toAddress;
	
	//TODO: way to handle modifying the address when stuff's moved
	//Means a set of functions to handle re-generating the inputs with the new values.
	int encoding;
};

struct imgSegment {
	uint32_t startAddress;
	uint32_t endAddress;
};

struct insSegment {
	uint32_t startAddress;
	int size;
	std::vector<uint32_t> data;
};
