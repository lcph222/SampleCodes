//this main contains methods and class i wrote over the year to do some sna-seq analysis. 

#include<iostream>
#include <stdlib.h>     /* system, NULL, EXIT_FAILURE */
#include<fstream>
#include<sstream>
#include"helper.h"
#include"SamStats.h"
#include<map>
#include <algorithm>
#include"SamAnalyze.h"
#include"indexFileParser.h"
#include"GTF.h"
#include"globals.h"
#include"Sam.h"
#include"SNP.h"
#include"vcfParser.h"
#include<fstream>
#include"gafParser.h"
#include<sstream>
//#include <windows.h>

using namespace std;

//vcf struct info
struct vcfFile{
	string Fn;
	string name;
};

//input: vcf file
//output: vector containing list of vcf file
bool loadVcfFileList(string fn, vector<vcfFile> & v){

	v.clear();

	ifstream fin(fn.c_str());

	if(!fin)
		return false;

	string line;
	while(getline(fin,line)){
		if(line.size() < 1)
			continue;
		std::istringstream ss(line);
		vcfFile tempLine;
		ss>>tempLine.Fn>>tempLine.name;
		v.push_back(tempLine);
	}

	fin.close();

	return true;
}

//input: file containing list of vcf files (path) and output folder
//output: snp files for each samples for each vcf file listed in input file 
bool builtVcfSNP(string fn, string sampleFolder){

	vector<vcfFile> fileList;
	
	if(!loadVcfFileList(fn,fileList)){
		cout<<"error loading vcfFileList"<<endl;
		return false;
	}
	vcfParser vcf;

	for(int i = 0; i < fileList.size(); i++){
		//vcf.buildSNPFile(fileList.at(i).Fn,sampleName+"_"+fileList.at(i).name,outputLocation1,outputLocation2);
		cout<<fileList.at(i).Fn<<endl;
		string cmd = "wc -l " + fileList.at(i).Fn;
		//system(cmd.c_str());
		vcf.buildSNPFileFromList(fileList.at(i).Fn, sampleFolder, fileList.at(i).name,0); //start at sample 1 (0)
	}


	//combine all result

}
//input: file to be sorted
//output: sorted file 
string sortFile(string file, string destination,string col,string name){

	//string column = std::to_string(long double (col));
	string outputName = destination + name + "-sorted.sam";
	//check if already exist
	
	
	ifstream fin(outputName.c_str());
	if(fin){
		fin.close(); //close it
		cout<<"found existing file "<<outputName<<endl;
		cout<<"using existing sorted file in output directory"<<endl;
		return outputName;
	}
	
	
	cout<<"begin sorting "<<file<<" on column"<<col<<endl;
	string cmd = "LC_ALL=C sort -V " +  file + " -o " + outputName;
	cout<<"command is: "<<cmd<<endl;
	system(cmd.c_str());
	cout<<"finished sorting "<<file<<endl;
	cout<<"new sorted file location is: "<<outputName<<endl;

	return outputName;
}

//input: index files which contains .Sam Files 
//output: sort each .Sam file (for later anlytical comparrasion)
void sortIndexFile(IndexFile &indexFile,string column,string des){

	for(int i = 0; i < indexFile.vectorIndexLine.size(); i++){
		string fn = indexFile.vectorIndexLine.at(i).fileLocation;
		string alignerName = indexFile.vectorIndexLine.at(i).alignerName;
		indexFile.vectorIndexLine.at(i).fileLocation = sortFile(fn,des,column,alignerName);
	}
}

//input: index file
//output create directory for each aligner listed in indexFile
void createDir(IndexFile indexFile){


	for(int i = 0; i < indexFile.vectorIndexLine.size(); i++){
		IndexLine indexLine = indexFile.vectorIndexLine.at(i);
		string localDir = indexLine.localPath;
		string outputDir = indexLine.OutptPath;
		/*
		//create dir
		if (CreateDirectory(localDir.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()){
			if (CreateDirectory(outputDir.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()){
			
			}
		}
		*/
		string sysCmd = "mkdir " + localDir;
		string sysCmd2 = "mkdir " + outputDir;
		system(sysCmd.c_str());
		system(sysCmd2.c_str());
	}
}

