#include "common.h"
#include <fstream>

#ifndef GRAPH_H_
#define GRAPH_H_



namespace MinCost{

	using namespace  boost;
	



	class Edge{
	public:

		static unsigned int nconfigurations;


		//current use 
		unsigned int x[NCONFIGURATIONMAX];

		//The -constant- cost rate
		float rate;
		//Preinstalled capacity (cannot be modified, so its cost should be removed
		unsigned int precapacity;
		
		//Short-life algorithmic-oriented attributes
		float length;
		int visited;
		unsigned int margin[NCONFIGURATIONMAX];
		
	Edge():rate(0.0),precapacity(0)
		{
			for(unsigned int i=0;i<nconfigurations;i++){
				x[i]=0;
			}
		}

		Edge& operator= (const Edge& old){
			for(unsigned int i=0;i<nconfigurations;i++){
				x[i]=old.x[i];
				margin[i]=old.margin[i];
			}
			rate=old.rate;
			precapacity=old.precapacity;
			length=old.length;
			visited=old.visited;
			return *this;
			
		}

	};


	class Vertex{
	public:

		//The different steps for the use of vertex 
		//The step 0 with zero cost and zero capacity is assumed, unless first step starts at 0
		//steps are assumed to be strictly increasing
		unsigned int nsteps;
		VertexStepCost steps[NSTEPMAX];
		//The index of the vertex in the graph. Actual nodes id is the final year id, virtual nodes id depends on graph construction 
		unsigned int index;



		//Defines how this node can be used for routing demands:
		//->Past vertices can only be used if source or target is Past too
		//->Present vertices can always be used
		//->Future vertices can only be used if source or target is Future too
		//->Deadend vertices can only be used ifthis is the source or the target
		enum Period {Past,Present,VirtualVertical,VirtualHorizontal,Future,Deadend} stage;
		
		//Short-life attributes
		//The current partition for metric equation research
		int partition;
		float ratio;
		//The total number of attached cables, for each configuration
		unsigned int degreeCount[NCONFIGURATIONMAX];


		
	Vertex():nsteps(0),index(0xFFFF)
		{
			for(unsigned int i=0;i<Edge::nconfigurations;i++)
				degreeCount[i]=0;

		}

		Vertex& operator= (const Vertex& old){
			for(unsigned int i=0;i<old.nsteps;i++)
				steps[i]=old.steps[i];
			for(unsigned int i=0;i<Edge::nconfigurations;i++)
				degreeCount[i]=old.degreeCount[i];
			
			nsteps=old.nsteps;
			partition=old.partition;
			ratio=old.ratio;
			stage=old.stage;
			return *this;
		}

		bool isCrossable() const{
			return stage==VirtualVertical || stage==VirtualHorizontal || nsteps!=0 ;
		}
		void setCost(std::vector<VertexStepCost> steps){
			nsteps=steps.size();
			for(unsigned int i=0;i<steps.size();i++)
				this->steps[i]=steps[i];
		}
   

		//cost computation
		float stepCost(int dstep=0){
			if(!isCrossable())
				return 0;
			
			int stepIndex=_currentStep()+dstep;
				if(stepIndex<0)
					return 0.0;
				assert(stepIndex<nsteps);
				return steps[stepIndex].second;
		}
		//Input argument is the different to current step!
		bool hasStep(int dstep=0){
			if(!isCrossable())
				return true;
			return _currentStep()+dstep<nsteps;
		}



		unsigned int margin(unsigned int configurationIndex){
						if(!isCrossable())
							return 10000;
						
			assert(configurationIndex<Edge::nconfigurations);
			assert(steps[_currentStep()].first>=degreeCount[configurationIndex]);
			return steps[_currentStep()].first-degreeCount[configurationIndex];
		}
		unsigned int margin(unsigned int* result){
			if(!isCrossable())
			{
				for(unsigned int i=0;i<Edge::nconfigurations;i++)
				{
					result[i]=10000;
				}
				return 10000;
			}			
			unsigned int stepCapacity=steps[_currentStep()].first;
			for(unsigned int i=0;i<Edge::nconfigurations;i++)
			{
				assert(stepCapacity>=degreeCount[i]);
			    result[i]=stepCapacity-degreeCount[i];
			}
		}
	
		
		
		private:

		int _currentStep(){

			//Non-crossable nodes have no limit 
			assert(isCrossable());
			
				unsigned int maxCount=0;
				for(unsigned int i=0;i<Edge::nconfigurations;i++)
					if(degreeCount[i]>maxCount)
						maxCount=degreeCount[i];
				if(maxCount==0)
				return -1;
			for(unsigned int i=0;i<nsteps;i++)
				if(maxCount<=steps[i].first)
					return (int)i;
			//Node is oversaturated!
			assert(false);
			return 10000;
		}
   
	};

