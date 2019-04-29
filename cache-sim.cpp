#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <math>
#include <chrono>

using namespace std;

//an instruction class, holds the address and whether it is a read/write
class instruction{
	public:

	unsigned long long address;
	string flag;
	instruction(string addr, string load_store);
};

//instruction construction
instruction::instruction(string addr, string load_store){
	stringstream ss;
	ss << std::hex << addr;
	ss >> this->address;
	this->flag = load_store;
}

//only use tag and valid bit, but that's all you need. This is a single cacheline
class cacheSet{
	public:

	int index;
	int validBit; //validBit is used both for LRU and cold/hot
	int tag;
	int data;
};

//the trace goes here
vector<instruction> trace;

//DONT FORGET TO push_back() when the vector is first empty
int directMapped(int cacheSize){
	int hits = 0;
	int total = 0;
	int setSize = cacheSize/32;
	vector<unsigned long long> cache[setSize];
	int index;
	unsigned long long tag;
	for(int i = 0; i< trace.size(); i++){
		//dealing with individual instructions goes here
		total++;
		index = (trace[i].address>>5)%setSize;
		tag = trace[i].address>>((unsigned long long)(log2(setSize))+5);
		if(cache[index] == tag){
			hits++;
		}else{
			cache[index] = tag;
		}
 	}
 	return hits;
}


int setAssociative(int associativity){
	int hits = 0;
	int total = 0;
	int totalSets = 16384/32;
	int setSize = totalSets/associativity;
	int waySize = 16384/associativity;
	vector<cacheSet> cache[totalSets];
	int index;
	unsigned long long tag;
	bool found;
	int universalIndex;
	int LRU = 0;
	int LRUtime;
	for(int i = 0; i < trace.size(); i++){
		//dealing with individual instructions goes here
		total++;
		LRUtime = INT_MAX;
		index = (trace[i].address>>5)%setSize;
		tag = trance[i].address>>((unsigned long long)(log2(setSize))+5);
		found = false;
		for(int j = 0; j < associativity; j++){
			universalIndex = j*waySize+index;
			if(cache[universalIndex].tag == tag && !found){
				hits++;
				cache[universalIndex].validBit == total; //sets validBit to instr# to keep track of LRU
				found = true;
			}
			if(LRUtime > cache[universalIndex].validBit){
				LRUtime = cache[universalIndex].validBit;
				LRU = j;
			}
		}
		if(!found){
			universalIndex = LRU*waySize+index;
			cache[universalIndex].tag = tag;
			cache[universalIndex].validBit = total;
		}
	}

	return hits;

}


int fullAssociative(){
	int hits = 0;
	int total = 0;
	int totalSets = 16384/32;
	vector<cacheSet> cache[totalSets];
	int index;
	unsigned long long tag;
	bool found;
	int LRU = 0;
	int LRUtime;
	for(int i = 0; i < trace.size(); i++){
		total++;
		LRUtime = INT_MAX;
		found = false;
		tag = trace[i].address>>5;
		for(int j = 0; j < totalSets; j++){
			if(cache[j].tag == tag && !found){
				hits++;
				cache[j].validBit == total;
				found = true;
			}
			if(LRUtime > cache[j].validBit){
				LRUtime = cache[j].validBit;
				LRU = j;
			}
		}
		if(!found){
			cache[LRU].tag = tag;
			cache[LRU].validBit = total;
		}
	}
	return hits;
}

int hotCold(){ //need to make hot/cold LRU approximation
	int hits = 0;
	int total = 0;
	int totalSets = 16384/32;
	vector<cacheSet> cache[totalSets];
	for(int i = 0; i < cache.size(); i++){
		cache[i].validBit = 0; //validBit takes the place of the hot cold bit
	}
	unsigned long long tag;
	int index;
	bool found;
	for(int i = 0; i < trace.size(); i++){
		total++;
		found = false;
		tag = trace[i].address>>5;
		for(int j = 0; j < cache.size(); j++){
			if(tag == cache[j].tag && !found){
				hits++;
				found = true;

				//hot cold bit manipulation
				int parent=0;
				for(int power = 0; power < log2(totalSets); power++){
					int oldVal = (j/(totalSets/(2<<power)))%2;
					if(oldVal == 0){
						cache[j].validBit = 1;
						parent = (2* parent)+1;
					}else{
						cache[j].validBit = 0;
						parent = 2*(parent+1)
					}

				}
			}

		} //what if we didn't find it?
		if(!found){
			index=0;
			for(int power = 0; power < log2(totalSets); power++){
				if(cache[index].validBit == 0){
					cache[index].validBit = 1;
					index = (2*index)+1;
				}else{
					cache[index].validBit = 0;
					index = 2*(index+1);
				}
			}
			index = index + 1 - totalSets;
			cache[index] = tag;
		}
	}

	return hits;
}


