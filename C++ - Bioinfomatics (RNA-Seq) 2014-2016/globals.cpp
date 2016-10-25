#include"globals.h"
#include<sstream>


string GroundTruth = "GroundTruth";
string GSNAP = "GSNap";
bool test = true; //sam
bool sorted = false; //directory, sort file
bool debug = false;
bool outputScenarioDebug = true;
int lowestCoverageConstant = 5;
int maxEmpty = 10;
int maxIntronBase = 10;

int spliceJunctionOffset = 10;
int AllowMismatch = 3;

string intToString(int i){
	std::string s;
	std::stringstream out;
	out << i;
	s = out.str();
	return s;
}

int stringToInt(string s){

	int i;

	istringstream buffer(s);
	buffer>>i;
	
	return i;
}

bool checkBoundOverlap(int start, int end, int start2, int end2){

	if(end2 >= start && end2 <= end || start2 >= start && start2 <= end || start2 <= start && end2 >= end || start2 >= start && end2 <= end)
		return true;

	return false;
}

bool checkOverlap(Alignment * Al1, Alignment * Al2){

	if(Al1->RName != Al2->RName) //not in same chromosome
		return false;

	if(checkBoundOverlap(Al1->start, Al1->end, Al2->start,Al2->end))
		return true;

	return false;
}

bool sameAlignments(Alignment * Al,Alignment *Al2){
	
	if(Al->QName == Al2->QName && Al->RName == Al2->RName && Al->Pos == Al2->Pos && Al->Cigar == Al2->Cigar)
		return true;

	return false;
}

int findGroupIterator(vector<UniqueGroup> &v, Alignment * Al){

	for(int i = 0; i < v.size(); i++){
	
		if(Al->RName == v.at(i).chrName && checkBoundOverlap(v.at(i).start,v.at(i).end,Al->start,Al->end))
			return i;
	}

	return -1;
}

int findExactMatchIterator(vector<Alignment> &vectorAl,Alignment *Al){
	
	for(int i = 0; i < vectorAl.size(); i++){
		if(sameAlignments(&(vectorAl.at(i)),Al))
			return i;
	}

	return -1;
}

int getAvgReadPerBase(Alignment * temp, ReferenceGenomeSequenceObj * refObj){

	int totalCover = 0;

	for(int i = 0; i < temp->vectorCig.size(); i++){
		cig * c = &(temp->vectorCig.at(i));
		if(c->name == 'M'){
			for(int j = 0; j < c->value; j++)
				totalCover += refObj->chromosomeMap[temp->RName].chrMCountArray[j + c->refPos];
		}
	}

	return (int) totalCover/temp->numMatched;
}

