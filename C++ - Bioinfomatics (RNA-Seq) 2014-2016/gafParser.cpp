#include"gafParser.h"
#include<fstream>
#include"globals.h"
#include<math.h>
#include <algorithm>

const int SEGMENT_SIZE = 30;
const int LENGTH_SEARCH = 100;


//0-based or 1-based
void parseExonPos(vector<int> & v, int offset, string s){
	v.clear();
	istringstream is(s);
	string token;
	while(getline(is,token,',')){
		int value = stringToInt(token) + offset;
		v.push_back(value);
	}

}
//open gaf and load file to RAM
bool gafParser::loadGafFile(string fn){

	ifstream fin(fn.c_str());

	if(!fin){
		cout<<"error openign "<<fn<<endl;
		return false;
	}

	//clear
	this->vectorGafField.clear();

	string line;

	while(getline(fin,line)){
	
		if(line.size() < 2)
			continue;


		int field = 1;
		istringstream is(line);
		string token;
		gafField temp;

		if(!test){ //if running on linux so \t delimiter
			while(field <= 12 && std::getline(is,token,'\t')){
		
				switch(field){
				
				case 1: temp.geneId = token; break;
				case 2: temp.chrName = token; break;
				case 3: temp.strand = token.at(0); break;
				case 4: temp.field4 = stringToInt(token); break;
				case 5: temp.field5 = stringToInt(token); break;
				case 6: temp.field6 = stringToInt(token); break;
				case 7: temp.field7 = stringToInt(token); break;
				case 8: temp.numExon = stringToInt(token); break;
				case 9: parseExonPos(temp.exonStartPos,0,token); break;
				case 10: parseExonPos(temp.exonEndPos,-1,token); break;
				case 11: temp.field11 = token; break;
				case 12: temp.field12 = token; break;
				}//switch
				field++;
			}
		}else{ //else on window or non tab delimiter 
			is>>temp.geneId>>temp.chrName>>temp.strand>>temp.field4>>temp.field5>>temp.field6>>temp.field7>>temp.numExon;
			is>>token; 
			parseExonPos(temp.exonStartPos,0,token);
			is>>token; 
			parseExonPos(temp.exonEndPos,-1,token);
			is>>temp.field11>>temp.field12;
		}
		
		if(!test && field != 13){
			cout<<"field is "<<field<<endl;
			cout<<"GAF line didn't have 13 fields"<<endl;
			cout<<line<<endl;
			continue;
		}

		this->vectorGafField.push_back(temp);

	}//while

	fin.close();
}


//return vecotr of each line
vector<gafField> * gafParser::getVectorGafField(){
	return &this->vectorGafField;
}


//create tree structure from exons
void gafParser::createGenomeTree(){

	unordered_map<string, map< std::pair<int,int>, exon  *> > ::iterator it;

	for(it = this->chrm2exonMap.begin(); it != this->chrm2exonMap.end(); it++){
	
		string chrName = it->first; //get chrmName
		//create map
		IntervalTree<exon*> * tree = NULL;
		creatExonIntervalTree(tree,it->second);
		this->genomeTree[chrName] = tree;
	}

}


//create interval tree
void gafParser::creatExonIntervalTree(IntervalTree<exon*> * & tree, map<std::pair<int,int>,exon *> & exonMap){

	map< std::pair<int,int>, exon *>::iterator it;

	vector< Interval<exon*> > intervals;

	for(it = exonMap.begin(); it != exonMap.end(); it++){
	
		intervals.push_back(Interval<exon*>(it->first.first,it->first.second, it->second));

	}

	tree = new IntervalTree<exon*>(intervals);


}

//find exon time, using tree structure created in above function 
exon * findExon( unordered_map<string, map< std::pair<int,int>, exon * > > & chrm2exonMap, string chrName, std::pair<int,int> p){

	unordered_map<string, map< std::pair<int,int>, exon * > >::iterator it;
	it = chrm2exonMap.find(chrName);

	if(it == chrm2exonMap.end())
		return NULL;

	map< std::pair<int,int>, exon * >::iterator it2;
	it2 = it->second.find(p);
	if(it2 == (it->second).end())
		return NULL;

	return it2->second;
}

