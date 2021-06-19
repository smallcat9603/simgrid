#include "common.h"
#include "graph.h"

namespace MinCost{


	class Optimizer{

	public:

		//The graph representing possible paths for cables, augmented with the current solution state (c.f. graph.h)
		Graph graph;

		//Data representing the considered topologies, for the different configurations
		//Within each configuration, demands should be ordered as follows:
		//* Source id always inferior to target id
		//* Ordered by decreasing distance (makes sense to start optimize more distant demands)
        //
        // DO NOT TOUCH THE NUMBER OF ELEMENTS AFTER prepare() is called
		//
		std::vector< std::vector<Demand> >    demands;



		~Optimizer(){
			//Clean everything (at least try!)
			bforeach(MetricEquation& m,metrics)
			{
				delete [] m.data;
			}
		}


		void prepare(){

			metrics.clear();
			change.clear();
			bestChange.clear();

			startPartition.clear();
			tmpElement.clear();
			tmpIndex.clear();
			tmpPath.clear();
			actives.clear();

			unsigned int NVertices=graph.getNVertices();
	
			for(unsigned int i=0;i<NVertices;i++)
				startPartition.push_back(i);

			gamma=(1+EPSILON)/std::pow( (float)(1+EPSILON)*NVertices,(float)(1.0/EPSILON));
			remainingSolutionTry=NSOLUTIONTRY;

		}
		
