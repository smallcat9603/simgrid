#include "Config.h"




/*
bool compareNodeInfoByFinalId(const NodeInfo& a, const NodeInfo& b)
{
	return a.finalId<b.finalId;
}
*/


std::vector<std::string> readConfigurationOptions(std::string filename,std::string keyword){
	std::vector<std::string> result;
	const unsigned int  LineMaxSize=1000;
	char l[LineMaxSize];
	std::ifstream inFile(filename.c_str());
	do{
		inFile.getline(l,LineMaxSize-1);
		//TODO: add proper line size overflow handling
		if(!inFile.good())
			break;
		std::string s=l;

		
		//std::cout<<"Reading <"<<s<<">"<<std::endl;
	    if (s.find(keyword)!=0)
			continue;
		//std::cout<<"We have a winner! "<<std::endl;

		int index2,index=keyword.length();


		//There should be a space immediately following the keyword!
		assert(s.length()==(unsigned int) index ||  s[index] ==' ');
	

		do{
			//Advance after spaces
			index=s.find_first_not_of(" ",index);
			index2=s.find(" ",index);
			if(index2<0)
				index2=s.length();
			result.push_back(s.substr(index,index2-index));
			index=index2;
		}while((unsigned int)index<s.length());
	}while(true);
	inFile.close();
	return result;
}


unsigned int SortingLastYear;

bool compareNodeInfoByIndex(const NodeInfo& a, const NodeInfo& b)
{
	if(a.year<=SortingLastYear && b.year>SortingLastYear )
		return true;
	if(b.year<=SortingLastYear && a.year>SortingLastYear )
		return false;
	return a.index.y<b.index.y || (a.index.y==b.index.y && a.index.x<b.index.x) ;
}

void Config::sortConversionList(unsigned int year){
	SortingLastYear=year;
	std::sort(conversionList.begin(),conversionList.end(),compareNodeInfoByIndex);
}











