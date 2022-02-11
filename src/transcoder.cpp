// disassembler1.cpp : Defines the entry point for the application.
//

#include "transcoder.h"

using namespace std;


//checks whether bit at position pos is 1
#define bitcheck(data, pos) (data & (1 << pos))
#define getbits(data, start, end) (data & ((1 << start) + (1 << end))) >> end


int main(int argc, char *argv[])
{
	if (argc != 2) {
		throw runtime_error("Invalid argument count; format is input file, output file.");
	}
	//TODO: check arguments are (valid) files

	const char* infilename = argv[1];
	ifstream file(infilename, std::ios::binary);

	if(!file){
		throw runtime_error("File cannot be read");
	}
	streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (size < 34) {
		throw runtime_error("File smaller than an ELF header");
		//if file is smaller than an ELF header, the header decode would read junk. 
	}
	//read the file into buffer
	vector<char> buffer(size);
	if (!file.read(buffer.data(), size)) {
		printf("file read was bad");
		return -1;
	}

	char* head = &buffer[0];
	list<string> output;

	//TODO: make this loop something more able to handle multiple segments
	while (head < &buffer[size]) {

		//Assembly pattern-matching goes here
		
	}
	return 0;
}
