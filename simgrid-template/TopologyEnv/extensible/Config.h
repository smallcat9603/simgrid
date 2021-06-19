/*
 * Config.h
 *
 *  Created on: Sep 14, 2014
 *      Author: chaix
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "Matrix.h"
#include "Parameters.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#define CABLE_COST

typedef long int Val;
typedef Matrix<Val> Mat;
typedef Matrix<float> FloatMat;

struct NodeInfo{
	unsigned int nativeId;
	unsigned int finalId;
	Position position;
	Position index;
	unsigned int year;
		
};

		
template<typename T> void writeVariable(std::string variableName, T value, std::ostream& out){
	out<<"\tres[\""<<variableName<<"\"]="<<value<<";\n";	
}
template<typename T> void writeVariable(std::string variableName, Matrix<T> value, std::ostream& out){
	out<<"\tres[\""<<variableName<<"\"]=";
	bool multiline=(value.getRows()>1);
	if(multiline)
		out<<"[";
	for(unsigned int r=0;r<value.getRows();r++)
	{
		out<<"[";
		for(unsigned int c=0;c<value.getCols();c++)
		{
			out<<value(r,c)<<(c+1<value.getCols()?",\t":" ]");
		}
		if(multiline)
			out<<(r+1<value.getRows()?",\n\t":"];");		 	
	}		 	
	out<<"\n";	
}
	

std::vector<std::string> readConfigurationOptions(std::string filename,std::string keyword);

struct Topology {
	//Attributes defining the topology
	//Years start from  0 
	unsigned int year;
	float probability;
	std::string name;
	//Only used for saving to files
	Mat nativeConnectivity;
	//Used for computation (index of nodes correspond to final year indexes)
	Mat finalConnectivity;
	

	//Attributes used for simplifying optimization process
	Mat permutation;
	FloatMat dCost;
	FloatMat dEnergy;

	void read(std::istream& in) {
		in >> year >> probability>>name;
		nativeConnectivity.fromSquareStream(in);
		nativeConnectivity.mirrorMode=true;
	}
	void write(std::ostream& out) {
		out << year << "\t"<<probability<<"\t"<<name<<std::endl;;
		nativeConnectivity.toSquareStream(out);
	}
};


//Assume that cabinets are ordered either by rows (if ordered by column, this should not change  the resulting energy and cost anyway)
struct CabinetYear {
	
	unsigned int Size;
	
	Position cornerPosition;
	Position cornerIndex;

	Position getRackPosition(unsigned int index){
		Position tmpIndex;
		tmpIndex.x=(index%cornerIndex.x);
		tmpIndex.y=(index/cornerIndex.x);
		return Parameters::RackIndexToPosition(tmpIndex);
	}	
	
	void read(std::istream& in) {
		in >> cornerIndex.y >> cornerIndex.x ;;
		distance.fromSquareStream(in);
		distance.mirrorMode=true;
		assert(distance.isSquare());
		Size=distance.getRows();
		assert(cornerIndex.x*cornerIndex.y==(int)Size);
		
		cornerPosition=Parameters::RackIndexToPosition(cornerIndex);

		std::cout<<" Read cabinet year with size "<<Size<<" and corner index ("<<cornerIndex.x	<<","<<cornerIndex.y<<")"<<std::endl;	
		
	}
	
	void write(std::ostream& out) {
		out << cornerIndex.y << "\t"<< cornerIndex.x << std::endl;;
		distance.toSquareStream(out);
	}
private:
	Mat distance;

};

class Config {
public: 
	//Current year, starting at year 0
	unsigned int currentYear;
	unsigned int CableNormDegree;	
	bool UsePatchPanel;
	
	std::string fileBase;
	
	Config(const char* inputFileName)  {
		assert(load(inputFileName));
	}

	Config(){}

	virtual bool load(const char* filename ){
		std::ifstream inFile(filename);
		if(!inFile.good()){
			std::cerr<<"Could not open file <"<<filename<<"> for loading input data!"<<std::endl;
			return false;
		}
		unsigned int years,topology_count;
		inFile >> years >> topology_count >> currentYear >> CableNormDegree >> UsePatchPanel;
		unsigned int n=0;
		A.clear();
		for(unsigned int y=0;y<years;y++){
			A.push_back(CabinetYear());
			A.back().read(inFile);
			assert(A.back().Size>n);
			n=A.back().Size;
		}

		/*
		  //Test for convertId
		for(unsigned int y=0;y<years;y++){
			for(unsigned int i=0;i<A[y].Size;i++){
				unsigned int fi=convertId(i,y);
				std::cout<<"y"<<y<<" "<<i<<"->"<<fi<<std::endl;
				assert(convertId(fi,A.size()-1,y)==i);
			}
		}
		*/

		T.clear();
		for(unsigned int t=0;t<topology_count;t++)
		{
			T.push_back(Topology());
			T.back().read(inFile);
			assert(T.back().year < A.size());
		}
		
		//Optionally try to load best permutations (makes only sense for years > 0 )
		unsigned int loadedPermutation=0;
		for(unsigned int ti=0;ti<topology_count;ti++)
		{
			Topology& t=T[ti];
			unsigned int NTop = A[t.year].Size;
			if(t.permutation.fromVectorStream(inFile))
			{
				loadedPermutation++;
				assert(t.permutation.getRows()==NTop);
			}else{
				t.permutation = Mat::buildPermutationVector(NTop);
			}
		}

		
		inFile.close();


		//Convert topologies to the last year indexes, to ensure coherency of the results
		_buildConversionList();

		unsigned int FinalSize=A[A.size()-1].Size;
		for(unsigned int ti=0;ti<topology_count;ti++)
		{
			Topology& t=T[ti];
			sortConversionList(t.year);
			//Initialize the converted connectivity list
			t.finalConnectivity=Mat(FinalSize,FinalSize);
			t.finalConnectivity.mirrorMode=true;
			for(unsigned int a=0;a<t.nativeConnectivity.getRows();a++)
			{
				unsigned int newA=conversionList[a].finalId;
				for(unsigned int b=a;b<t.nativeConnectivity.getRows();b++)
				{
					if(t.nativeConnectivity(a,b)==0)
						continue;
					unsigned int newB=conversionList[b].finalId;
					t.finalConnectivity(newA,newB)=t.nativeConnectivity(a,b);
				}
			}
			//Convert the permutation list
			for(unsigned int r=0;r<t.permutation.getRows();r++)
			{
				t.permutation(r)=conversionList[t.permutation(r)].finalId;
			}
		}
		
		//Store file base for save
		fileBase=filename;
		fileBase=fileBase.substr(0,fileBase.rfind('_'));
		
		std::cout << "Loaded "<< T.size()<<" topologies, "<<A.size()<<" years, "<<loadedPermutation<<" permutations"<<std::endl;


		return true;
	}

	virtual void save(){
		std::stringstream ss; ss<<fileBase<<"_"<<currentYear+1<<".in";

		//Save input file for next program
		std::ofstream outFile(ss.str().c_str());
		outFile << A.size() <<"\t"<<T.size() <<"\t"<<currentYear+1  << "\t" << CableNormDegree << "\t" << UsePatchPanel << std::endl;
		for(unsigned int y=0;y<A.size();y++){
			A[y].write(outFile);
		}
		for(unsigned int t=0;t<T.size();t++)
		{
			T[t].write(outFile);
		}

		for(unsigned int t=0;t<T.size();t++)
		{
			Mat nativePermutation=T[t].permutation;
			unsigned int year=T[t].year;
			for(unsigned int r=0;r<nativePermutation.getRows();r++)
			{
				nativePermutation(r)=convertId(nativePermutation(r),10000,year);
			}

			nativePermutation.toVectorStream(outFile);
		}
		outFile.close();	
	}

	unsigned int getN()
	{
		return A.back().Size;		
	}
	
	unsigned int getNTopologies() {
		return T.size();
	}

	unsigned int getNYears() {
		return A.size();
	}
	
	unsigned int getNPastCabinets()
	{
		if(currentYear==0)
			return 0;
		return A[currentYear-1].Size;	
	}


		unsigned int getTopologySize(unsigned int tIndex) {
			return T[tIndex].permutation.getRows();
		}

		unsigned int convertId(unsigned int id, unsigned int fromYear, unsigned int toYear=100000 ){
			if(toYear>=A.size())
				toYear=A.size()-1;
			if(fromYear>=A.size())
				fromYear=A.size()-1;
			Position p=_rackIndex(id,A[fromYear].cornerIndex);
			//std::cout<<p.x<<","<<p.y<<"c"<< A[fromYear].cornerIndex.x<<","<<A[fromYear].cornerIndex.y <<std::endl;
			return _rackId(p,A[toYear].cornerIndex);
		}

