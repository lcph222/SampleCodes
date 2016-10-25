//gaf class for loading gaf file

#ifndef _gafLoader_H
 #define _gafLoader_H

#include<iostream>
#include<vector>
using namespace std;

struct gafField{

	string id;
	string chrName;
	string strand;
	int field4;
	int field5;
	int field6;
	int field7;
	int numExon;
	vector<int> exonStartPosVector;//0-based
	vector<int> exonEndPosVector; //0-based
	string field11;
	string field12;
};

class gafLoader{

private:

	vector<gafField> gafFieldVector;

public:

	bool loadGafFile(string fn);
	vector<gafField> getGafVector();
};


#endif