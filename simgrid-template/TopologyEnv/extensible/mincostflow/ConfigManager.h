/*
 * ConfigManager.h
 *
 *  Created on: Sept 24, 2014
 *      Author: chaix
 */

#ifndef CONFIGMANAGER_H_
#define CONFIGMANAGER_H_

#include "../Config.h"
#include "common.h"
#include "graph.h"
#include <cerrno>
namespace MinCost{
	
	class ConfigManager: public Config {
	public:

	
		std::vector<std::vector<unsigned int> > C;
		std::vector<std::vector<Demand> > demands;

		Mat allConnectivity;

//The set of cables already installed (with their count)
		PathSet previousCables;
		
		virtual bool load(const char* allocFileName, const char* postFileName, const char* globalConfigFileName ){

			if(!Config::load(allocFileName))
				return false;


			if(postFileName!=NULL && postFileName[0]!='\0')
			{
				if(!previousCables.load(postFileName))
					return false;
			}
	
	
			//Decrease the current year, since we use the input of next year ( needed to know where topology nodes are mapped)
			assert(this->currentYear>0);
			this->currentYear--;
			
			//Read the POS configuration (i.e. the list of topologies for each configuration)

			std::vector<std::string> confVec=readConfigurationOptions(globalConfigFileName,"POST");
			assert(!confVec.empty());
			std::string& confStr=confVec[0];
		
			int index=0;
			int index2=0;
			C.clear();
			C.push_back(std::vector<unsigned int>());
			do{
				//Advance after spaces
				index=confStr.find_first_not_of("0123456789",index+1);
				unsigned int ti=std::strtol(confStr.substr(index2,index-index2).c_str(),NULL,10);

				assert(!errno);

				assert(ti<T.size());
				
				C.back().push_back(ti);

				if(index<0)
					break;
			
				switch(confStr[index]){
				case '.':
					break;
				case '+':
					C.push_back(std::vector<unsigned int>());
					break;
				default:
				    std::cerr<<"Character <"<<confStr[index]<<"> ( code "<<(unsigned int)confStr[index]<<") was unexpected in POS configurations string <"<<confStr<<" !"<<std::endl;  
					assert(false);
					//We dont like other characters in this formula, do we?
					break;		  		
				}

				index2=index+1;
			}while((unsigned int)index<confStr.length());


			std::cout << "Read "<<C.size()<<" configuration(s):"<<std::endl;
			for(unsigned int ci=0;ci<C.size();ci++){
				std::cout<<"["<<ci<<"]:";
				for(unsigned int i=0;i<C[ci].size();i++){
					unsigned int ti=C[ci][i];
					std::cout<<" "<<ti;
				}
				std::cout<<std::endl;
			}


			//Init the connectivity demands for each configuration
			Mat confConnectivity(getN(),getN());
			for(unsigned int ci=0; ci<C.size();ci++){
			
				//Build the aggregated connectivity for each configuration
				confConnectivity.reset();
				for(unsigned int i=0;i<C[ci].size();i++){
					unsigned int ti=C[ci][i];
					assert(ti<T.size());
					confConnectivity.maxWith(T[ti].finalConnectivity);
				}

				sortConversionList(getNYears());

				//Clean demands vector
				if(demands.size()<=ci)
					demands.push_back(std::vector<Demand>());
				else
					demands[ci].clear();

				std::cout<<"Configuration "<<ci<<std::endl;
				confConnectivity.toSquareStream(std::cout," ");
				
				for(unsigned int i=0;i<getN();i++)
				{
					Position posi=conversionList[i].position;
					unsigned int yi=conversionList[i].year;
				
					for(unsigned int j=i;j<getN();j++)
					{
						if(confConnectivity(i,j)==0)
							continue;
					
						Position posj=conversionList[j].position;
						unsigned int yj=conversionList[j].year;

						//Prune demands that are not for this year (i.e. Past-Past and Future-Future)
						if( (yi<currentYear && yj<currentYear) || (yi>currentYear && yj>currentYear) )
							continue;
					
						/*if(yi==year && yj==year)
						  {
						  //Present-present demand
						  }*/

						demands[ci].push_back(Demand());
						Demand& d=demands[ci].back();
						d.source=i;
						d.target=j;
						d.demand=confConnectivity(i,j);
						d.distanceProduct=d.demand*(posj-posi).getNorm();
					
					}
				}

				//Sort demands by decreasing distanceProduct
				std::sort(demands[ci].begin(),demands[ci].end(), compareDemandsByDecreasingDistance);

			}

			//Init the total connectivity matrix
			allConnectivity=Mat(getN(),getN());
			for(unsigned int ti=0;ti<T.size();ti++)
				allConnectivity+=T[ti].finalConnectivity;



			return true;
		}