protected:

	
		std::vector<CabinetYear> A;
		
		std::vector<Topology> T;

	
		static const unsigned int Inf=10000;

	
		inline unsigned int _Y(Position rackIndex){
			unsigned int colYear=0;
			unsigned int rowYear=0;
			for(unsigned int year=0;year<A.size();year++){
				if(A[year].cornerIndex.x<=rackIndex.x)
					colYear=year+1;
				if(A[year].cornerIndex.y<=rackIndex.y)
					rowYear=year+1;
			}

			assert(rowYear<A.size() && colYear<A.size() );
				return (std::max)(colYear,rowYear);
		}



		std::vector<NodeInfo> conversionList;

		void _buildConversionList(){

			conversionList.clear();
			Position currentIndex,lastIndex=A[A.size()-1].cornerIndex;
			for(currentIndex.x=0; currentIndex.x<lastIndex.x;currentIndex.x++)
			{
				for(currentIndex.y=0; currentIndex.y<lastIndex.y;currentIndex.y++)
				{
					//Find which year this node is newly installed..			
					NodeInfo inf;
					inf.year=_Y(currentIndex);
					inf.nativeId=_rackId(currentIndex,A[inf.year].cornerIndex);
					inf.finalId=_rackId(currentIndex,lastIndex);
					inf.position=Parameters::RackIndexToPosition(currentIndex);
					inf.index=currentIndex;
					conversionList.push_back(inf);
				}
			}
		
		}


		void sortConversionList(unsigned int year);

   
		Position _rackIndex(unsigned int id, unsigned int year=100000 )
		{
			if(year>=A.size())
				year=A.size()-1;
			return _rackIndex(id,A[year].cornerIndex);
		}

		Position _rackIndex(unsigned int id, Position corner )
		{
			Position p;
			p.x=id%corner.x;
			p.y=id/corner.x;
			return p;
		}

		unsigned int _rackId(Position index, Position corner )
		{
			return corner.x*index.y+index.x;		
		}
	
};


#endif /* CONFIG_H_ */