//unordered_map<string, unordered_map<int, vector<transcript> >> transcriptMap;
//built exon and transcript, exon is created once and is stored by pointer
void gafParser::createExonMap(){ //create exon and built transcript map

	//interval containing exon
	//vector< Interval <exon*> > intervals;

	for(int i = 0; i < this->vectorGafField.size(); i++){
	
		transcript trans;
		trans.strand = this->vectorGafField.at(i).strand; //set trans
		string chrName = vectorGafField.at(i).chrName;
		trans.chrName = chrName;
		trans.Id = this->vectorGafField.at(i).geneId;

		//set transcript start an end based on strand
		if(trans.strand == '+'){
			trans.start = this->vectorGafField.at(i).exonStartPos.front(); //starting pos
			trans.end = this->vectorGafField.at(i).exonEndPos.back();
		}else{ //reverse for negative strand
			trans.start = this->vectorGafField.at(i).exonStartPos.back(); 
			trans.end = this->vectorGafField.at(i).exonEndPos.front(); 
		}

		
		//extract transcript
		for(int j = 0; j < vectorGafField.at(i).exonStartPos.size(); j++){
			
			bool newExon = false;
			std::pair<int,int> p;
			p.first = this->vectorGafField.at(i).exonStartPos.at(j);
			p.second = this->vectorGafField.at(i).exonEndPos.at(j);
			//find exon
			//create new if not exist
			exon * e = findExon(this->chrm2exonMap,chrName,p);

			//create new exon if not exist
			if(e == NULL){
				newExon = true; 
				e = new exon();
				e->exonPos = p;
			}

			//add to trans
			trans.exonsVector.push_back(e);

			//find iter
			unordered_map< string, map< std::pair<int,int>, exon *> > ::iterator it; //parrallel maps
			
			//find chrName
			it = this->chrm2exonMap.find(chrName);

			
			//found chromosome
			if(it != this->chrm2exonMap.end()){
				if(newExon) //only add if new exon
					(it->second)[p] = e;
			}else{ //new chromosome, new exon
				map< std::pair<int,int>, exon *> tempMap;
				tempMap[p] = e;
				this->chrm2exonMap[chrName] = tempMap;
			}

			//intervals.push_back(Interval<exon*>(e.exonPos.first,e.exonPos.second,&e));

		}

		unordered_map<string, unordered_map<int, vector<transcript> >>::iterator transIt;
		transIt = this->transcriptMap.find(chrName);
		//found chromosome
		if(transIt != this->transcriptMap.end()){ 
			unordered_map<int, vector<transcript> >::iterator transLev2It;
			transLev2It = (transIt->second).find(trans.start);
			
			//found same startPos
			if(transLev2It != (transIt->second).end()){
				(transLev2It->second).push_back(trans); //add to vector
			}else{ //new transcript
				vector<transcript> newVectorTrans;
				newVectorTrans.push_back(trans);
				(transIt->second)[trans.start] = newVectorTrans;
			}

		}else{
			//build map
			unordered_map<int, vector<transcript> > tempTransMap;
			vector<transcript> tempVectorTrans;
			tempVectorTrans.push_back(trans);
			tempTransMap[trans.start] = tempVectorTrans;
			this->transcriptMap[chrName] = tempTransMap; 
		}

	}

}

//add a snp to an exon (snp in exon's range)
void gafParser::addSnpToExon(vector<SNP_FIELD> * v){

	//go through each snp

	for(int i = 0; i < v->size(); i++){
		//tree is in 0-based position
		vector<Interval<exon*> > exonNodes;
		unordered_map<string, IntervalTree<exon*> *> ::iterator it;
		it = this->genomeTree.find(v->at(i).chr);
		//can't find
		if(it == this->genomeTree.end())
			continue;

		IntervalTree<exon*> * tree = it->second;

		int snpPos = v->at(i).pos; //0-based position
		tree->findOverlapping(snpPos,snpPos, exonNodes);
		for(int j = 0; j < exonNodes.size(); j++){
			bool exist = false;
			//check if already set
			if(exonNodes.at(j).value->snpPos.size() > 0){
				if(find(exonNodes.at(j).value->snpPos.begin(),exonNodes.at(j).value->snpPos.end(),snpPos) == exonNodes.at(j).value->snpPos.end())
					exist = false;
				else
					exist = true;
			}

			if(!exist){ //new snp 
				exonNodes.at(j).value->snp = true;
				exonNodes.at(j).value->snpNum++; //add anp position to exon
				exonNodes.at(j).value->snpPos.push_back(snpPos); //0-based
				exonNodes.at(j).value->SNPS.push_back(v->at(i)); //push back node
			}//if

		}//end for loop
		if(exonNodes.size() > 0)
			results.numberSnpInExon++;
	}

	//set total snp
	results.totalSnp = v->size();
}