//createing new genomes with moddified SNPS
bool SNPWork(int argc, char * argv[]){

	if(argc != 6)
		return false;

	string fn = argv[1]; 
	string SNPFile = argv[2];
	string outputLocation = argv[3];
	string chrList = argv[4];
	
	int numberOfRead;
	Sam sam;
	sam.stringToInt(argv[5],numberOfRead);

	indexFileParser ifp(fn,outputLocation);
	ifp.setIndexVector();
	IndexFile indexFile = ifp.getIndexFileObject();

	//sort file
	if(!sorted)
		sortIndexFile(indexFile,"1",outputLocation);

	cout<<"creating subdirectory "<<endl;

	//creat subdirectory
	createDir(indexFile);

	SNP snpObject;
	cout<<"setting index file"<<endl;
	snpObject.setIndexFile(indexFile); //set and open input files
	cout<<"loading snp file"<<endl;
	snpObject.loadSNPFile(SNPFile,true); //hash it
	cout<<"Preforme compare ps-ref"<<endl;
	snpObject.Compare_PS_Ref_result(outputLocation,numberOfRead);
	cout<<"done..."<<endl;
	return true;
}

//extract SNP from VCF file
bool vcf2SNP(int argc, char *argv[]){

	cout<<"begin"<<endl;

	if(argc < 4)
		return false;

	string vcfFile = argv[2];
	string sampleFolder = argv[3];
	//string outputPath1 = argv[4];
	//string outputPath2 = argv[5];

	cout<<"vcFile:"<<vcfFile<<endl;

	//builtVcfSNP(vcfFile,outputPath1,outputPath2,name);
	builtVcfSNP(vcfFile,sampleFolder);

	return true;
}

//loading in .GAF file and calculate snp stat in transcriptome
bool gaf(int argc, char *argv[]){

	if(argc < 5)
		return false;

	string gafFile = argv[1];
	string snpFile = argv[2];
	string outputLocation = argv[3];
	string name = argv[4];

	gafParser gaf;
	cout<<"loading gaf file"<<endl;
	gaf.loadGafFile(gafFile);
	cout<<"done"<<endl;
	cout<<"creating exon map"<<endl;
	gaf.createExonMap();
	cout<<"done"<<endl;
	cout<<"create genome tree"<<endl;
	gaf.createGenomeTree();
	cout<<"done"<<endl;
	
	//loading snp
	SNP snpObject;
	snpObject.loadSNPFile(snpFile,false);
	vector<SNP_FIELD> snpVector = snpObject.getVectorSnpField();
	cout<<"adding snp file to exon map"<<endl;
	gaf.addSnpToExon(&snpVector);
	cout<<"calculate snp stats in transcriptome"<<endl;
	gaf.CalculateSnpInTranStat(outputLocation,name);
	cout<<"done"<<endl;
	return true;
}

//parse only SNP from transcriptome after loading ing GAF file
bool parseSNP(int argc, char *argv[]){

	if(argc < 6)
		return false;

	string gafFile = argv[2];
	string snpFile = argv[3];
	string outputLocation = argv[4];
	string name = argv[5];

	
	gafParser gaf;
	cout<<"loading gaf file"<<endl;
	gaf.loadGafFile(gafFile);
	cout<<"done"<<endl;
	cout<<"creating exon map"<<endl;
	gaf.createExonMap();
	cout<<"done"<<endl;
	cout<<"create genome tree"<<endl;
	gaf.createGenomeTree();
	cout<<"done"<<endl;

	//loading snp
	SNP snpObject;
	snpObject.loadSNPFile(snpFile,false);
	vector<SNP_FIELD> snpVector = snpObject.getVectorSnpField();
	gaf.ParseOnlySNPInTranscriptome(outputLocation,name,&snpVector);

}

