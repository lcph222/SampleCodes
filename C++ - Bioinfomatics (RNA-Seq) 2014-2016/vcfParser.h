//this class allows user to parse vcf file and extract snp info contains in each individual sample and 2 haplotypes 

#ifndef _vcfParser_H
 #define _vcfParser_H

#include<string>
#include<sstream>
#include<vector>

using namespace std;


struct vcfField{ //struct used to store parse tokens from each line

	int numFieldFound;

	string CHROM;
	int POS;
	string ID;
	string REF;
	string ALT;
	int QUAL;
	string FILTER;
	string INFO;
	string FORMAT;
	string sampleID;
	vector<string> sampleIdList;
};

struct sample{ //each sample will have 2 output files for 2 haplotypes (SNP)
	string sampleID;
	string chrName; 
	ofstream *fout1, *fout2;
	
	sample(){
		fout1 = NULL; fout2 = NULL;
	}


};

class vcfParser{
private:
	vcfField tempField; //current readline
public:
	bool parseLine(string line, int startPos); //Parse line
	bool buildSNPFile(string fn,string sampleName,string outputPath1,string outputPath2); //build snp
	bool buildSNPFileFromList(string fn,  string sampleFolder,  string chrName, int startPosition); //build snp from a list
	void setSampleVector(string line, vector<sample> & v,string sampleFolder, string chrName); //read in and set sample information
	void closeFilePointer(vector<sample> & v); //close ofstream
};


#endif