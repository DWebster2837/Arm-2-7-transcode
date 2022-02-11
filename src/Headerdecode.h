#pragma once
#include <stdio.h>
#include <stdexcept>


struct headerData {
	//executable flags
	uint32_t e_flags;


	//program header table offset, entry size and entry count. 
	uint32_t e_phoff;
	uint16_t e_phentsize;
	uint16_t e_phnum;


	//section header table offset, entry size, entry count and section name entry index
	uint32_t e_shoff;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;

	bool endian;
};

struct headerData elfHeaderDecode(char* buffer);