		void run(){

			prepare();
		
			bool stop=false;

			while(!stop){
		
				//---------------------------------------------------------------------------------------------------------------------
				//1a) Add segment capacity, until ALL current metric constraints are OK
				// First failMeasure is defined as sum of the overflow ff of metrics m:
				// ff(m)=sum( shortestLength(m,k)*d_k, demand k) - sum(x_u*lambda_u, edge u)
				// failMeasure=sum( max(0,ff(m)) , metrics m)
				// Increase the capacity of links in order to maximize the efficiency rho=failMeasure/cost=F/C of the configuration
				// The gradient can be written drho/dx=(F2-F)/(C2-C)= sum ( lambda * (x2-x) , metrics ) /(cost(x2) - cost(x)), if ff>>0  
				//First case: Only links (and their associated linear cost) are augmented 
				//    If cost is linear to x2-x (i,e ff>>0), we get  drho/dx=sum ( lambda, metrics )/costrate
				//    Since F and cost have non-linearity ( resp. saturation at 0 and maximum amongst configurations), the gradient decreases as soon as a metric becomes satisfied. In addition, we insert a minimum value to cost difference of 0.5, so that cables slack are attributed in priority, but still following failure measure differential. 
				//    Now how to select a new fraction a2/b2 to add to existing fraction a/b, in order to maximize gain (i.e. (a+a2)/(b+b2) ? 
				//    It is profitable to add (a2,b2) when (a+a2)/(b+b2)>a/b <=> (a+a2)*b>a*(b+b2) <=> a2*b>a*b2 <=> a2/b2 > a/b
				//    For each given edge, the algorithm is then the following:
				//    1) Initialize gain at minimum of slack limit or min(fail/lambda, metrics) as a= sum ( lambda, metrics ), b=(costrate*max(.5 , x2-sat(x) ))
				//    2) Increase gain to slack limit or metric satisfaction thresholds, and update a and b as long as ratio improves. Since x increases, metrics get satisfied, and node limits increase, we stop someday 
				//Second case: One node is promoted and connected edges are scaled-up to maximize gain
				//    The same base equation holds, with addition of the node malus: drho/dx= sum ( lambda * (x2-x) , metrics ) /(costrate*(x2 - x) + nodedcost )
				//    Logically, we try to balance additionnal cost incurred by node upgrade by increasing multiple edges (x_i)  at once:
				//    drho/dx= sum(sum ( lambda * (x2-x) , metrics ), x_i) /(costrate*sum(x2 - x , x_i ) + nodedcost )
				//    Hence, if we define for each edge  a=sum ( lambda * (x2-x) , metrics )  and b=costrate*(x2 - x), we get following algorithm:
				//    1) Order edges by decreasing ratio a/b
				//    2) Add edges as long as sum(a)/(sum(b)+nodedcost)<a2/b2
				//---------------------------------------------------------------------------------------------------------------------

				bool metricOk;
				do{

					metricOk=true;

					//Compute the result of each metric equation
					//O(metric*(graph.Init+demand*graph.Distance+edge ) )
					float failMeasure=0.0;
					std::sort(metrics.begin(),metrics.end(),compareMetricsByConfiguration);
					bforeach(MetricEquation& m, metrics){
						m.delta=computeMetricDelta(m);
						if(m.delta>0){
							failMeasure+=m.delta;
							metricOk=false;
						}
					}


					float cost=_updateCost();
					std::cout <<"Step 1, cost:"<<cost<<", remainingTry:"<<remainingSolutionTry<<", metrics:"<<metrics.size()<<std::endl;					

					//Propose one candidate for each node (promotion to next step + amortization by cables margin around) and one per cable (respecting both margin and maximizing constraints)
					unsigned int eq;
					float currentRatio[Edge::nconfigurations];
					float baseRatio[Edge::nconfigurations];
					float bestRatio=-1;
					unsigned int currentX[Edge::nconfigurations];
					bestChange.clear();
					bforallEdges(de,graph){
						Edge& e=graph(de);
						actives.clear();
						for(unsigned int i=0;i<Edge::nconfigurations;i++){
							currentRatio[i]=0.0;
							currentX[i]=e.x[i];
						}
						graph.updateEdgesMarginFromVertices();
						bforeach(MetricEquation& m, metrics){
							//Anyway this constraint is OK already
							if(m.delta<=0)
								continue;
							MetricElement* el=m.data;
							while(el->lambda!=0){
								if(el->edgeDesc==de)
								{
									eq=(unsigned int)e.x[m.configurationIndex]+std::ceil(m.delta/el->lambda);
									if(eq<e.margin[m.configurationIndex]){
										actives.push_back(&m);
										m.rate=el->lambda;
										m.eq=eq;
									}
									currentRatio[m.configurationIndex]+=el->lambda;
									break;
								}
							}
						}

						//sort by configuration and then value
						std::sort(actives.begin(),actives.end(),compareActiveConstraints);
						for(unsigned int i=0;i<Edge::nconfigurations;i++)
							currentRatio[i]=baseRatio[i];
				
						//TODO: Increase currentX until currentRate decreases 


						float totalRatio=0;
						for(unsigned int i=0;i<Edge::nconfigurations;i++){
							totalRatio+=currentRatio[i];
						}
						totalRatio/=std::max((float)0.5,tableMax(currentRatio,Edge::nconfigurations)-tableMax(baseRatio,Edge::nconfigurations));

						if(totalRatio>bestRatio)
						{
							change.clear();
							for(unsigned int i=0;i<Edge::nconfigurations;i++)
								if(currentRatio[i]>e.x[i])
								{
									change.push_back(ChangeElement());
									change.back().edgeDesc=de;
									change.back().configurationIndex=i;
									change.back().count=currentRatio[i];
								}
							bestChange.swap(change);
							bestRatio=totalRatio;
						}	
					}//End of case for increase of individual edges

			
					//Increase node step
					//First, compute the expected gain
					bforallVertices(dv2, graph){
						graph(dv2).ratio=0.0;
					}
			
					bforeach(MetricEquation& m, metrics){
						MetricElement* el=m.data;
						while(el->lambda!=0){
							eq=std::ceil(m.delta/el->lambda);//(unsigned int)e.x[m.configurationIndex]+
							Vertex v=graph(graph.source(el->edgeDesc));
							if(eq>v.margin(m.configurationIndex))
								v.ratio+=el->lambda*(eq-v.margin(m.configurationIndex));
							Vertex v2=graph(graph.target(el->edgeDesc));
							if(eq>v2.margin(m.configurationIndex))
								v2.ratio+=el->lambda*(eq-v2.margin(m.configurationIndex));
						}
					}

					bforallVertices(dv, graph){
						Vertex& v=graph(dv);
						if(v.ratio/v.stepCost(1) > bestRatio){
							//TODO: Create a new change set with corresponding edges
						}		
					}

				} while(!metricOk);

				graph.updateCapacity(bestChange);

				//---------------------------------------------------------------------------------------------------------------------
				//1b) Remove unnecessary capacities
				// Direct cable changes by the saving in cable + profit-sharing to node saving (across configurations)
				// For each edge, proposition is then either the metric limit satisfaction (xl, (x-xl)*(costrate+nodedcost/(nodeload)) ) or the node degradation limit (xd, (x-xd)*(costrate+nodedcost/(nodeload)) )
				//---------------------------------------------------------------------------------------------------------------------
				float dcost;
				do{
					dcost=0.0;
			
					//Compute the margin of each edge and constraint
					graph.updateEdgesMarginFromConstraints(metrics);
			
					float currentCost;
					unsigned int currentDegree;
					Graph::EdgeDescriptor bestEdge;
					bforallEdges(de,graph){
						Edge& e=graph(de);
						
						currentDegree=0;
						for(unsigned int i=0;i<Edge::nconfigurations;i++)
							currentDegree=std::max(currentDegree,e.x[i]-e.margin[i]);

						unsigned int baseDegree=tableMax(e.x,Edge::nconfigurations);
						currentCost=(baseDegree-currentDegree)*e.rate;

						//TODO: Also include the anticipation for step reduction

						if(currentCost>dcost)
						{
							bestEdge=de;
							dcost=currentCost;
						}
					}

					//Apply the best reduction
					if(dcost>0)
						for(unsigned int i=0;i<Edge::nconfigurations;i++)
							graph(bestEdge).x[i]-=graph(bestEdge).margin[i];
			
				}while(dcost>0);
	
				//---------------------------------------------------------------------------------------------------------------------
				//2) Try to generate a set of violated bipartition constraints  
				//---------------------------------------------------------------------------------------------------------------------
				bool foundviolatedPartitionConstraint=false;
				for(unsigned int confIndex=0 ; confIndex<demands.size() ; confIndex++){
					//Need to get all segments covered by an bipartition equality. 
					bool uncoveredSegment;
					for(;;){

						//O(edge+node)
						//Randomize the start partition...
						unsigned j,k,tmp;
						for(unsigned int i=0 ; i<startPartition.size()/2 ;i++){
							j=rand()%startPartition.size();
							k=rand()%startPartition.size();
							tmp=startPartition[j];
							startPartition[j]=startPartition[k];
							startPartition[k]=tmp;
						}
						//...and decide that first half is the first partition
						unsigned int i;
						for(i=0 ; i<startPartition.size()/2 ;i++)
							graph(startPartition[i]).partition=0;
						for( ; i<startPartition.size();i++)
							graph(startPartition[i]).partition=1;

						//Make sure that at least one new segment will be visited (by freezing it and swapping the partition of one if necessary)
						uncoveredSegment=false;
						bool needSwap=true;
						unsigned int frozenNodeA,frozenNodeB; 
						bforallEdges(de,graph){
							Edge& e=graph(de);
						
							if(e.visited)
								continue;
							uncoveredSegment=true;
							if(!needSwap)
								continue;
							frozenNodeA=graph.source(de);
							frozenNodeB=graph.target(de);
							if(graph(frozenNodeA).partition!=graph(frozenNodeA).partition)
								needSwap=false;
						}
	    
						if(!uncoveredSegment)
							break;
	
						if(needSwap)
							graph(frozenNodeA).partition=!graph(frozenNodeA).partition;
						tmpIndex.push_back(frozenNodeA);
						tmpIndex.push_back(frozenNodeB);

						//TODO: Change the partitions according to algo in litterature, storing visited nodes in tmpIndex


						//Remember the tested segments (visit property), check if generated constraint is violated and memorize partition segments for constraints generation
						tmpElement.clear();
						bforallEdges(de2,graph){		
						Edge& e2=graph(de2);
							if(graph(graph.source(de2)).partition!=graph(graph.target(de2)).partition){
								//Found a partition segment
								tmpElement.push_back(MetricElement());
								tmpElement.back().edgeDesc=de2;
								tmpElement.back().lambda=1.0;
								e2.visited=1;
							}
						}
						//Terminal element
						tmpElement.push_back(MetricElement());

						foundviolatedPartitionConstraint=foundviolatedPartitionConstraint || !addConstraintIfViolated(confIndex,tmpElement);

					}
				}

    
				//---------------------------------------------------------------------------------------------------------------------
				//3) If could not find violated bipartition constraints, try to build a solution
				// Lets consider a simple linear cost flow problem with set capacity. 
				// -> Each configuration is computed independently
				// -> The cost of patch-panel is not included
				// -> Use the (1+epsilon) Garg-Konemann heuristic based on exponential congestion of links 
				// 
				//---------------------------------------------------------------------------------------------------------------------
				bool builtSolution=true;
				if(!foundviolatedPartitionConstraint){

					buildGreedy(remainingSolutionTry-- > 0);
					
				}

				stop=builtSolution;
			}





		}



