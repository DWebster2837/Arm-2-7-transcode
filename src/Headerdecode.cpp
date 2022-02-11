#include "headerdecode.h"


headerData elfHeaderDecode(char* buffer) {
	//check file format magic number; currently, just checks for ELF. 
	if (!(buffer[0] == 0x7F &&
		buffer[1] == 0x45 &&
		buffer[2] == 0x4c &&
		buffer[3] == 0x46)) {
		throw std::runtime_error("File does not have an ELF header");
	}
	if (buffer[4] != 0x01) {
		throw std::runtime_error("File is not 32-bit");
	}
	//if (buffer[5] != 0x01) {
	//	printf("File is not little-endian");
	//	exit(0);
	//}
	//Code supports both endian-ness-ess
	if (buffer[10] != 0x02) {
		throw std::runtime_error("File is not an executable");
	}
	if (buffer[12] != 0x28) {
		throw std::runtime_error("File instruction set is not ARM. Note that 64-bit ARM is not supported.");
	}
	struct headerData data{};
	//flags are instruction set specific. In ARM's case, it's unused (according to ARM DUI 0101 A). 
	data.e_flags = buffer[0x24];

	//program header table offset, entry size and entry count. 
	data.e_phoff = buffer[0x1C];
	data.e_phentsize = buffer[0x2A];
	data.e_phnum = buffer[0x2C];

	//section header table offset, entry size, entry count and section name entry index
	data.e_shoff = buffer[0x20];
	data.e_shentsize = buffer[0x2E];
	data.e_shnum = buffer[0x30];
	data.e_shstrndx = buffer[0x32];

	return data;
	//NOTE: this is not exhaustive; fields that are not used in this program are not decoded by this. 
}