		//typedef boost::compressed_sparse_row_graph<boost::bidirectionalS,Vertex,Edge,no_property,unsigned int,unsigned int>  _GraphType;
		//typedef boost::adjacency_list<boost::vecS,boost::vecS,bidirectionalS,Vertex,Edge>  _GraphType;
	//typedef boost::property<boost::vertex_index_t,unsigned int, Edge> EdgeProperty;
		typedef boost::adjacency_list<boost::vecS,boost::vecS,undirectedS,Vertex,Edge>  _GraphType;
		typedef typename boost::graph_traits<_GraphType>::edge_descriptor _EdgeDesc;
		typedef typename boost::graph_traits<_GraphType>::edge_iterator _EdgeIt;
		typedef typename boost::graph_traits<_GraphType>::out_edge_iterator _OutEdgeIt;
		typedef typename boost::graph_traits<_GraphType>::vertex_iterator _VertexIt;
		

		
	class Graph{
	public:

		typedef _EdgeDesc EdgeDescriptor;
/*
	void init(const std::vector<EdgeBounds>& edgeBounds, const std::vector<Edge>& edgeProps, const std::vector<Vertex>& vertexProps){
		//data(vertexProps.size())

			assert(edgeBounds.size()==edgeProps.size());   

			//Init properties

			for(IndexType i=0; i<edgeProps.size(); i++){
				assert(edgeBounds[i].first<vertexProps.size());
				assert(edgeBounds[i].second<vertexProps.size());
				std::pair<_EdgeDesc,bool> res=boost::add_edge(edgeBounds[i].first,edgeBounds[i].second,data);
				if(!res.second)
				{
					std::cerr<<"Edge from node "<< edgeBounds[i].first <<" to "<<edgeBounds[i].second<<" could not be created!"<<std::endl;
				}else
				{
					data[res.first]=edgeProps[i];
				}

			}
			for(IndexType i=0; i<vertexProps.size(); i++)
				data[i]=vertexProps[i];

		}
*/

		//You need to update metrics delta beforehand
		void updateEdgesMarginFromConstraints(const std::vector<MetricEquation>& metrics);
		//You need to update vertex degree count beforehand
		void updateEdgesMarginFromVertices();

		void clear(){invalidatedSearch=true;data.clear();}
		std::size_t addVertex(const Vertex& v)
		{
			invalidatedSearch=true;
			unsigned int desc=boost::add_vertex(v,data);
			data[desc].index=desc;
			return desc;
		}
		EdgeDescriptor addEdge(const Edge& e, std::size_t a,std::size_t b)
		{
			invalidatedSearch=true;
			std::pair<EdgeDescriptor,bool> res=boost::add_edge(a,b,e,data);
			assert(res.second);
			return res.first;
		}
		
		
		float getLength(unsigned int source,unsigned int target)
		{

			_runPathSearch(source,target);

			return distance[target];	
		}


		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!!!!NOTE THAT RETURNED PATH IS BACKWARD!!!!! 
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		float getPath(unsigned int source,unsigned int target,std::vector<EdgeDescriptor>& outPath)
		{
			outPath.clear();

			_runPathSearch(source,target);

			//Unroll the path to source
			unsigned int v=target;
			while(predecessors[v]!=v){
				//Find the edge
				bforeach(const EdgeDescriptor& e,boost::out_edges(v,data)){
					if(opposite(e,v)!=predecessors[v] )
						continue;
					outPath.push_back(e);
					break;
				}
				v=predecessors[v];
			}
			//If false the target was unreachable!
			if(v!=source)
			{
				std::cout<<"Could not reach "<<source<<" from "<<target<<std::endl<<"Prede.: ";  
				for(unsigned int i=0;i<predecessors.size();i++){
					std::cout<<i<<":"<<predecessors[i]<<" ";
				}
				std::cout<<std::endl<<"Distance: ";  
				for(unsigned int i=0;i<distance.size();i++){
					std::cout<<i<<":"<<distance[i]<<" ";
				}
				std::cout<<std::endl;  
				return -1;
			}
			
			return distance[target];	
		}

		void initLengths(const MetricElement* m );
		void initLengths(float value )
		{
			invalidatedSearch=true;
			std::pair<MinCost::_EdgeIt,MinCost::_EdgeIt> edges=boost::edges(data);
			bforeach(const EdgeDescriptor& e,edges){
				assert(value>0);
				data[e].length=value;
			}
		}
		void updateLength(unsigned int source,unsigned int target,float newValue)
		{
			assert(newValue>0);
			invalidatedSearch=true;
			std::pair<MinCost::_OutEdgeIt,MinCost::_OutEdgeIt> edges=boost::out_edges(source,data);
			bforeach(const EdgeDescriptor& e,edges){
				if(boost::target(e,data)!=target && boost::source(e,data)!=target )
					continue;
				data[e].length=newValue;
			}
		    //Could not find edge from source to target --> Problem
			assert(false);
		}

		void updateCapacity(const std::vector<ChangeElement>& changes);
		