int setNoWrite(int associativity){
	int hits = 0;
	int total = 0;
	int totalSets = 16384/32;
	int setSize = totalSets/associativity;
	int waySize = 16384/associativity;
	vector<cacheSet> cache[totalSets];
	int index;
	unsigned long long tag;
	bool found;
	int universalIndex;
	int LRU = 0;
	int LRUtime;
	for(int i = 0; i < trace.size(); i++){
		//dealing with individual instructions goes here
		total++;
		LRUtime = INT_MAX;
		index = (trace[i].address>>5)%setSize;
		tag = trace[i].address>>((unsigned long long)(log2(setSize))+5);
		found = false;
		for(int j = 0; j < associativity; j++){
			universalIndex = j*waySize+index;
			if(cache[universalIndex].tag == tag && !found){
				hits++;
				cache[universalIndex].validBit == total; //sets validBit to instr# to keep track of LRU
				found = true;
			}
			if(LRUtime > cache[universalIndex].validBit){
				LRUtime = cache[universalIndex].validBit;
				LRU = j;
			}
		}
		if(!found || trace[i].flag == "L"){
			universalIndex = LRU*waySize+index;
			cache[universalIndex].tag = tag;
			cache[universalIndex].validBit = total;
		}
	}

	return hits;

}

int prefetch(int associativity){ //Need to make it prefetch
	int hits = 0;
	int total = 0;
	int lrucounter = 0;
	int totalSets = 16384/32;
	int setSize = totalSets/associativity;
	int waySize = 16384/associativity;
	vector<cacheSet> cache[totalSets];
	int index;
	unsigned long long tag;
	bool found;
	int universalIndex;
	int LRU = 0;
	int LRUtime;
	for(int i = 0; i < trace.size(); i++){
		//dealing with individual instructions goes here
		total++;
		lrucounter++;
		LRUtime = INT_MAX;
		index = (trace[i].address>>5)%setSize;
		tag = trance[i].address>>((unsigned long long)(log2(setSize))+5);
		//to prefetch, get a second tag, then run the bellow code again for that tag
		found = false;
		for(int j = 0; j < associativity; j++){
			universalIndex = j*waySize+index;
			if(cache[universalIndex].tag == tag && !found){
				hits++;
				cache[universalIndex].validBit == lrucounter; //sets validBit to instr# to keep track of LRU
				found = true;
			}
			if(LRUtime > cache[universalIndex].validBit){
				LRUtime = cache[universalIndex].validBit;
				LRU = j;
			}
		}
		if(!found){
			universalIndex = LRU*waySize+index;
			cache[universalIndex].tag = tag;
			cache[universalIndex].validBit = lrucounter;
		}


		//prefetching goes here: its just the top repeated with a different tag/index
		tag = trance[i].address+32>>((unsigned long long)(log2(setSize))+5);
		index = (trace[i].address+32>>5)%setSize;
		LRUtime = INT_MAX;
		lrucounter++;

		found = false;
		for(int j = 0; j < associativity; j++){
			universalIndex = j*waySize+index;
			if(cache[universalIndex].tag == tag && !found){
				//hits++; WE do not count hits when prefetching
				cache[universalIndex].validBit == lrucounter; //sets validBit to instr# to keep track of LRU
				found = true;
			}
			if(LRUtime > cache[universalIndex].validBit){
				LRUtime = cache[universalIndex].validBit;
				LRU = j;
			}
		}
		if(!found){
			universalIndex = LRU*waySize+index;
			cache[universalIndex].tag = tag;
			cache[universalIndex].validBit = lrucounter;
		}


	}

	return hits;

}


//Need to make prefetch on a miss
//^ it will look the same as the one above, plus an if statement to make sure it only fetches on a miss
int prefetchMiss(int associativity){ //Need to make it prefetch
	int hits = 0;
	int total = 0;
	int lrucounter = 0;
	int totalSets = 16384/32;
	int setSize = totalSets/associativity;
	int waySize = 16384/associativity;
	vector<cacheSet> cache[totalSets];
	int index;
	unsigned long long tag;
	bool found;
	int universalIndex;
	int LRU = 0;
	int LRUtime;
	for(int i = 0; i < trace.size(); i++){
		//dealing with individual instructions goes here
		total++;
		lrucounter++;
		LRUtime = INT_MAX;
		index = (trace[i].address>>5)%setSize;
		tag = trance[i].address>>((unsigned long long)(log2(setSize))+5);
		//to prefetch, get a second tag, then run the bellow code again for that tag
		found = false;
		for(int j = 0; j < associativity; j++){
			universalIndex = j*waySize+index;
			if(cache[universalIndex].tag == tag && !found){
				hits++;
				cache[universalIndex].validBit == lrucounter; //sets validBit to instr# to keep track of LRU
				found = true;
			}
			if(LRUtime > cache[universalIndex].validBit){
				LRUtime = cache[universalIndex].validBit;
				LRU = j;
			}
		}
		if(!found){
			universalIndex = LRU*waySize+index;
			cache[universalIndex].tag = tag;
			cache[universalIndex].validBit = lrucounter;
		}

		if(!found){
			//prefetching goes here: its just the top repeated with a different tag/index
			tag = trance[i].address+32>>((unsigned long long)(log2(setSize))+5);
			index = (trace[i].address+32>>5)%setSize;
			LRUtime = INT_MAX;
			lrucounter++;

			found = false;
			for(int j = 0; j < associativity; j++){
				universalIndex = j*waySize+index;
				if(cache[universalIndex].tag == tag && !found){
					//hits++; WE do not count hits when prefetching
					cache[universalIndex].validBit == lrucounter; //sets validBit to instr# to keep track of LRU
					found = true;
				}
				if(LRUtime > cache[universalIndex].validBit){
					LRUtime = cache[universalIndex].validBit;
					LRU = j;
				}
			}
			if(!found){
				universalIndex = LRU*waySize+index;
				cache[universalIndex].tag = tag;
				cache[universalIndex].validBit = lrucounter;
			}
		}


	}

	return hits;

}

