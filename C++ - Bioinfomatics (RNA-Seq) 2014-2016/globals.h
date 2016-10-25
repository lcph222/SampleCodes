//this is a global file which contains some global function and variables

#ifndef _globals_h
 #define _globals_h


#include<string>
#include"helper.h"
#include<fstream>
using namespace std;

extern string GroundTruth;
extern string GSNAP; 
extern int ReadLength; //length of paired-ends read
extern bool test; //test is true is when reading in a file on window or not tab delimited 
extern bool sorted; //if file is sorted or not
extern int spliceJunctionOffset; //allow number of offset for splice junctions position when compare with grouth truth
extern int AllowMismatch; //allow mismatch when comparing
extern bool debug; //debug is debug, then output some debug file
extern bool outputScenarioDebug; //output debug files if this is true
extern int lowestCoverageConstant; //minimum coverage
extern int maxEmpty; //number of empty based allow in splice junction
extern int maxIntronBase; //max intron base allow

//function
extern string intToString(int i); //convert int to string
int stringToInt(string s); //convert string to int
bool checkBoundOverlap(int start, int end, int start2, int end2); //check if two read overlap
int findGroupIterator(vector<UniqueGroup>& v, Alignment *Al); //find iterator for an object
int findExactMatchIterator(vector<Alignment> &vectorAl,Alignment *Al); 
bool sameAlignments(Alignment * Al,Alignment *Al2); //check if two alignments is the same
int getAvgReadPerBase(Alignment * temp, ReferenceGenomeSequenceObj * refObj); //get coverage
bool checkOverlap(Alignment * Al1, Alignment * Al2); //check if two alignment is overlap or not

//struct
struct customOfstream{

	ofstream * arrayOutFile;
	int size;

	customOfstream(){size = 0; arrayOutFile = NULL;};
	bool set(int size, string fn, string extension){
	
		if(arrayOutFile != NULL)
			return false;

		this->size = size;

		arrayOutFile = new ofstream[size];

		for(int i = 0; i < size; i++){
		
			string filename = fn + intToString(i + 1) + extension;
			cout<<"Opening "<<fn<<endl;
			arrayOutFile[i].open(filename.c_str());
			
			if(!arrayOutFile){
				deSet();
				return false;
			}else
				cout<<"Successed"<<endl;
		}
	}

	void deSet(){
	
		if(arrayOutFile != NULL && size > 0){
		
			for(int i = 0; i < size; i++){
				arrayOutFile[i].close();
			}

			delete [] arrayOutFile;
			arrayOutFile = NULL;
		}

	}

	ofstream * getPointerToFile(){
		return this->arrayOutFile;
	}

};

#endif