		Edge& operator() (const EdgeDescriptor& e){return data[e];}
		Vertex& operator() (const IndexType& v){return data[v];}


		IndexType source(const EdgeDescriptor& e){return boost::source(e,data);}
		IndexType target(const EdgeDescriptor& e){return boost::target(e,data);}
		IndexType opposite(const EdgeDescriptor& e,const unsigned int v)
		{
			unsigned int si=boost::source(e,data);
			unsigned int ti=boost::target(e,data);
			return ((si==v)?ti:si);
		}
		
			
/*		
		_VertexIt _vertices(std::pair<_VertexIt,_VertexIt>& outVertices){outVertices=boost::vertices(data);return outVertices.first;}
		_EdgeIt _edges(std::pair<_EdgeIt,_EdgeIt>& outEdges){outEdges=boost::edges(data);return outEdges.first;}

		_OutEdgeIt _outEdges(IndexType srcVertex,std::pair<_OutEdgeIt,_OutEdgeIt>& outEdges){outEdges=boost::out_edges(srcVertex,data);return outEdges.first;}
*/		
		std::pair<_VertexIt,_VertexIt>  _vertices(){return boost::vertices(data);}
		std::pair<_EdgeIt,_EdgeIt>  _edges(){return boost::edges(data);}
		std::pair<_OutEdgeIt,_OutEdgeIt> _outEdges(IndexType srcVertex){return boost::out_edges(srcVertex,data);}

		
		unsigned int getNVertices() const{return boost::num_vertices(data);}


		void save(std::string filename){
			std::ofstream file(filename.c_str());
			file<<boost::num_vertices(data)<<" Vertices"<<std::endl;
			bforeach(const _GraphType::vertex_descriptor& v,boost::vertices(data)){
				Vertex& vv=data[v];
				file<<"VERTEX "<<vv.index<<std::endl;
				bforeach(const _GraphType::edge_descriptor& e,boost::out_edges(v,data)){

					Edge& ee=data[e];
					unsigned int si=data[boost::source(e,data)].index;
					unsigned int ti=data[boost::target(e,data)].index;
					
					file<<"-("<< ee.rate <<")-> "<< ((si==vv.index)?ti:si)<<std::endl;
					
				}
			}

			file.close();
		}
			
		

	private:


		_GraphType data;

		bool invalidatedSearch;
		unsigned int lastSource;
		std::vector<unsigned int> predecessors;
		std::vector<float> distance;
		void _runPathSearch(unsigned int source,unsigned int target);
		
	};



	
//Element on a metric inequality vector lambda. A 0 lamda value means that other lambda are zero
struct MetricElement {
MetricElement():lambda(0){}
MetricElement(const MetricElement& old):edgeDesc(old.edgeDesc),lambda(old.lambda){}
	MetricElement& operator=(const MetricElement& old){edgeDesc=old.edgeDesc;lambda=old.lambda;return *this;}
    MinCost::Graph::EdgeDescriptor edgeDesc;
  float lambda;
};


//An element defining a change in the solution
struct ChangeElement {
  MinCost::Graph::EdgeDescriptor edgeDesc;
	unsigned int count; //Final count
  unsigned int configurationIndex;
};



}; //namespace MinCost

//#define _genericForall( desc, val , graphname, descType, itType, valType, itFct ) descType desc; std::pair<itType,itType> val##iter; for ( valType& val= graphname((desc=*graphname.itFct(val##iter))); val##iter.first != val##iter.second; val=graphname(desc=*(++val##iter.first)))

#define bforallVertices(descname, graphname ) bforeach(const _GraphType::vertex_descriptor& descname, graphname._vertices() )
#define bforallEdges(descname, graphname ) bforeach(const _GraphType::edge_descriptor& descname, graphname._edges() )



//#define bforallVertices(descname, edgename,  graphname ) _genericForall(descname,vertexname,graphname,MinCost::IndexType,MinCost::_VertexIt,MinCost::Vertex,_vertices)
//#define bforallEdges( descname, edgename, graphname ) _genericForall(descname,edgename,graphname,MinCost::Graph::EdgeDescriptor,MinCost::_EdgeIt,MinCost::Edge,_edges)


//#define _genericForallProx( desc, val , vert, graphname, descType, itType, valType, itFct ) descType desc ; std::pair<itType,itType> val##iter; for ( valType& val= graphname(desc=*graphname.itFct(vert,val##iter)); val##iter.first != val##iter.second; val=graphname(desc=*(++val##iter.first)))

//#define bforallOutEdges( descname,edgename, srcVertex, graphname ) _genericForallProx(descname, edgename, srcVertex, graphname,MinCost::Graph::EdgeDescriptor,MinCost::_OutEdgeIt,MinCost::Edge,_outEdges)

#define bforallOutEdges( descname, srcVertex, graphname ) bforeach(const _GraphType::edge_descriptor& descname, graphname._outEdges( srcVertex ) )

	





#endif // GRAPH_H_












