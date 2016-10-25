//this class is used to parse snp from vcf files (1000 genomes project)
//since vcf files are huge, 2500+ samples and it's too big to store in RAM, 
//I had to parsed 500 samples at a time and 100,000 Lines at a time. 
//buildSNPFileFromList initialize the work

#include"vcfParser.h"
#include"globals.h"
#include<fstream>
#include<iostream>

const int MAXFILESTREAM = 505; //max files to open at the same time
const int MAXLINE = 100000; //number of line to process each time

struct sampleResult{
	bool error; //no phase or unphase
	bool phase;
	bool firstHasAlt,secondHasAlt;
	char firstAltChar,secondAltChar;
	int firstAltPos,secondAltPos;
	sampleResult(){
		phase = false; firstHasAlt = false; secondHasAlt = false; 
		error = false;
	}
	void reset(){
		phase = false; firstHasAlt = false; secondHasAlt = false; 
		error = false;
	}
	
};

struct Filter{

	bool refSingle;
	bool filterPass;
	bool formatGTExist;
	bool sampleIDPass;
	vector<sampleResult> vectorSampleResult; //vector sample result
	//bool phase;
	//int firstAltPos,secondAltPos;
	//bool firstHasAlt,secondHasAlt;
	
	void clear(){refSingle = false; filterPass = false;
				 formatGTExist = false; sampleIDPass = false; //phase = false;
				 //firstHasAlt = false; secondHasAlt = false; 
				 //vectorSampleResult.clear();
	}
	void setVectorSampleResult(int size){
	
		vectorSampleResult.clear();
		for(int i = 0; i < size; i++){
			//test each sample
			sampleResult sampRes;
			this->vectorSampleResult.push_back(sampRes);
		}
	}
};

//return true if success, false otherwise, also mark result.error = true
bool sampleResultTest(vcfField &testObj, sampleResult & result, string s){

		result.reset();
		//result.error = false;

		std::size_t found = s.find(':');
		string genotypeString;
		if(found != std::string::npos){
			genotypeString = s.substr(0,(int)found);
		}
		else
			genotypeString = s;

		//serach for '/' or '|'
		std::size_t found2 = genotypeString.find('|');
	
		if(found2 != std::string::npos){
			result.phase = true;
		}
		else{ //'/'
			result.phase = false;
			found2 = genotypeString.find('/');
			if(found2 == std::string::npos){
				//cout<<"didn't found '/' or '|' in sampleID when GT is presents!";
				result.error = true;
				return false;
			}
		}

		

		//nothing failed yet
		result.firstAltPos = stringToInt(genotypeString.substr(0,found2));
		result.secondAltPos = stringToInt(genotypeString.substr(found2 + 1));


		//must be at least one non zero 
		if(result.firstAltPos == 0 && result.secondAltPos == 0){
			result.error = true;
			return false;
		}

		string altStringToken;
		std::istringstream altSS(testObj.ALT);
		vector<string> altVector;

		while(getline(altSS,altStringToken,',')){
			altVector.push_back(altStringToken); //parse and store in vector
		}

		//test for first one
		if(result.firstAltPos <= altVector.size()){
			if(result.firstAltPos > 0 && altVector.at(result.firstAltPos -1).size() == 1){
				result.firstHasAlt = true;
				result.firstAltChar = altVector.at(result.firstAltPos -1).at(0); //convert to char
			}
		}else{
			cout<<"found alt pos greater than the number of alternative"<<endl;
			cout<<testObj.CHROM<<" "<<testObj.POS<<" "<<testObj.ALT<<" "<<testObj.sampleID<<endl;
			result.error = true;
			return false;
		}

		//test for second one
		if(result.secondAltPos <= altVector.size()){
			if(result.secondAltPos > 0 && altVector.at(result.secondAltPos -1).size() == 1){ //only if alternative is also 1 to 1
				result.secondHasAlt = true;
				result.secondAltChar = altVector.at(result.secondAltPos -1).at(0); //convert to char
			}
		}else{
			cout<<"found alt pos greater than the number of alternative"<<endl;
			cout<<testObj.CHROM<<" "<<testObj.POS<<" "<<testObj.ALT<<" "<<testObj.sampleID<<endl;
			result.error = true;
			return false;
		}

		if(!result.firstHasAlt && !result.secondHasAlt){
			result.error = true;
			return false;
		}

		return true;
}