		void save(){

			Config::save();

			std::stringstream ss;ss<<"post_"<<currentYear+1<<".in";
			previousCables.save(ss.str().c_str());

		}


		void init() {
		}


		void createGraph(Graph& g){

			
			std::vector<VertexStepCost> switchSteps=Parameters::getSwitchSteps();

			std::vector<VertexStepCost> patchpanelsSteps=Parameters::getPatchPanelSteps();

			
			g.clear();
			Edge::nconfigurations=C.size();
			

			//Create one vertex per rack (will make it easier to access  them later )
			Vertex v;
			for(unsigned int i=0;i<getN();i++){
				v.setCost(switchSteps);
				assert(switchSteps.empty()); //If not empty, need to change stage attribute according to node installation year
				v.stage=Vertex::Deadend;
				unsigned int desc=g.addVertex(v);
				assert(desc==i);
			}

			
			//Promote  racks at the stage border as patch-panel capable, if enabled
			if(UsePatchPanel){
				Position finalCorner=A.back().cornerIndex;
				Position patchId;
				if(currentYear+1<A.size()){
					Position corner=A[currentYear].cornerIndex;
					for(patchId.x=corner.x-1,patchId.y=0;patchId.y<corner.y;patchId.y++){

						g(_rackId(patchId,finalCorner)).setCost(Parameters::getPatchPanelSteps());
					}
					for(patchId.x=0,patchId.y=corner.y-1;patchId.x+1<corner.x;patchId.x++){
						g(_rackId(patchId,finalCorner)).setCost(Parameters::getPatchPanelSteps());
					}
				}
				if(currentYear>0){
					Position corner=A[currentYear-1].cornerIndex;
					for(patchId.x=corner.x-1,patchId.y=0;patchId.y<corner.y;patchId.y++){
						g(_rackId(patchId,finalCorner)).setCost(Parameters::getPatchPanelSteps());
					}
					for(patchId.x=0,patchId.y=corner.y-1;patchId.x+1<corner.x;patchId.x++){
						g(_rackId(patchId,finalCorner)).setCost(Parameters::getPatchPanelSteps());
					}
				}
			}


			sortConversionList(getNYears());

			//Install cables generated by previous POST steps
			previousCables.resetReading();
			PathSet::Value tmpValue;
			unsigned int ncables;
			unsigned int n1,n2;
			while(previousCables.readNextValue(tmpValue))
			{
				assert(tmpValue.second==2);
				ncables=tmpValue.first;
				previousCables.readNextValue(tmpValue);
				n1=convertIdFromPathset(tmpValue);
				previousCables.readNextValue(tmpValue);
				n2=convertIdFromPathset(tmpValue);

				if(conversionList[n1].year<currentYear && conversionList[n2].year<currentYear )
					continue;

				float rackDistance=(conversionList[n1].position-conversionList[n2].position).getNorm();
				_addGroupEdges(g,n1,n2,rackDistance,ncables);	
			}


			Edge e;
			for(unsigned int i=0;i<getN();i++)
			{
				Position posi=conversionList[i].position;
				unsigned int yi=conversionList[i].year;
				
				for(unsigned int j=i;j<getN();j++)
				{
					if(allConnectivity(i,j)==0)
						continue;

				
					Position posj=conversionList[j].position;
					unsigned int yj=conversionList[j].year;

					//Prune demands that are not for this year (i.e. Past-Past and Future-Future)
					if( (yi<currentYear && yj<currentYear) || (yi>currentYear && yj>currentYear) )
						continue;

					//std::cout <<"Possible path "<<i<<"<->"<<j<<std::endl;
			

					float rackDistance=(posi-posj).getNorm();
					e.rate= Parameters::cableCost((Parameters::cableLength(rackDistance)));							   
					
					if(yi==currentYear && yj==currentYear)
					{
						//Present-present demand -> Add a direct edge
						g.addEdge(e,i,j);
					} else if(yi<currentYear || yj<currentYear)
					{
						//Past-Present or Past-Future -> There should be some cables from previous post steps for this
						assert(g.getLength(i,j)>=0);
					} else {
						//Present-Future
						assert( (yi==currentYear && yj>currentYear) || (yi>currentYear && yj==currentYear));
						unsigned int n1,n2;
						if(yi<yj){n1=i;n2=j;}else{n1=j;n2=i;}

						_addGroupEdges(g,n1,n2,rackDistance,0);
						
					}
		
				}
			}

			//g.save("intern.graph");


		}

