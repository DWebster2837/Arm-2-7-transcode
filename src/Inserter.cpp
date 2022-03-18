#include "Inserter.h"
#include <vector>
#include <queue>

class Inserter {
public:
	Inserter() {
		Tokens = std::vector<Token>{};
		//This value is heuristic; ideally, I would
		//test how many tokens per kB of program is average
		//but I don't think it matters too much.
		Tokens.reserve(1000000);
	}

	void addToken(uint32_t fromAddress, uint32_t toAddress, int encoding) {
		Tokens.push_back(Token(fromAddress, toAddress, encoding));
	}
	void addInsert(uint32_t start, int size, uint32_t* data) {
		inserts.push_back(insSegment(start, size, std::vector<uint32_t>(data, data+size)));
	}
	void addUnchanged(uint32_t start, uint32_t end) {
		unchanged.push_back(imgSegment(start, end));
	}
	void Run() {
		while (!unchanged.empty() || !inserts.empty()) {

		}
	}
private:
	std::vector<Token> Tokens;
	std::deque<imgSegment> unchanged;
	std::deque<insSegment> inserts;


};