		//If result (theta) is strictly positive, constraint is violated
		float computeMetricDelta(MetricEquation& m){
			return computeMetricDelta(m.configurationIndex,m.data);
		}

		float computeMetricDelta(unsigned int confIndex, const MetricElement* data){
			//Compute theta for metric m
			float theta=0.0;
			//Init the graph with lengths from the metric (i.e. lambdas)
			graph.initLengths(data);
			bforeach(Demand& d, demands[confIndex]){
				theta+=graph.getLength(d.source,d.target)*d.demand;	      
			}
			//Compute the weighted sum of capacities
			float capacities=0.0;
			const MetricElement* el=data;
			while(el->lambda!=0){
				capacities+=graph(el->edgeDesc).x[confIndex]*el->lambda;
			}
			return theta-capacities;
		}

		
		bool addConstraintIfViolated(unsigned int confIndex,std::vector<MetricElement>& tmpElement){
			//Add a terminal element, in case..
			tmpElement.push_back(MetricElement());

			 
			if(computeMetricDelta(confIndex,&(tmpElement.at(0)))<=0)
				return false;
			
			
			metrics.push_back(MetricEquation());
			metrics.back().configurationIndex=confIndex;
			metrics.back().data=new MetricElement[tmpElement.size()];
			for(unsigned int i=0;i<tmpElement.size();i++)
				metrics.back().data[i]=tmpElement[i];

			return true;
		}


