#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <math.h>
#include <cmath>
#include <cstdlib>
#include <bits/stdc++.h>
#include <chrono>

using namespace std;

//an instruction class, holds the address and whether it is a read/write
class instruction{
	public:

	unsigned long long address;
	string flag;
	instruction();
};

instruction::instruction(){
}

//instruction construction
/*instruction::instruction(string addr, string load_store){
	istringstream ss(addr);
	ss >> std::hex >> this->address;
//	this->address = addr;
	this->flag = load_store;
}*/

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
	vector<unsigned long long> cache(setSize);
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
	vector<cacheSet> cache(totalSets);
	int index;
	unsigned long long tag;
	bool found;
	int universalIndex;
	int lru = 0;
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
				lru  = j;
			}
		}
		if(!found){
			universalIndex = lru*waySize+index;
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
	vector<cacheSet> cache(totalSets);
	for(int i = 0; i < totalSets; i++){
		cache[i].validBit = 0;
		cache[i].tag = 0;
		
	}
	int index;
	unsigned long long tag;
	bool found;
	int lru = 0;
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
				lru = j;
			}
		}
		if(!found){
			cache[lru].tag = tag;
			cache[lru].validBit = total;
		}
	}
	return hits;
}

int hotCold(){ //need to make hot/cold LRU approximation
	int hits = 0;
	int total = 0;
	int totalSets = 16384/32;
	vector<cacheSet> cache(totalSets);
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
						parent = 2*(parent+1);
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
			cache[index].tag = tag;
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
	vector<cacheSet> cache(totalSets);
	int index;
	unsigned long long tag;
	bool found;
	int universalIndex;
	int lru = 0;
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
				lru = j;
			}
		}
		if(!found || trace[i].flag == "L"){
			universalIndex = lru*waySize+index;
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
	vector<cacheSet> cache(totalSets);
	int index;
	unsigned long long tag;
	bool found;
	int universalIndex;
	int lru = 0;
	int LRUtime;
	for(int i = 0; i < trace.size(); i++){
		//dealing with individual instructions goes here
		total++;
		lrucounter++;
		LRUtime = INT_MAX;
		index = (trace[i].address>>5)%setSize;
		tag = trace[i].address>>((unsigned long long)(log2(setSize))+5);
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
				lru = j;
			}
		}
		if(!found){
			universalIndex = lru*waySize+index;
			cache[universalIndex].tag = tag;
			cache[universalIndex].validBit = lrucounter;
		}


		//prefetching goes here: its just the top repeated with a different tag/index
		tag = trace[i].address+32>>((unsigned long long)(log2(setSize))+5);
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
				lru = j;
			}
		}
		if(!found){
			universalIndex = lru*waySize+index;
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
	vector<cacheSet> cache(totalSets);
	int index;
	unsigned long long tag;
	bool found;
	int universalIndex;
	int lru = 0;
	int LRUtime;
	for(int i = 0; i < trace.size(); i++){
		//dealing with individual instructions goes here
		total++;
		lrucounter++;
		LRUtime = INT_MAX;
		index = (trace[i].address>>5)%setSize;
		tag = trace[i].address>>((unsigned long long)(log2(setSize))+5);
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
				lru = j;
			}
		}
		if(!found){
			universalIndex = lru*waySize+index;
			cache[universalIndex].tag = tag;
			cache[universalIndex].validBit = lrucounter;
		}

		if(!found){
			//prefetching goes here: its just the top repeated with a different tag/index
			tag = trace[i].address+32>>((unsigned long long)(log2(setSize))+5);
			index = (trace[i].address+32>>5)%setSize;
			LRUtime = lru;
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
					lru = j;
				}
			}
			if(!found){
				universalIndex = lru*waySize+index;
				cache[universalIndex].tag = tag;
				cache[universalIndex].validBit = lrucounter;
			}
		}


	}

	return hits;

}


int main(int argc, char *argv[]){
	string input = argv[1];
	string output = argv[2];

	ifstream ifile(input);
	string line;
	unsigned long long addr;
	string f;
	int countem = 0;
	//it is entirely possible that the vector runs out of space
	cout << trace.max_size() << endl;
	while(ifile >> f >> std::hex >> addr){
		countem++;

//		stringstream ss(line);
//		printf("ples");
//		ss>>f>>addr;
//		printf("1: %s\n2: %s\n\n", addr, f);
//		instruction greg;
//		greg.address = addr;
//		if(f != NULL){
//		greg.flag = f;
//		}
//		trace[countem].address = addr;
//		trace[countem].flag = f;
//		trace.push_back(greg);
//		printf("%lu", trace[countem].address);
	}

	trace.resize(countem+1);
	cout << "AHHHHHHH" << countem <<endl;
	int counter2 = 0;
	ifstream repeat(input);
	while(repeat >> f >> std::hex >> addr){
		
		
//		cout << counter2 << endl;
		trace[counter2].address = addr;
		trace[counter2].flag = f;
		counter2++;
		if(counter2 == countem-2){
			cout << "break at dotted line" << endl;
			break;
			cout << "It didn't break, did it?" << endl;
		}
	}

//	printf("Do we get here?");
	cout << "this blows" <<endl;
	int oneA = directMapped(1024);
	int oneB = directMapped(4096);
	int oneC = directMapped(16384);
	int oneD = directMapped(32768);
	cout << "how low can we go?" << endl;
	int twoA = setAssociative(2);
	int twoB = setAssociative(4);
	int twoC = setAssociative(8);
	int twoD = setAssociative(16);
	cout << "can we go down low?" << endl;

	cout << "how is this even possible? will this break it?" << endl;
//	int threeA = fullAssociative();
	cout << "we can make it about this low" << endl;
	int threeB = hotCold();
	cout << "all the way to the flo" << endl;
	int fourA = setNoWrite(2);
	int fourB = setNoWrite(4);
	int fourC = setNoWrite(8);
	int fourD = setNoWrite(16);
	cout << "can we make it to the top?" << endl;
	int fiveA = prefetch(2);
	int fiveB = prefetch(4);
	int fiveC = prefetch(8);
	int fiveD = prefetch(16);
	cout << "like we're never gonna stop?" << endl;
	int sixA = prefetchMiss(2);
	int sixB = prefetchMiss(4);
	int sixC = prefetchMiss(8);
	int sixD = prefetchMiss(16);
	cout << "booger blaster" << endl;

	printf("%d,%d, %d, %d, %d", oneA, oneB, oneC, oneD, countem);

}
