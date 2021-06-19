#include "graph.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>



namespace boost{

	using namespace MinCost;

	typedef graph_traits<_GraphType> gt;
	template<typename Tag,typename _KeyType> struct Adaptor {
        typedef _KeyType KeyType;
        const _GraphType* g;
        typedef Tag tag;
        //Functor actually computing the value for each key
	    
        Adaptor():g(NULL){};
		Adaptor(const MinCost::_GraphType* g):g(g){}
        void set(const MinCost::_GraphType* g)
			{
                assert(this->g==NULL);
                this->g=g;
			}

		float getWeight(const KeyType& key){
return (*g)[key].length;
		}
	};
	typedef Adaptor<boost::edge_weight_t,gt::edge_descriptor> EdgeWeightMap;
	template<> struct property_traits<EdgeWeightMap>
	{
		typedef int value_type;//       The type of the property.
		typedef int& reference;//       A type that is convertible to the value type.
		typedef gt::edge_descriptor key_type; //The type of the key object used to look up the property. The property map may be templated on the key type, in which case this typedef can be void.
		typedef read_write_property_map_tag category; //The category of the property: a type convertible to readable_property_map_tag.
	};

EdgeWeightMap get(edge_weight_t t,const MinCost::_GraphType& g)
{
        return EdgeWeightMap(&g);
}


	float get(EdgeWeightMap map,property_traits<EdgeWeightMap>::key_type key)
	{
        return map.getWeight(key);
	}



	typedef property_map<MinCost::_GraphType, unsigned int MinCost::Vertex::*>::type VertexIndexMap;
	
	void _runDjisktra(MinCost::_GraphType& data, gt::vertex_descriptor source, std::vector<unsigned int>& predecessors, std::vector<float>& distance ){


		assert(false);
		//FIXME: For a reason the Dijkstra algorithm executes normally but does not modify the distance and predecessor vectors.. 
		
		VertexIndexMap vertexMap=get(&Vertex::index, data);
		property_map<MinCost::_GraphType, float MinCost::Edge::*>::type weightMap=get(&Edge::length, data);

		const unsigned int N=num_vertices(data);

		iterator_property_map<std::vector<float>::iterator,VertexIndexMap> distanceMap(distance.begin(), vertexMap);

		iterator_property_map<std::vector<unsigned int>::iterator,VertexIndexMap> predecessorMap(predecessors.begin(), vertexMap);

		std::vector<boost::default_color_type> colorVec(num_vertices(data), boost::color_traits<boost::default_color_type>::white());
		iterator_property_map<std::vector<boost::default_color_type>::iterator,VertexIndexMap> colorMap(colorVec.begin(), vertexMap);

//Init the color map to avoid to go through vertex with no capacity 
		for(unsigned int i=0;i<N;i++)
		{
			predecessors[i]=i;
			distance[i]=std::numeric_limits<float>::max();
			//	if(!data[i].isCrossable())
			//	colorMap[i]=boost::color_traits<int>::black();
		}

		boost::dijkstra_shortest_paths_no_init(data,source,
									   predecessorMap,
									   distanceMap,
									   weightMap,
									   vertexMap,
									   std::less<float>(),
									   boost::closed_plus<float>(),
											   (float)0,
									   boost::dijkstra_visitor<boost::null_visitor>(),colorMap);



	}


}

	
namespace MinCost{


	unsigned int Edge::nconfigurations=NCONFIGURATIONMAX;

/*
  void pruneDemands(const std::vector< std::vector<Demand> >& allDemands, std::vector< std::vector<Demand> > currentDemands, IndexType firstPresent, IndexType lastPresent){

  currentDemands.clear();

//Remove demands with both ends are either past or future
bforeach( const std::vector<Demand>& conf, allDemands){
currentDemands.push_back(std::vector<Demand>());
bforeach(const Demand& d, conf){
if( (d.source<firstPresent && d.target<firstPresent) ||  (d.source>lastPresent && d.target>lastPresent))
continue;
currentDemands.back().push_back(d);
}
			
}
}

*/
	