//built a hash map of snpss
void gafParser::buildSnpMap(vector<SNP_FIELD> * v){

	for(int i = 0; i < v->size(); i++){
	
		string chrName = v->at(i).chr;
		unordered_map<string, unordered_map<int,snpStats> >::iterator it;
		it = this->snpMap.find(chrName);
		
		snpStats tempStat;

		if(it != this->snpMap.end()){ //found existing
			(it->second)[v->at(i).pos] = tempStat;
		}else{
			unordered_map<int,snpStats> tempMap;
			tempMap[v->at(i).pos] = tempStat;
			this->snpMap[chrName] = tempMap;
		}
	}
}

//calcualte stats about snp in transcriptome
void gafParser::CalculateSnpInTranStat(string outputLocation,string name){

	//go through each transcript

	bool positiveStrand;

	for(int i = 0; i < this->vectorGafField.size(); i++){
		
	
		cout<<"transcriptome: "<<i<<endl;
		cout<<this->vectorGafField.at(i).strand<<endl;

		if(this->vectorGafField.at(i).strand == '+')
			positiveStrand = true;
		else
			positiveStrand = false;

		vector<int> snpPos;
		snpPos.clear();

		int spliceJunctionSize = 0;

		//for each exon
		for(int j = 0; j < this->vectorGafField.at(i).numExon; j++){
		
			string chrName = this->vectorGafField.at(i).chrName;
			std::pair<int,int> exonPos;
			exonPos.first = this->vectorGafField.at(i).exonStartPos.at(j);
			exonPos.second = this->vectorGafField.at(i).exonEndPos.at(j);
			
			unordered_map<string , map< std::pair<int,int> , exon *> >::iterator it;
			it = this->chrm2exonMap.find(chrName);

			if(it == this->chrm2exonMap.end())
				continue;

			cout<<"Exon: "<<exonPos.first<<"-"<<exonPos.second<<endl;

			//get map
			exon * iterExon = NULL;
			iterExon =  ((it->second)[exonPos]);
			
			//copy it, so operation wont mess with it
			vector<int> copySnpPos;
			if(iterExon->snpNum > 0)
				copySnpPos = iterExon->snpPos;

			if( j > 0){ //only when there's more than one exon
				if(positiveStrand)
					spliceJunctionSize += exonPos.first - this->vectorGafField.at(i).exonEndPos.at(j - 1) - 1;
				else
					spliceJunctionSize += this->vectorGafField.at(i).exonStartPos.at(j - 1) - exonPos.second - 1;
				
				for(int k = 0; k < copySnpPos.size(); k++){
					if(positiveStrand)
						copySnpPos.at(k) -= spliceJunctionSize;
					else
						copySnpPos.at(k) += spliceJunctionSize;
				} 
			}
			
			cout<<"spliceJunctionSize "<<spliceJunctionSize <<endl;
			
			if(iterExon->snpNum > 0){
				cout<<"NumSnp = "<<iterExon->snpNum<<endl;
				for(int n = 0; n < iterExon->snpNum; n++)
					cout<<iterExon->snpPos.at(n)<<endl;
				snpPos.insert(snpPos.end(),copySnpPos.begin(),copySnpPos.end());
			}
		}
	
		if(snpPos.size() > 1){
			results.numTranMoreThanOneSnp++;
		
			//sort the vector
			sort(snpPos.begin(),snpPos.end());

			cout<<"snp pos: "<<endl;
			//calcualte distance,, should be in order
			for(int l = 0; l < snpPos.size(); l++){
				cout<<snpPos.at(l)<<endl;
			}

			
			for(int m = 1; m < snpPos.size(); m++){
				int dist = abs(snpPos.at(m) - snpPos.at(m - 1));
				cout<<snpPos.at(m)<<" - "<<snpPos.at(m - 1)<<" = "<<dist<<endl;
				results.distMap[dist]++;
				if(dist == 0)
					cout<<"dist 0 : "<<i<<endl;
			}
		
		}//if

		results.numSnpInTran[snpPos.size()]++;
		cout<<"-------------------------------------"<<endl;

	}//for

	//output
	this->results.outputSNP(outputLocation,name);
	this->results.outputDistMap(outputLocation,name);
	this->results.outputNumSnpInTran(outputLocation,name);

}