//load a list of string in a file to a vector
bool loadListToVector(string fn, vector<string> &v){

	v.clear();

	ifstream fin(fn.c_str());

	if(!fin)
		return false;

	string line;

	while(getline(fin,line)){
	
		if(line.size() < 1)
			continue;

		string token;
		istringstream is(line);

		is>>token; //only get the first one

		v.push_back(token);
	}


	fin.close();

	return true;
}

//combine all the snp files (e.g. chr1-chrY) to a single file
bool combineSnpFile(string fn, string samplePath){

	vector<string> v;
	if(!loadListToVector(fn,v)){
		cout<<"error loading "<<fn<<endl;
		return false;
	}

	for(int i = 0; i < v.size(); i++){ 
		string path1 = samplePath + v.at(i) + "/SNPFiles/genome1/*.txt ";
		string path2 = samplePath + v.at(i) + "/SNPFiles/genome2/*.txt ";
		string cmd1 = "cat " + path1 + " > " + samplePath + v.at(i) + "/SNPFiles/genome1/totalSNP.txt";
		string cmd2 = "cat " + path2 + " > " + samplePath + v.at(i) + "/SNPFiles/genome2/totalSNP.txt";
		cout<<"command 1: "<<cmd1<<endl;
		cout<<"command 2: "<<cmd2<<endl;
		system(cmd1.c_str());
		system(cmd2.c_str());
	}

	return true;
}

//build dir for ps stuff, 
void buildDir(string location, vector<string> &v){


	for(int i = 0; i < v.size(); i++){
	
		string cmd = "mkdir ";
		string sampleDir = cmd + location + v.at(i);
		//string psDir = sampleDir + "/PersonalGenome1";
		//string psDir2 = sampleDir + "/PersonalGenome2";
		string snpSubDir = sampleDir + "/SNPFiles";
		string snpSubDirGenome = snpSubDir + "/genome1";
		string snpSubDirGenom2 = snpSubDir + "/genome2";

		system(sampleDir.c_str()); //creat dir for sample
		//system(psDir.c_str()); //creat dir for sample
		//system(psDir2.c_str()); //creat dir for sample
		system(snpSubDir.c_str()); //creat dir for sample
		system(snpSubDirGenome .c_str()); //creat dir for sample
		system(snpSubDirGenom2.c_str()); //creat dir for sample
	
	}

}

//creat directory for personal genome
bool createDirForPS(string location, string fn){

	vector<string> v; 
	
	if(!loadListToVector(fn,v))
		return false;

	buildDir(location,v);

	return true;
}

//create sub directory for personal snp
bool createPSG(string sampleListFile, string chrListFile, string chrmPath,string sampleFolder){

	vector<string> sampleList;

	if(!loadListToVector(sampleListFile,sampleList))
		return false;

	for(int i = 0; i < sampleList.size(); i++){
	
		cout<<"working on "<<sampleList.at(i)<<endl;

		string snpFile1 = sampleFolder + sampleList.at(i) + "/SNPFiles/genome1/totalSNP.txt";
		string snpFile2 = sampleFolder + sampleList.at(i) + "/SNPFiles/genome2/totalSNP.txt";
		string psgPath1 = sampleFolder + sampleList.at(i)+ "/PersonalGenome1/";
		string psgPath2 = sampleFolder + sampleList.at(i)+ "/PersonalGenome2/";

		SNP snpObject;
		if(!snpObject.loadSNPFile(snpFile1,true)){
			cout<<"error on "<<sampleList.at(i)<<" snp file"<<endl;
			continue;
		}

		cout<<"building genome 1"<<endl;
		Sam sam;
		ReferenceGenomeSequenceObj ref;
		sam.loadAllChromosome(chrmPath,chrListFile,ref);
		cout<<"genome loaded"<<endl;
		cout<<"Size is "<<ref.chromosomeMap.size()<<endl;
		snpObject.updateGenomeWithSNP(ref);
		snpObject.outputReferenceGenome(ref,psgPath1);
		//clear space
		//ref.~ReferenceGenomeSequenceObj();
		

		SNP snpObject2;
		if(!snpObject2.loadSNPFile(snpFile2,true)){
			cout<<"error on "<<sampleList.at(i)<<" snp file"<<endl;
			continue;
		}

		cout<<"building genome 2"<<endl;
		Sam sam2;
		ReferenceGenomeSequenceObj ref2;
		sam2.loadAllChromosome(chrmPath,chrListFile,ref2);
		cout<<"genome loaded 2"<<endl;
		cout<<"Size is "<<ref.chromosomeMap.size()<<endl;
		snpObject2.updateGenomeWithSNP(ref2);
		snpObject2.outputReferenceGenome(ref2,psgPath2);

		cout<<"done for "<<sampleList.at(i)<<endl;

	}

	return true;
}