//return false if failed, 
bool filterTest(vcfField &testObj,Filter & result, string & line,int startPos){
	//reset to false
	result.clear();
	
	if(testObj.REF.size() == 1)
		result.refSingle = true;
	else
		return false;

	if(testObj.FILTER == "PASS")
		result.filterPass = true;
	else
		return false;

	std::istringstream ss(testObj.FORMAT);
	string token;
	int field = 1;
	//GT has to be the first field if exist
	while(std::getline(ss,token,':')){
		if(token == "GT" && field == 1)
			result.formatGTExist = true;
		field++;
	}

	//if true
	if(result.formatGTExist){
	
		//do for each one
		for(int i= startPos; i < (startPos + MAXFILESTREAM); i++){	

			//test each sample
			//sampleResult sampRes;
			
			//push back empty if not in search ragne
			//if(i < startPos || i >= (startPos + MAXFILESTREAM)){
			//	result.vectorSampleResult.push_back(sampRes);
			//	continue;
			//}

			if(i >= result.vectorSampleResult.size())
				break;
			
			sampleResultTest(testObj, result.vectorSampleResult.at(i), testObj.sampleIdList.at(i)); //i is sample i in vector
			
			//push back result
			//result.vectorSampleResult.push_back(sampRes); //store results
		}
	
	}else
		return false;

	//if(!result.firstHasAlt && !result.secondHasAlt)
		//return false;

	return true;
}

//position is
bool vcfParser::parseLine(string line, int startPos){

	if(line.size() < 2 || line.at(0) == '#')
		return false;

	//clear vector
	this->tempField.sampleIdList.clear();

	istringstream is(line);
	string token;
	int field = 1;
	bool done=false;

	if(!test){
		//10 only works for 1 sample
		//while(field <= 10 && std::getline(is,token,'\t')){
		while(std::getline(is,token,'\t')){

			switch(field){
				case 1: this->tempField.CHROM = token; break;	
				case 2: this->tempField.POS = stringToInt(token); break;
				case 3: this->tempField.ID = token;  break;
				case 4: this->tempField.REF = token; break;
				case 5: this->tempField.ALT = token; break;
				case 6: this->tempField.QUAL = stringToInt(token); break;
				case 7: this->tempField.FILTER = token; break;
				case 8: this->tempField.INFO = token; break;
				case 9: this->tempField.FORMAT = token; break;
				default: 
					if((field - 10) < (startPos + MAXFILESTREAM)){
						this->tempField.sampleIdList.push_back(token); 
					}
					else{
						done = true;
					} break;

				//case 10: this->tempField.sampleID = token; break;
			}//switch

			field++;
			if(done)
				break;
		}//while

	}//if
	else{
		is>>tempField.CHROM>>tempField.POS>>tempField.ID>>tempField.REF>>tempField.ALT;
		is>>tempField.QUAL>>tempField.FILTER>>tempField.INFO>>tempField.FORMAT; //>>tempField.sampleID;
		while(is>>token)
			tempField.sampleIdList.push_back(token);
	}

	tempField.numFieldFound = field -1;//start at 1

	if(field < 11 && !test){
		cout<<"found field of size "<<field<<endl;
		cout<<line<<endl;
		return false;
	}

	return true;
}