//parse snp that exists in transcrimptome only and output to file
void gafParser::ParseOnlySNPInTranscriptome(string outputLocation,string name,vector<SNP_FIELD> * v){

	string fn = outputLocation + name + "_SNPInTrans.txt";
	ofstream fout(fn.c_str());

	for(int i = 0; i < v->size(); i++){
		
		vector<Interval<exon*> > exonNodes;
		unordered_map<string, IntervalTree<exon*> *> ::iterator it;
		it = this->genomeTree.find(v->at(i).chr);
		
		//can't find chromosome
		if(it == this->genomeTree.end())
			continue;
		IntervalTree<exon*> * tree = it->second;
		int snpPos = v->at(i).pos; //0-based position
		tree->findOverlapping(snpPos,snpPos, exonNodes);

		//snp is in transcriptome
		if(exonNodes.size() > 0){ //move to 1 position
			fout<<v->at(i).chr<<"\t"<<v->at(i).pos + 1<<"\t"<<v->at(i).refChar<<"->"<<v->at(i).perChar<<endl;
		}
			
	}

	fout.close();
}


//return a vector relative to the first position of exon
//first value is the starting point of exon
//last value is the last point of exon
//between are the snp position
//return false if there's no snp
bool getRelativeSnpPos(vector<exon *> * Exons, vector<snp_sj_info> & result){

	result.clear();
	//push back first position
	snp_sj_info first;
	first.pos = Exons->front()->exonPos.first;
	first.relPos = 0;

	result.push_back(first);
	
	int spliceJunctionSize = 0;

	for(int i = 0; i < Exons->size(); i++){
	
		//when moving to the next exon
		if(i > 0){ //calculate splice junction size
			spliceJunctionSize += Exons->at(i)->exonPos.first - Exons->at(i - 1)->exonPos.second - 1;
		}

		//get all the snp pos
		vector<int> * snpPos = &Exons->at(i)->snpPos;
		for(int snpIt = 0; snpIt < snpPos->size(); snpIt++){ //for each snp, calculate it's position relative to the first one
			snp_sj_info snpNode;
			snpNode.snp = true;
			snpNode.pos = snpPos->at(snpIt);
			snpNode.relPos = snpNode.pos - first.pos - spliceJunctionSize; //remove splice junction size
			snpNode.snpObj = Exons->at(i)->SNPS.at(snpIt); //parrallel vector
			result.push_back(snpNode); //push back into results
		}
	}

	//push back last position, relative to the first one
	snp_sj_info last;
	last.pos = Exons->back()->exonPos.second;
	last.relPos = last.pos - first.pos - spliceJunctionSize;
	result.push_back(last); //push back last point

	if(result.size() < 3)
		return false;

	return true;
}


