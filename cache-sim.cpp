#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <chrono>

using namespace std;

class instruction{
	public:

	unsigned int address;
	string flag;
	instruction(string addr, string load_store);
};

instruction::instruction(string addr, string load_store){
	stringstream ss;
	ss << std::hex << addr;
	ss >> this->address;
	this->flag = load_store;
}

class cacheSet{
	public:

	int index;
	int validBit;
	int tag;
	int data;
};

vector<instruction> trace;

int directMapped(int cacheSize){
	int hits = 0;
	int setSize = cacheSize/32;
	vector<cacheSet> cache[setSize];

	for(int i = 0; i< trace.size(); i++){
		//dealing with individual instructions goes here

	}
}