//fempFields and filters are parrallel vector
//sampleVector and filter.vectorSampleResult are parrallel vector
void outputVcfSNP(vector<vcfField> & tempFields, vector<Filter> & filters, vector<sample> & sampleVector,int startPosition){

	for(int i = startPosition; i < (startPosition + MAXFILESTREAM); i++){
	
		if(i >= sampleVector.size())
			break; //don't do when i is greater than the max index

		ofstream * fout = sampleVector.at(i).fout1;
		ofstream * fout2 = sampleVector.at(i).fout2;

		for(int j = 0; j < tempFields.size(); j++){ // for each result

			//filter only stored temp line, current, vectorSampleResult stores maxfilesstream
			sampleResult * samRes = &filters.at(j).vectorSampleResult.at(i);
			string sampleId = sampleVector.at(i).sampleID;

			//check for error
			if(samRes->error){
				continue;
			}


			vcfField obj = tempFields.at(j); //tempFields only stored current level
			string chrName = "chr" + obj.CHROM;
		
		
			if(!fout->is_open() || !fout2->is_open())
				cout<<"error in opening file stream"<<endl;

			if(samRes->firstHasAlt){
				*fout<<chrName<<"\t"<<obj.POS<<"\t"<<obj.REF<<"->"<<samRes->firstAltChar<<"\t"<<obj.QUAL<<"\t"<<obj.FORMAT<<"\t";
				//<<obj.sampleIdList.at(i)<<"\t"<<obj.REF<<"\t"<<obj.ALT;
				if(samRes->phase)
					*fout<<"\tPhase";
				else
					*fout<<"\tNonPhase";

				if(obj.QUAL == 100)
					*fout<<"\tPerfect";
				else
					*fout<<"\tNonPerfect";
				*fout<<endl;
			}
			if(samRes->secondHasAlt){
				*fout2<<chrName<<"\t"<<obj.POS<<"\t"<<obj.REF<<"->"<<samRes->secondAltChar<<"\t"<<obj.QUAL<<"\t"<<obj.FORMAT<<"\t";
				//<<obj.sampleIdList.at(i)<<"\t"<<obj.REF<<"\t"<<obj.ALT;
				if(samRes->phase)
					*fout2<<"\tPhase";
				else
					*fout2<<"\tNonPhase";
				if(obj.QUAL == 100)
					*fout2<<"\tPerfect";
				else
					*fout2<<"\tNonPerfect";
				*fout2<<endl;
			}

		}//for

	}//outer for
}

bool vcfParser::buildSNPFile(string fn,string sampleName,string outputPath1,string outputPath2){

	cout<<"Building snp for file "<<fn<<endl;

	string snpFn1 = outputPath1 + sampleName + "_SNP_Genome1.txt";
	string snpFn2 = outputPath2 + sampleName + "_SNP_Genome2.txt";
	
	ofstream fout1(snpFn1.c_str());
	ofstream fout2(snpFn2.c_str());

	ifstream fin(fn.c_str());


	if(!fin){
		cout<<"error opening "<<fn<<endl;
		return false;
	}

	string line;

	while(getline(fin,line)){
	
		//if(!parseLine(line))
			continue;
	
		Filter filterResult;

		//test pass
		//if(filterTest(tempField,filterResult,line)){	
		//	outputVcfSNP(fout1,fout2,tempField,filterResult);
		//}
	}

	fin.close();
	fout1.close();
	fout2.close();

	cout<<"Done.."<<endl;
}

void openFileStream(vector<sample> & v, string sampleFolder, string chrName, int startPosition, int numStream){

	for(int i = startPosition; i < (startPosition + numStream); i++){
		
		if(i >= v.size()) //reach limit
			break;

		string sampleName = v.at(i).sampleID;

		//directory was precomputed and create like the following path from main.cpp --createPSGDir
		string snpFn1 = sampleFolder + sampleName +"/SNPFiles/genome1/" + sampleName + "_SNP_" + chrName + ".txt";
			string snpFn2 = sampleFolder + sampleName +"/SNPFiles/genome2/" + sampleName + "_SNP_" + chrName + ".txt";
			v.at(i).fout1 = new ofstream(snpFn1.c_str());
			v.at(i).fout2 = new ofstream(snpFn2.c_str());
			if((!(v.at(i).fout1)->is_open())|| (!(v.at(i).fout2)->is_open())){
				cout<<"can't open "<<i<<endl;
				v.at(i).fout1->close();
				v.at(i).fout2->close();
				v.at(i).fout1 = NULL;
				v.at(i).fout2 = NULL; 
			}
		
	}

	//cout<<"number of open file: "<<v.size()<<endl;
}

void closeFileStream(vector<sample> & v, int startPosition, int numStream){

	for(int i = startPosition; i < (startPosition + numStream); i++){
		
		if(i >= v.size()) //reach limit
			break;
		
		if(v.at(i).fout1 != NULL){
			v.at(i).fout1->close();
			delete v.at(i).fout1;
			v.at(i).fout1 = NULL;
		}

		if(v.at(i).fout2 != NULL){
			v.at(i).fout2->close();
			delete v.at(i).fout2; 
			v.at(i).fout2 = NULL; 
		}
		
	} //for loop

}