	bool compareDemandsByDecreasingDistance(const Demand& a, const Demand& b){
		return a.distanceProduct>b.distanceProduct;
	}

	
	bool compareActiveConstraints(const MetricEquation*  a, const MetricEquation* b){
		return a->configurationIndex < b->configurationIndex || (  a->configurationIndex == b->configurationIndex  &&  a->eq < b->eq  );
	}

	bool compareMetricsByConfiguration(const MetricEquation& a, const MetricEquation& b){
		return a.configurationIndex<b.configurationIndex;
	}


	void Graph::updateEdgesMarginFromConstraints(const std::vector<MetricEquation>& metrics){

		bforeach(const EdgeDescriptor& de,boost::edges(data)){			
			Edge& e=data[de];
			for(unsigned int i=0;i<MinCost::Edge::nconfigurations;i++)
				e.margin[i]=0;
		}
		bforeach(const MinCost::MetricEquation& m, metrics){
			//All constraints should be OK now
			assert(m.delta<=0);
			MinCost::MetricElement* el=m.data;
			while(el->lambda!=0){
				Edge& e=data[el->edgeDesc];
				unsigned int& margin=e.margin[m.configurationIndex];
				float eq=-m.delta/el->lambda;
				assert(eq>=0);
				margin=std::min(e.x[m.configurationIndex],std::max(margin,(unsigned int)eq));
			}
		}

	}

	void Graph::updateEdgesMarginFromVertices(){
		unsigned int sourceMargin[Edge::nconfigurations];
		unsigned int targetMargin[Edge::nconfigurations];
		bforeach(const EdgeDescriptor& de,boost::edges(data)){			
			Edge& e=data[de];
			data[boost::source(de,data)].margin(sourceMargin);
			data[boost::source(de,data)].margin(targetMargin);
			for(unsigned int i=0;i<MinCost::Edge::nconfigurations;i++)
				e.margin[i]=std::min(sourceMargin[i],targetMargin[i]);
				
		}
	}


		void Graph::initLengths(const MetricElement* m )
		{
			invalidatedSearch=true;
			bforeach(const EdgeDescriptor& de,boost::edges(data)){			
				data[de].length=0.0;
			}
			while(m->lambda!=0)
			{
				assert(m->lambda>0);
				data[m->edgeDesc].length=m->lambda;
				m++;
			}
			
		}

		void Graph::updateCapacity(const std::vector<ChangeElement>& changes)
		{
			bforeach(const ChangeElement& el, changes){
				data[el.edgeDesc].x[el.configurationIndex]=el.count;
			}
		}


	void Graph::_runPathSearch(unsigned int source,unsigned int target){

		if(invalidatedSearch || source !=lastSource || predecessors.empty())
		{
			const unsigned int N=boost::num_vertices(data);
			distance.resize(N);
			predecessors.resize(N);
			for(unsigned int i=0;i<N;i++)
			{
				predecessors[i]=i;
				distance[i]=std::numeric_limits<float>::max();
			}

		
			//Check for a route up to 2  edge (assuming that 3- and more routes will be longer or at least less desirable)..
			//In particular when patch panels are not used, most route will be two hops
			//The case for complex relaxation is not considered () 
			bforeach(const EdgeDescriptor& e,boost::out_edges(source,data)){
				unsigned int middle=opposite(e,source);
				if(data[e].length<distance[middle]){
						distance[middle]=data[e].length;
						predecessors[middle]=source;
				}
				if(data[middle].isCrossable()){
					bforeach(const EdgeDescriptor& e2,boost::out_edges(middle,data)){
						unsigned int second=opposite(e2,middle);
						if(second ==target ){
							if(data[e2].length+distance[middle]<distance[target]){
								distance[target]=data[e2].length+distance[middle];
								predecessors[target]=middle;
							}
						}
					
					}
				}
			}

			if(predecessors[target]!=target)
				return;



			invalidatedSearch=false;
			lastSource=source;
					
			save("djisktra.graph");
			_runDjisktra(data,source,predecessors,distance);

		}
	}


	

}













