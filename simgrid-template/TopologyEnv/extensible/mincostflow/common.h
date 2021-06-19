#ifndef _COMMON_H
#define _COMMON_H


#define BOOST_GRAPH_USE_NEW_CSR_INTERFACE
#include <iostream>
#include <vector>
#include <cmath>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/foreach.hpp>
#define bforeach         BOOST_FOREACH

#include "../Parameters.h"
#include "../Matrix.h"


//Maximum number of graph configurations
#define NCONFIGURATIONMAX 8
//Maximum number of steps in node configurations 
#define NSTEPMAX 4
//How many times we try to build a near-optimal solution before going greedy
#define NSOLUTIONTRY 10
#define EPSILON 0.1

namespace MinCost{

	typedef unsigned int IndexType;
	
	struct Demand{

		IndexType source;
		IndexType target;
		unsigned int demand;
		float distanceProduct;
	};


//	void pruneDemands(const std::vector< std::vector<MinCost::Demand> >& allDemands, std::vector< std::vector<MinCost::Demand> > currentDemands, IndexType firstPresent, IndexType lastPresent);

	bool compareDemandsByDecreasingDistance(const Demand& a, const Demand& b);
			
	std::vector<std::vector<unsigned int> > readConfigurationsDefinition(std::string confString);

	
	struct MetricElement;


	struct MetricEquation{
		unsigned int configurationIndex;
		MetricElement* data;

		//Short lifetime attributes (simplifies the algorithms)
		float delta; //The current value of the difference of the metric equation 
		unsigned int eq; //Equality value (i.e. smallest value where constraint is satisfied)
		float rate; //The rate at which equation changes, e.g. when the number of cables is increased on a specific edge
	};
	bool compareMetricsByConfiguration(const MetricEquation& a, const MetricEquation& b);

	struct ChangeElement;

//First is constraint pointer, second is the equality value (i.e smallest value where constraint is satisified)
	typedef std::pair<MetricEquation*, float> ActiveConstraint;
//Sort by configuration and then by equality value (used in 1.a part )
	bool compareActiveConstraints(const MetricEquation* a, const MetricEquation* b);

	template < typename T > inline  T tableMax(const T* table, unsigned int size){
		assert(size>0);
		//Init the max with first value, and this it later
		T val=*table;size--;table++;
		for(;size>0;size--,table++){
			if(*table>val)
				val=*table;
		}
		return val;	
	}

}

#endif // _COMMON_H
