void vcfParser::setSampleVector(string line, vector<sample> & v, string sampleFolder, string chrName){

	string token;
	int field = 1;
	
	istringstream is(line);

	if(!test){
		while(std::getline(is,token,'\t')){
			
			if(field >= 10){ //token is sample id name
				sample s;
				s.sampleID = token;
				v.push_back(s); 
			}
				
			field++;
		}//while

	}//if
	else{
		while(is>>token){
			if(field >= 10){ //token is sample id name
				sample s;
				s.sampleID = token;
				v.push_back(s); //push back
			}
			field++;
		}
	}

}

void vcfParser::closeFilePointer(vector<sample> & v){

	for(int i = 0; i < v.size(); i++){
		if(v.at(i).fout1 != NULL)
			v.at(i).fout1->close();

		if(v.at(i).fout2 != NULL)
			v.at(i).fout2->close();

	}

}

//parse snp from startPosition(start sample to + samples)
//may be slow, can be speed up when needed(lots more complicated codes)
//due to limit of parrallel file stream, i can only open 500 at the same time
//recursive function
bool vcfParser::buildSNPFileFromList(string fn, string sampleFolder, string chrName, int startPosition){


	//map
	ifstream fin(fn.c_str());

	if(!fin){
		cout<<"error opening "<<fn<<endl;
		return false;
	}

	string line;
	vector<sample> sampleVector;
	bool set = false; //when everything is set for output file
	Filter filterResult; //result for each line

	vector<Filter> Results;
	vector<vcfField> vectorTempFields;

	int numProcLine = 1; 

	while(getline(fin,line)){
	
		if(numProcLine % 10000 == 0)
				cout<<"number of proccessed line: "<<numProcLine<<endl;

		if(vectorTempFields.size() >= MAXLINE){
			cout<<"Output next "<<MAXLINE<<"Results "<<numProcLine<<endl;
			//processed
			outputVcfSNP(vectorTempFields,Results,sampleVector,startPosition); //sample  vector is a vector containing each sample and its fout file
			//clear result
			vectorTempFields.clear();
			Results.clear(); 
			cout<<"done..."<<endl;
			//break;
		}

		numProcLine++;

		//parse list of sample
		if(!set && line.size() > 1 ){
			string copy = line;
			istringstream is(copy);
			string token;
			is>>token;
			if(token == "#CHROM"){
				set = true; //found header
				setSampleVector(line,sampleVector,sampleFolder, chrName); //open file stream from startPosition to + numberStream
				openFileStream(sampleVector,sampleFolder,chrName,startPosition,MAXFILESTREAM); 
				//set filterS
				filterResult.setVectorSampleResult(sampleVector.size());
			}
			continue;
		}

		
		if(set){
			//parse is, true if at least 10 fields(1-sample on 10th field)
			if(!parseLine(line,startPosition)) //store result in tempField class-obj
				continue;
				//test pass
	
			if(filterTest(tempField,filterResult,line,startPosition)){	
				//means gt exist
			//	outputVcfSNP(tempField,filterResult,sampleVector,startPosition); //sample  vector is a vector containing each sample and its fout file
				Results.push_back(filterResult); //problem, filterResult stores all sample result
				tempField.sampleIdList.clear(); //clear vector save space, doesn't need it for output
				vectorTempFields.push_back(tempField);
			}

		}
	}

	fin.close(); //close vcf file

	//output result
	if(vectorTempFields.size() > 0)
		outputVcfSNP(vectorTempFields,Results,sampleVector,startPosition); //sample  vector is a vector containing each sample and its fout file

	//closeFilePointer(sampleVector);
	closeFileStream(sampleVector,startPosition,MAXFILESTREAM); //close filestream

	//clear data
	Results.clear();
	vectorTempFields.clear();

	//recursively call til complete
	if((startPosition + MAXFILESTREAM) < sampleVector.size()){
		sampleVector.clear();
		cout<<"Re-Read(Recursive)"<<endl; 
		buildSNPFileFromList(fn,sampleFolder,chrName,(startPosition + MAXFILESTREAM)); 
	}

	cout<<"return from recursive"<<endl;
	return true;
}