//find and output "SNP CLUSTER" from gaf file
bool snpCluster(int argc, char * argv[]){

	if(argc < 6)
		return false;

	string gafFile = argv[2];
	string snpFile = argv[3];
	string outputLocation = argv[4];
	string name = argv[5];

	gafParser gaf;
	cout<<"loading gaf file"<<endl;
	gaf.loadGafFile(gafFile);
	cout<<"done"<<endl;
	cout<<"creating exon map"<<endl;
	gaf.createExonMap();
	cout<<"done"<<endl;
	cout<<"create genome tree"<<endl;
	gaf.createGenomeTree();
	cout<<"done"<<endl;
	
	//loading snp
	SNP snpObject;
	snpObject.loadSNPFile(snpFile,false);
	vector<SNP_FIELD> snpVector = snpObject.getVectorSnpField();
	cout<<"adding snp file to exon map"<<endl;
	gaf.addSnpToExon(&snpVector);
	cout<<"building snp cluster"<<endl;
	gaf.BuildClusterSnpFromTranscripts(outputLocation,name);

}

//string outputLocation, string prefix, int start, int end
//comapre two alignment file (General statistic in SamAnalyze class)
void compare2AlignmentFile(int argc, char * argv[]){

	if(argc < 5)
		return;
	string fileList = argv[2];
	string outputLocation = argv[3];
	string outputDebug = argv[4];

	bool outputDebugFile;
	if(outputDebug == "yes")
		outputDebugFile = true;
	else
		outputDebugFile = false;
	
	indexFileParser ifp(fileList,outputLocation);
	ifp.setIndexVector();
	IndexFile indexFile = ifp.getIndexFileObject();

	//sort file
	if(!sorted)
		sortIndexFile(indexFile,"1",outputLocation);

	SamAnalyze analyzeObj;
	analyzeObj.setIndexFile(indexFile);
	analyzeObj.NewMethodComparasion(outputLocation,outputDebugFile);
}

