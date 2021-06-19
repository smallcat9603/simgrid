#include "common.h"
#include <fstream>

namespace MinCost{


	class PathSet{
		public:
		
		typedef std::pair<unsigned int, unsigned int> Value;

		void clear(){data.clear();phases.clear();resetReading();}
		
		void addNewPath(const PathSet& in ){
			bforeach(const Value& v, in.data){
				data.push_back(v);
			}
		}
		void addNewElement(Value in){
				data.push_back(in);
		}
		void addNewPhase(){phases.push_back(data.size());}
		void save(const char* filename){
			std::ofstream file(filename);
			//Save phases
			file<<phases.size();
			bforeach(std::size_t& ph, phases){
				file<<"\t"<<ph;
			}
			file<<std::endl;
			resetReading();
			PathSet p;
			while(readNextPath(p)){
				file<<p.data[0].first<<"\t"<<p.data[0].second;
				for(unsigned int i=1;i<p.data.size();i++){
					file<<"\t"<<p.data[i].first<<","<<p.data[i].second;
				}
				file<<std::endl;
			}
			file.close();
		}
		
		bool load( const char* filename){
			std::ifstream file(filename);
			if(!file.good())
				return false;
			unsigned int first,second,size;
			char tmp;
			clear();
			//Load phases
			file>>size;
			for(unsigned int i=0;i<size;i++){
				file>>first;
				phases.push_back(first);
			}
			//Load paths
			while(file.good()){
				file>>first>>size;
				data.push_back(Value(first,size));
				for(unsigned int i=0;i<size;i++){
					file>>first>>tmp>>second;
					data.push_back(Value(first,second));
				}
			}
			file.close();
			return true;
		}
		void find(PathSet& out, Value source, Value target){
			out.clear();
			resetReading();
			size_t lastIndex;
			while(currentIndex<data.size()){
				lastIndex=currentIndex+data[currentIndex].second;
				if( (_equal(data[currentIndex+1],source) || _equal(data[currentIndex+1],target)) &&
				    (_equal(data[lastIndex],source) || _equal(data[lastIndex],target)) )
				{
					do{
						out.data.push_back(data[currentIndex]);
					} while(++currentIndex<=lastIndex);
				}
				else
					currentIndex=lastIndex+1;
		   }
		}

		void resetReading(){currentIndex=0;}
		void skipPhases(unsigned int nphases){
			bool startCount=false;
			bforeach(size_t& i, phases){
				if(currentIndex < i)
					startCount=true;
				if(startCount && nphases==0)
					break;
				if(startCount){
					nphases--;
					//Advance currentIndex to the first path of next phase
					currentIndex=i;
				}
			}
		}
		bool readNextPath(PathSet& out){
			if(currentIndex>=data.size())
				return false;
				unsigned int size=data[currentIndex].second;
				out.clear();
				out.data.push_back(data[currentIndex++]);
				for(unsigned int i=0;i<size;i++){
					assert(currentIndex<data.size());
					out.data.push_back(data[currentIndex++]);
				}
				return true;
		}
		bool readNextValue(Value& out){
			if(currentIndex>=data.size())
				return false;
			out=data[currentIndex];
				return true;
		}

		unsigned int getNPhases(){return phases.size();}

		//Remove one to the number of nodes to get hop count..
		inline unsigned int getCurrentPathHopCount()
		{
			assert(data[currentIndex].second>0);
			return data[currentIndex].second - 1 ;
			
		}
	private:

		//Paths are stored linearly
		//First value: first-> number of cables, second-> number of nodes in this path
		//Following values: first-> tenth of row, second -> tenth of column
		std::vector<Value> data;

		std::vector<std::size_t> phases;
		//Index for sequential reading
		unsigned int currentIndex;

		bool _equal(const Value& a, const Value& b){
			return a.first==b.first && a.second==b.second;
		}
	};

}