		void buildGreedy(bool stopOnViolatedConstraint){


			//unsigned int metricIndexes[demands.size()];
			

			const unsigned int AdjustementQuantity=1;

			
			std::vector<MetricElement> tmpElement;
			
			for(unsigned int confIndex=0 ; confIndex<demands.size() ; confIndex++){
						//Initialize lengths and usage 
						graph.initLengths(gamma);
						bforallEdges(de,graph){		
							Edge& e=graph(de);
							e.visited=e.x[confIndex];
							e.length*=e.rate;
						}
		    
						bforeach(Demand& d, demands[confIndex]){

							unsigned int currentDemand=d.demand;

							do{
								unsigned int maxFlow=currentDemand;
								tmpPath.clear();
								graph.getPath(d.source,d.target,tmpPath);
								bforeach(Graph::EdgeDescriptor& de2 , tmpPath){
									if(graph(de2).visited==0){
										//Got an invalid set of capacities (and all links is overflow for a demand )
										if(stopOnViolatedConstraint)
										{
											//Build a minimal set of elements with current lengths set
											float minLength=0;
											bforallEdges(de3,graph){
												Edge& e3=graph(de3);
												if(e3.length>0 && e3.length<minLength)
														minLength=e3.length;
											}
											tmpElement.clear();
											bforallEdges(de4,graph){
												Edge& e4=graph(de4);
												if(e4.length>minLength){
													tmpElement.push_back(MetricElement());
													tmpElement.back().lambda=e4.length-minLength;
													tmpElement.back().edgeDesc=de4;
												}
											}
											tmpElement.push_back(MetricElement());
											
											addConstraintIfViolated(confIndex,tmpElement);											
											return ;

											
										}else{
											
											//Adjust the capacities to allow this to work
											bforeach(Graph::EdgeDescriptor& de3 , tmpPath){
												Edge& e3=graph(de3);
												if(e3.visited==0)
												{
													e3.visited+=AdjustementQuantity;
													e3.x[confIndex]+=AdjustementQuantity;
												}
											}
										}

									}
									if(maxFlow>graph(de2).visited)
										maxFlow=graph(de2).visited;
								}
								currentDemand-=maxFlow;
								bforeach(Graph::EdgeDescriptor& de3 , tmpPath){
									graph(de3).visited-=maxFlow;
									graph(de3).length*=(1+EPSILON*maxFlow/graph(de3).x[confIndex]);
								}

			
							} while(currentDemand>0);
						   
						}
					}

			

		}

		float getCost(){

			return _updateCost();
			
		}


	private:
	
		//Data representing the metric equations (empty at first)
		std::vector<MetricEquation> metrics;

		//Temporary vector of changes in the solution for local heuristic changes
		std::vector<ChangeElement> change,bestChange;

		//Algorithmic vectors:
		//Vector for random  partition selection 
		std::vector<unsigned int> startPartition;
		std::vector<MetricElement> tmpElement;
		std::vector<unsigned int> tmpIndex;
		std::vector<Graph::EdgeDescriptor> tmpPath;
		std::vector<MetricEquation*> actives;

		float gamma;

		unsigned int remainingSolutionTry;


		//O(edges*configurations)
		float _updateCost(){
			float cost=0.0;
			unsigned int maxCount,tmp;
			bforallVertices(dv,graph){
				Vertex& v=graph(dv);
				for(unsigned int i=0;i<Edge::nconfigurations;i++)
					v.degreeCount[i]=0;
				bforallOutEdges(de,dv,graph){
					Edge& e=graph(de);
					maxCount=0;
					for(unsigned int i=0;i<Edge::nconfigurations;i++)
					{
						tmp=e.x[i];
						if(tmp>maxCount)
							maxCount=tmp;
						v.degreeCount[i]+=tmp;
					}  
					//Half cost for cables
					cost+=maxCount*e.rate/2;
				}
				//Compute cost for the patch-panel
				cost+=v.stepCost();
			}
			return cost;
					
		}


	};

};