//input: many alignments files from many different aligners
//output: a combine result (which try to select the best alignment for each read)
//output: should have a high accuracy when compare to individual aligner's result. 
void combineRnaSeqAl(int argc, char * argv[]){

	if(argc < 12)
		return;

	Sam sam;

	//get indexFile for fn in argv[1]
	string fn = argv[2]; 
	string outputLocation = argv[3];
	string annotation = argv[4];
	string chrmPath = argv[5];
	string chrList = argv[6];
	string prefix = argv[7];
	int firstSeq = stringToInt(argv[8]);
	int lastSeq = stringToInt(argv[9]);
	int numberOfRead = stringToInt(argv[10]);
	string redoPhase1 = argv[11];
	
	//bulding index
	indexFileParser ifp(fn,outputLocation);
	ifp.setIndexVector();
	IndexFile indexFile = ifp.getIndexFileObject();
	
	
	//gtf
	//GTF gtfObject(annotation,chrmPath,chrList);
	//gtfObject.setGenome(); //create splice graph
	
	//sort file
	if(!sorted){
		sortIndexFile(indexFile,"1",outputLocation);
		return;
	}

	cout<<"creating subdirectory "<<endl;

	//creat subdirectory
	createDir(indexFile);

	//testing
	//load chromosme
	ReferenceGenomeSequenceObj ref;
	sam.loadAllChromosome(chrmPath,chrList,ref);
	cout<<"genome loaded"<<endl;
	cout<<"Size is "<<ref.chromosomeMap.size()<<endl;

	//creat-sub directory
	cout<<"here "<<endl;
	SamAnalyze SA(indexFile,annotation,chrmPath,chrList,numberOfRead,&ref); //set indexFile
	
	cout<<"here 2"<<endl;
	string statsDir = outputLocation + "chromosomeStats/";
	string cmd = "mkdir " + statsDir;
	system(cmd.c_str());
	//SA.AnalyzeAlignmentsStats(statsDir);


	//loading stats
	string suffix = "_Stats.data";
	if(redoPhase1 != "YES" && redoPhase1 != "yes"){

		SA.loadChrStats(statsDir,suffix);
		//loading S1 files
	
		if(!SA.loadQnameS1(statsDir)){
			cout<<"error loading Qname S1 file"<<endl;
			return;
		}
	}

	//combine results
	//SA.combineSamResults(outputLocation); //return false if error
	if(redoPhase1 == "YES" || redoPhase1 == "yes")
		SA.phase1(statsDir,numberOfRead,true,prefix,firstSeq,lastSeq);
	
	if(redoPhase1 == "NO" || redoPhase1 == "no")
		SA.phase1(outputLocation,numberOfRead,false,prefix,firstSeq,lastSeq);
	system("pause");

}

int main(int argc, char *argv[]){
	
	if(argc <= 1){
		return 0;
	}

	string command = argv[1];

	if(command == "--compare2AlignmentFile"){
		compare2AlignmentFile(argc,argv);
	}


	if(command == "--snpCluster"){
		snpCluster(argc,argv);
		return 0;
	}

	if(command == "--combineSNP"){
		if(argc < 4)
			return 1;
		string fileList = argv[2];
		string snpPath = argv[3];
		combineSnpFile(fileList,snpPath);
		return 0; 
	}

	//gaf(argc,argv);
	if(command == "--parseSNP"){
		parseSNP(argc,argv);
		return 1;
	}

	if(command == "--vcf2snp"){
		vcf2SNP(argc,argv);
		return 1;
	}
//	SNPWork(argc,argv);
//		return 1;

	if(command == "--buildPSGDir"){
		string fn = argv[2];
		string location = argv[3];
		createDirForPS(location,fn);
	}

	if(command == "--buildPSG"){
		cout<<"building personal reference genome"<<endl;
		if(argc < 6){
			cout<<"error auguements"<<endl;
			return 1;
		}
		createPSG(argv[2],argv[3],argv[4],argv[5]);
		/*
		string SNPFile = argv[2];
		string outputLocation = argv[3];
		string chrList = argv[4];
		string chrmPath = argv[5];


		SNP snpObject;
		if(!snpObject.loadSNPFile(SNPFile,true))
			return 1;
		
		Sam sam;
		ReferenceGenomeSequenceObj ref;
		sam.loadAllChromosome(chrmPath,chrList,ref);
		cout<<"genome loaded"<<endl;
		cout<<"Size is "<<ref.chromosomeMap.size()<<endl;
		snpObject.updateGenomeWithSNP(ref);
		snpObject.outputReferenceGenome(ref,outputLocation);

		cout<<"done"<<endl; */
		return 0;
	}

	if(command == "--combineRnaSeqAl"){
		combineRnaSeqAl(argc,argv);
		return 0;
	}


	return 0;
}