		unsigned int convertIdFromPathset(PathSet::Value& value)
		{
			assert(false);
		}
	

		//Create edges and a virtual vertex for implementing the group from node n1. Node n2 is only given to make sure that it will be included in the group 
		bool _addGroupEdges(Graph& graph, unsigned int n1, unsigned int n2, float rackDistance, unsigned int precapacity=0){

			assert(_Y(_rackIndex(n1))<=currentYear);
			assert(_Y(_rackIndex(n2))>_Y(_rackIndex(n1)));



			
			//This constant allows to prune some groups which distance are similar.
			const float GroupRateFactor=1.1;
			//Rate of connections of group to patch panels  
			const int GroupPatchPanelRate=40;
			
			/*
			if(UsePatchPanel){
				//The sampling at which cables are connected to patch-panels 
				const unsigned int PatchPanelSamplingPeriod=2;
				//Patch panels: Insert cables to a sampling of the patch panels, and destination cable
				Position finalCorner=A.back().cornerIndex;
				Position corner=A[currentYear].cornerIndex;
				Position patchId=corner;
				Position sourceId=conversionList[n1].index;
				Position targetId=conversionList[n2].index;
				if(targetId.y>targetId.x)
				{
					unsigned int start=std::min(sourceId.y,targetId.y);
					unsigned int end=std::max(sourceId.y,targetId.y);
					for(patchId.y=corner.y-1,patchId.x=start;patchId.x<end;patchId.x+=PatchPanelSamplingPeriod){
						unsigned int pid=_rackId(patchId,finalCorner);
						rackDistance=(conversionList[n1].position-conversionList[pid].position).getNorm();
						e.rate= Parameters::cableCost((Parameters::cableLength(rackDistance)));							 								 g.addEdge(e,n1,pid);
						rackDistance=(conversionList[pid].position-conversionList[n2].position).getNorm();
						e.rate= Parameters::cableCost((Parameters::cableLength(rackDistance)));							 								 g.addEdge(e,pid,n2);
					}
				} else {
					unsigned int start=std::min(sourceId.x,targetId.x);
					unsigned int end=std::max(sourceId.x,targetId.x);
					for(patchId.x=corner.x-1,patchId.y=start;patchId.y<end;patchId.y+=PatchPanelSamplingPeriod){
						unsigned int pid=_rackId(patchId,finalCorner);
						rackDistance=(conversionList[n1].position-conversionList[pid].position).getNorm();
						e.rate= Parameters::cableCost((Parameters::cableLength(rackDistance)));							 								 g.addEdge(e,n1,pid);
						rackDistance=(conversionList[pid].position-conversionList[n2].position).getNorm();
						e.rate= Parameters::cableCost((Parameters::cableLength(rackDistance)));							 								 g.addEdge(e,pid,n2);
					}
				}
				return true;
			}*/
						

			assert(rackDistance>0);
			const float nominalRate=Parameters::cableCost((Parameters::cableLength(rackDistance)));

			Position finalIndex=_rackIndex(n2);
			
			Vertex::Period period=(finalIndex.x<finalIndex.y)?Vertex::VirtualVertical:Vertex::VirtualHorizontal;
			
			//Check if there already exist an identic group (that could have been generated for different n2

			Edge e;
		  
			bforallOutEdges(de, n1, graph){
				Edge& cande=graph(de);
				
				if(cande.rate<nominalRate || cande.rate>=GroupRateFactor*nominalRate)
					continue;
				unsigned int candidateId=graph.opposite(de,n1);
				Vertex& candidate=graph(candidateId);
				if(candidate.stage!=period)
					continue;
								
				//Found a matching candidate, do not need to add a new group, just a a free cable to destination
				graph.addEdge(e,candidateId,n2);
				return false;
			}

			
			Vertex v;
			v.stage=period;
			std::vector<VertexStepCost> virtualCost;virtualCost.push_back(VertexStepCost(10000,1.0));
			v.setCost(virtualCost);
			unsigned int desc=graph.addVertex(v);



			//If we use patch-panels, use connect from source through a selection of patch panels  

			
			Position tmpIndex,startIndex;
			Position finalCornerIndex=A.back().cornerIndex;
			if(_Y(_rackIndex(n1))==currentYear){
				startIndex=A[currentYear].cornerIndex;

				if(UsePatchPanel){
					if(period==Vertex::VirtualVertical){
						tmpIndex.y=startIndex.y-1;
						for(tmpIndex.x=0;tmpIndex.x<A[currentYear].cornerIndex.x;tmpIndex.x++){
							if(tmpIndex.x!=finalIndex.x && (rand()%100)>GroupPatchPanelRate)
								continue;
							unsigned int tmpId=_rackId(tmpIndex,A[currentYear].cornerIndex);
							float distance=(conversionList[tmpId].position-conversionList[n1].position).getNorm();
							float toPatchRate=Parameters::cableCost((Parameters::cableLength(distance)));
							e.rate=toPatchRate;
							graph.addEdge(e,n1,tmpId);
							e.rate=nominalRate-toPatchRate;
							graph.addEdge(e,tmpId,desc);
							
						}
					}else{
						tmpIndex.x=startIndex.x-1;
						for(tmpIndex.y=0;tmpIndex.y<A[currentYear].cornerIndex.y;tmpIndex.y++){
							if(tmpIndex.y!=finalIndex.y && (rand()%100)>GroupPatchPanelRate)
								continue;
							//Redundant code=bad!
							unsigned int tmpId=_rackId(tmpIndex,A[currentYear].cornerIndex);
							float distance=(conversionList[tmpId].position-conversionList[n1].position).getNorm();
							float toPatchRate=Parameters::cableCost((Parameters::cableLength(distance)));
							e.rate=toPatchRate;
							graph.addEdge(e,n1,tmpId);
							e.rate=nominalRate-toPatchRate;
							graph.addEdge(e,tmpId,desc);
							
						}
					}
				}else{
					//No patch rate
					e.rate=nominalRate;
					graph.addEdge(e,n1,desc);
					e.rate=0;
				    graph.addEdge(e,desc,n2);
				}
			}
			else
			{
				startIndex=A[currentYear-1].cornerIndex;				
				e.precapacity=precapacity;
				e.rate=nominalRate;
				graph.addEdge(e,n1,desc);
			}

			if(period==Vertex::VirtualVertical){
				startIndex.x=0;
				finalIndex.x=finalCornerIndex.x-1;
			} else {
				startIndex.y=0;
				finalIndex.y=finalCornerIndex.y-1;
			}

			
/*
			//Add virtual edges from the virtual node to reachable nodes
			sortConversionList(getNYears());
			Position sourcePosition=conversionList[n1].position;
			e.rate=0;
			e.precapacity=0;
			for(tmpIndex.x=startIndex.x; tmpIndex.x<=finalIndex.x;tmpIndex.x++)
				for(tmpIndex.y=startIndex.y; tmpIndex.y<=finalIndex.y;tmpIndex.y++)
				{
					unsigned int candidateId=_rackId(tmpIndex,finalCornerIndex);
					if( (conversionList[candidateId].position-sourcePosition).getNorm()<=rackDistance)
						graph.addEdge(e,desc,candidateId);
					
				}

*/
			return true;
			
		}

	};




}
#endif /* CONFIGMANAGER_H_ */