//the first and last position are exon start and ends
//the middle are snp position + splice-junction accepter/donor
//return false if there's no snp
//all position are relative to the first for relative position, position is its origianl position
bool getRelativeSnpPosIncludeSpliceJunction(vector<exon *> *Exons, vector<snp_sj_info> & result, char strand){

	result.clear();
	snp_sj_info first;
	first.pos = Exons->front()->exonPos.first;
	first.relPos = 0; //relative position = first position
	//push back first position
	result.push_back(first);

	int spliceJunctionSize = 0;
	int numberOfSnp = 0;
	for(int i = 0; i < Exons->size(); i++){
	
		//when moving to the next exon
		if(i > 0){ //calculate splice junction size
			snp_sj_info sjNode;
			int oldSpliceSize = spliceJunctionSize;
			//update
			spliceJunctionSize += Exons->at(i)->exonPos.first - Exons->at(i - 1)->exonPos.second - 2; //including donor/or acceptor site
			if(strand == '+'){ //add donor site
				sjNode.sj_donor = true;
				sjNode.pos = Exons->at(i - 1)->exonPos.second + 1; 
				//position = sj-donor - startPos(exon) - old_SJ_size
				sjNode.relPos = sjNode.pos  - first.pos - oldSpliceSize; //minus old size
			}else{//add acceptor site
				sjNode.sj_acc = true;
				sjNode.pos = Exons->at(i)->exonPos.first - 1; 
				sjNode.relPos = sjNode.pos - first.pos - spliceJunctionSize; //minus current size
			}
			//add to list
			result.push_back(sjNode); //add sjNode to result
		}

		//get all the snp pos
		vector<int> * snpPos = &Exons->at(i)->snpPos;
		numberOfSnp += snpPos->size(); //count number of snp exist in Exons vector

		for(int snpIt = 0; snpIt < snpPos->size(); snpIt++){ //for each snp, calculate it's position relative to the first one
			snp_sj_info snpNode;
			snpNode.snp = true;
			snpNode.pos = snpPos->at(snpIt);
			snpNode.relPos = snpNode.pos - first.pos - spliceJunctionSize; //remove splice junction size
			snpNode.snpObj = Exons->at(i)->SNPS.at(snpIt); //parrallel vector
			result.push_back(snpNode); //push back into results
		}

	}

	//add last point
	snp_sj_info last;
	last.pos = Exons->back()->exonPos.second;
	last.relPos = last.pos - first.pos - spliceJunctionSize;

	if(numberOfSnp < 1)
		return false;

	return true;
}

//return true if found segment greater than >= segmentLength w/o sj or snp
//first and last of v is trans position (start,end)
//v = 0,4,6,8,etc...
//first position is starting position
//rest are number of bases away
bool serchForSegmentLeft(vector<snp_sj_info> & v, int startPos, int segmentLength,int RemaindingBases){


	//this function will stop at i = 0, hench no exceeding exon start position
	for(int i = startPos; i > 0; i--){ 

		int currPos = v.at(i).relPos;
		int leftPos;
		
		leftPos = v.at(i -  1).relPos;
		
		int currSegLen;
		if(i == 1) //count the first base of exon
			currSegLen = currPos - leftPos;
		else//minus 1 for snp or sj
			currSegLen = currPos - leftPos - 1;//segment between two snp or snp and sj
		
		if(currSegLen > RemaindingBases){
			if(RemaindingBases >= segmentLength) //remainding bases will determine
				return true;
			else
				return false;
		}else{
			RemaindingBases -= (currSegLen + 1); //update remainding bases to serach
			if(currSegLen >= segmentLength)
				return true;
			else
				continue; 
		}
	}

	return false;
}

bool serchForSegmentRight(vector<snp_sj_info> & v, int startPos, int segmentLength,int RemaindingBases){


	//this function will stop at i = 0, hench no exceeding exon start position
	for(int i = startPos; i < (v.size() - 1); i++){
		int currPos = v.at(i).relPos;
		int rightPos;
		rightPos = v.at(i +  1).relPos;

		int currSegLen;
		if((i + 1) == (v.size() - 1 )) //count the first base of exon
			currSegLen = rightPos - currPos;
		else 
			currSegLen = rightPos - currPos - 1;//segment between two snp or snp and sj
		
		if(currSegLen > RemaindingBases){
			if(RemaindingBases >= segmentLength) //remainding bases will determine
				return true;
			else
				return false;
		}else{
			RemaindingBases -= (currSegLen + 1); //update remainding bases to serach, also count the current base
			if(currSegLen >= segmentLength)
				return true;
			else
				continue; 
		}
	}

	return false;
}

