#include "optimizer.h"
#include "pathset.h"
#include "ConfigManager.h"


int main( int argc, char* argv[]){

	if(argc<1){
		std::cout<<" Usage "<<argv[0]<<": configurationFile"<<std::endl;
		return -2;
	}


	
	unsigned int lastYear=100;
	
	MinCost::Optimizer opt;
	MinCost::PathSet p;
	MinCost::ConfigManager config;

	
	for(unsigned int year=0; year<lastYear; year++){

		
		
		std::stringstream ss;ss<<"alloc_"<<year+1<<".in";
		std::stringstream ss2;ss2<<"post_"<<year<<".in";
		
		if(!config.load(ss.str().c_str(),((year==0)?NULL:ss2.str().c_str()),argv[1])){
			std::cerr<<"Could not read <"<<ss.str()<<">, ending optimization the dirty way!"<<std::endl;
			break;
		}

		assert(year==config.currentYear);

		//Instantiate the graph
		config.createGraph(opt.graph);
		opt.demands.swap(config.demands);
		
		opt.graph.save("init.graph");
		
		opt.prepare();

		//opt.run();

		opt.buildGreedy(false);
		
		std::cout<<"Year"<<year<<" costs "<<opt.getCost()<<std::endl;
		

	}

}