//first and last of v is exon begin and end position
bool parsedSnp(vector<snp_sj_info> & v, vector<SNP_FIELD> & result){

	result.clear();
	if(v.size() < 3)
		return false;

	//for each snp
	for(int i = 1; i < (v.size() - 1); i++){
		
		if(!v.at(i).snp)
			continue;

		//only for snp
		if(serchForSegmentLeft(v,i,SEGMENT_SIZE,LENGTH_SEARCH)){
			result.push_back(v.at(i).snpObj); //push back snp object
		}else if(serchForSegmentRight(v,i,SEGMENT_SIZE,LENGTH_SEARCH)){ //serach for right side
			result.push_back(v.at(i).snpObj); //push back snp object
		}

	}

	if(result.size() > 0)
		return true;

	return false;
}

//return vector of snp position but distance is realtive to the first one
//return result, empty means does not exist
void snpClustering(transcript * trans, vector<SNP_FIELD> & vectorResultClusterSNP, vector<SNP_FIELD> & vectorResultClusterSNPSJ){

	vectorResultClusterSNP.clear();
	vectorResultClusterSNPSJ.clear();

	//iterate through each exon, make a copy
	vector<exon *> exonVector = trans->exonsVector;

	if(trans->strand == '-') //reverse, now they're in normal order left -> right
		std::reverse(exonVector.begin(),exonVector.end());

	vector<snp_sj_info> relativeSnpPosVector; 
	vector<snp_sj_info> relativeSnpSJPosVector;

	//get relativeposition vector or snp + exon
	if(getRelativeSnpPos(&exonVector,relativeSnpPosVector)){
		parsedSnp(relativeSnpPosVector,vectorResultClusterSNP);
			//found snp cluster		
	}

	//get relativePosition vector for snp + exon + sj
	if(getRelativeSnpPosIncludeSpliceJunction(&exonVector,relativeSnpSJPosVector,trans->strand)){
		parsedSnp(relativeSnpSJPosVector,vectorResultClusterSNPSJ);
			//found snp + sj cluster 		
	}

}

void outputSnpVector(ofstream & fout, vector<SNP_FIELD> & v,transcript * t){

	string tab = "\t";

	for(int i = 0; i < v.size(); i++){
		//snp info
		fout<<v.at(i).chr<<tab<<v.at(i).pos + 1<<tab<<v.at(i).refChar<<"->"<<v.at(i).perChar<<tab;
		fout<<t->Id<<tab<<t->strand<<tab;
		for(int j = 0; j < t->exonsVector.size(); j++){
			fout<<t->exonsVector.at(j)->exonPos.first + 1<<"-"<<t->exonsVector.at(j)->exonPos.second + 1<<",";
		}
		fout<<endl;
	}

}


void gafParser::BuildClusterSnpFromTranscripts(string outputLocation, string name){

	string fn1 = outputLocation + name + "_snpCluster.txt";
	string fn2 = outputLocation + name + "_snpSJCluster.txt";

	ofstream fout(fn1.c_str());
	ofstream fout2(fn2.c_str());



	//header
	fout<<"# SegmentSize"<<SEGMENT_SIZE<<" SerachLength(Left and right) "<<LENGTH_SEARCH<<endl;

	unordered_map<string, unordered_map<int, vector<transcript> > > ::iterator it;

	//for each chromosome
	for(it = this->transcriptMap.begin(); it != this->transcriptMap.end(); it++){
	
		unordered_map<int, vector<transcript> >::iterator it2;
	
		for(it2 = (it->second).begin(); it2 != (it->second).end(); it2++){ //for each set of transcript
		
			for(int i = 0; i < (it2->second).size(); i++){ //for each transcript
			
				//get a list of snp position
				vector<SNP_FIELD> snpCluster, snpSjCluster;
				snpClustering(&(it2->second).at(i),snpCluster,snpSjCluster); //get result
				outputSnpVector(fout,snpCluster,&(it2->second).at(i));
				outputSnpVector(fout2,snpSjCluster,&(it2->second).at(i));
				//output results

			}//3rd for

		}//2nd for
	
	}//1st for

	fout.close();
	fout2.close();

}