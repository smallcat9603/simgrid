using namespace std;
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <limits>

#include "CostManager.h"


/****************************************************************/
/*
 Solver for Quadratic Bottleneck Assignment Problem
 by Ikki Fujiwara <ikki@nii.ac.jp>  2012/09/12
 based on:

 This programme implement a simulated annealing for the
 quadratic assignment problem along the lines describes in
 the article D. T. Connoly, "An improved annealing scheme for
 the QAP", European Journal of Operational Research 46, 1990,
 93-100.

 Compiler : g++ or CC should work.

 Author : E. Taillard,
 EIVD, Route de Cheseaux 1, CH-1400 Yverdon, Switzerland

 Date : 16. 3. 98

 Format of data file : Example for problem nug5 :

 5

 0 1 1 2 3
 1 0 2 1 2
 1 2 0 1 2
 2 1 1 0 1
 3 2 2 1 0

 0 5 2 4 1
 5 0 3 0 2
 2 3 0 0 0
 4 0 0 0 5
 1 2 0 5 0

 Additionnal parameters : Number of iterations, number of runs

 */

/********************************************************************/

const long Infinite = (std::numeric_limits<Val>::max)();
long nb_iter_initialisation = 1000; // Connolly proposes nb_iterations/100

//Start execution time for time-limited computation
time_t startTime;

bool timeout(double limit)
{
	//if limit is zero, limit is deactivated.
	if(limit==0)
		return false;

	  time_t now;
	   time(&now);
	double seconds = difftime(now, startTime);
	//std::cout<<seconds << "elapsed..\n";
	return seconds>limit;
}
double current_time() {
	return (double(clock()) / double(1000));
}

Val unif(Val low, Val high);
double my_rand();

/************************** sa for qap ********************************/

/*long calc_delta_cost(long n, type_matrice & a, type_matrice & b,
 type_vecteur & p, long r, long s, long cost) {
 long d1;
 long cost1 = 0;
 long pi, pj;

 for (long i = 1; i <= n; i++) {
 for (long j = 1; j <= n; j++) {
 pi = (i == r) ? p[s] : (i == s) ? p[r] : p[i];
 pj = (j == r) ? p[s] : (j == s) ? p[r] : p[j];
 cost1 = max(cost1, a[i][j] * b[pi][pj]);
 }
 }
 d1 = cost1 - cost;

 return(d1);
 }*/

long no_res, best_res, best_iter=0;

std::vector<Mat> best_p;
Val bestCost=Infinite;
Val bestCableCost=Infinite;
Val bestEnergyCost=Infinite;
Val bestCableLength=Infinite;

void printSolution(std::vector<Mat> best_p, Val bestCost, Val bestCableCost, Val bestEnergyCost, Val 	bestCableLength){
	for(unsigned int t=0;t<best_p.size();t++)
	{
		std::cout<<'@'<<best_iter<<" "<<bestCost<<" "<<bestCableCost<<" "<<bestEnergyCost<<" "<<bestCableLength<<" "<<t<<" [";
		 best_p[t].toVectorStream(std::cout,",");
		 std::cout<<"]\n";		 
	}

}

void annealing(CostManager& costManager , long nb_iterations, double limit, std::ostream& paretoStream) {

        nb_iter_initialisation =10; // mymax<long>(100,nb_iterations/100);

	bool foundBetterSolution=false;

	const unsigned int n=costManager.getN();

	long ntop,ti,r, s;

	long delta;
	long k = n * (n - 1) / 2, mxfail = k, nb_fail, no_iteration;
	long dmin = Infinite, dmax = 0;
	double t0, tf, beta, tfound, temperature;
	//long best_iter;

	//double cpu = current_time();

	const unsigned int FirstMovable=costManager.getNPastCabinets();

	//Gradient descent, no temperature
	for (no_iteration = 1; no_iteration <= nb_iter_initialisation;
			no_iteration = no_iteration + 1) {

		//std::cout << "Iteration "<<no_iteration<<std::endl;
					
		if(no_iteration%100==0 && timeout(limit))
			break;

		ti=costManager.getRandomTopologyIndex(my_rand());
		ntop=costManager.getTopologySize(ti);
		r = unif(FirstMovable, ntop - 1);
		s = unif(FirstMovable, ntop - 2);
		if (s >= r)
			s = s + 1;

		delta = costManager.computeDelta(ti,r, s);
		if (delta > 0) {
			dmin = mymin(dmin, delta);
			dmax = mymax(dmax, delta);
		};
		costManager.commitSwap(ti,r, s,false);
	};

	//Initialize temperature
	t0 = dmin + (dmax - dmin) / 10.0;
	tf = dmin;
	beta = (t0 - tf) / (nb_iterations * t0 * tf);

	nb_fail = 0;
	tfound = t0;
	temperature = t0;
	ti=0;
	while(costManager.getTopologySize(ti)<=FirstMovable)
		ti++;
	r = FirstMovable;
	s = FirstMovable+1;
	for (no_iteration = 1;
			no_iteration <= nb_iterations - nb_iter_initialisation;
			no_iteration = no_iteration + 1) {
		temperature = temperature / (1.0 + beta * temperature);


		if(no_iteration%100==0 && timeout(limit))
			break;

		ntop=costManager.getTopologySize(ti);
		s = s + 1;
		if (s >= ntop) {
			r = r + 1;
			if (r > ntop - 2)
			{
				r = FirstMovable;
				ti= ti +1;
				if(ti>= costManager.getNTopologies())
					ti=0;
				while(costManager.getTopologySize(ti)<=FirstMovable)
					ti++;
			}
			s = r + 1;
		};

		delta = costManager.computeDelta(ti,r, s);
		if ((delta < 0) || (my_rand() < exp(-double(delta) / temperature))
				|| mxfail == nb_fail) {

			if (costManager.commitSwap(ti,r, s)) {
				tfound = temperature;
				best_res = no_res;
				best_iter = no_iteration;

				if(costManager.getCurrentCost()<bestCost)
				{
					bestCost=costManager.getCurrentCost();
					foundBetterSolution=true; //dont copy solution right away, may be too heavy (and unefficient)
					bestCableCost=costManager.getCableCost();
					bestEnergyCost=costManager.getEnergyCost();
					bestCableLength=costManager.getCableLength(0);

					std::cout << "Improving cost to (tot"<< bestCost<<",cbl"<<bestCableCost<<",ene"<<bestEnergyCost<<") at res="<< no_res << " temperature="<<temperature<< " best_iter="<<best_iter<<std::endl;
					paretoStream<< no_res << "\t;"<<temperature<< "\t;"<<best_iter<<"\t;"<<bestCableCost<<"\t;"<<bestEnergyCost<<"\t;";
					costManager.getStageCableCost(costManager.RedundancyCoeff).toVectorStream(paretoStream,";");					
					
				}

			};

			nb_fail = 0;
		} else
			nb_fail = nb_fail + 1;

		if (mxfail == nb_fail) {
			beta = 0;
			temperature = tfound;
		};

	};


	if(foundBetterSolution)
		best_p=costManager.getBestSolution();

}

int main(int argc, char *argv[]) {

	if (argc < 8) {
		printf("QAP solver v2.01 (%s) by Ikki/Fabien\n", __DATE__);
		printf("Usage: %s #iteration #trial #time_limit_seconds #costCoeff #energyCoeff #redundancyCoeff  qapFileName\n", argv[0]);
		return EXIT_FAILURE;
	}

	long nb_iterations = atoi(argv[1]);
	long nb_res = atoi(argv[2]);
	double maxSeconds = atof(argv[3]);
	float costCoeff = atof(argv[4]);
	float energyCoeff = atof(argv[5]);
	float redundancyCoeff = atof(argv[6]);
	//argv[7] is the input data file
	double sqMax=std::sqrt(maxSeconds);


	time(&startTime);

	if (nb_iterations<=0 || nb_res<=0 ) {
			printf("Parameter 1 (Nb iterations) and parameter 2 (nb reset) should be strictly positive, use time-limited simulations instead \n");
		}

	if (costCoeff<0 || energyCoeff<0 || redundancyCoeff<0 || costCoeff+energyCoeff+redundancyCoeff==0) {
			printf("Parameter 3 (Cost coeff.), parameter 4 (Energy coeff.) and parameter 5 (Redundancy coeff.) should be positive, and at least one not zero.\n");
			return EXIT_FAILURE;
		}


	std::ofstream paretoStream;
	if(argc>8 && std::string(argv[8])=="clean"){
		paretoStream.open("pareto.csv");
		paretoStream<< "#NRes\tTemp\tIter\tCable\tEner\tRed\t"<<std::endl;
		paretoStream<< "\"C"<<costCoeff<<",E"<<energyCoeff<<",R"<<redundancyCoeff<<"\""<<std::endl;
	}else{
		paretoStream.open("pareto.csv",std::ios::app);
		//Add empty lines for gnuplot to detect a different dataset
		paretoStream<<std::endl<<std::endl;
		paretoStream<< "\"C"<<costCoeff<<",E"<<energyCoeff<<",R"<<redundancyCoeff<<"\""<<std::endl;
	}


	CostManager costManager(costCoeff,energyCoeff,redundancyCoeff,argv[7]);
	costManager.init();
	best_p=costManager.getBestSolution();
    bestCost=costManager.getBestCost();
	bestCableCost=costManager.getCableCost();
    bestEnergyCost=costManager.getEnergyCost();
	bestCableLength=costManager.getCableLength(0);
		
	printf("Running allocation for year %d of %d\n",costManager.currentYear+1,costManager.getNYears());
	printf("Initial permutation gives Energy:%f and Cable:%f\n",costManager.getEnergyCost(),costManager.getCableCost());
	
	if(costManager.currentYear <costManager.getNYears())	
	{
		for (no_res = 1; no_res <= nb_res; no_res = no_res + 1){
	
		  	annealing(costManager, nb_iterations,mymin<double>(sqMax*no_res,maxSeconds),paretoStream);

			//Reset cost manager for next trial
			costManager.init();
	
			if(timeout(maxSeconds))
				break;
		};
	}

	//std::cout<<"Finished simulations "<<std::endl;

	printSolution(best_p, bestCost, bestCableCost, bestEnergyCost, bestCableLength);

	//Revert costmanager for the best case
	costManager.setPermutations(best_p);

	paretoStream<< " \t; \t; \t;"<<bestCableCost<<"\t;"<<bestEnergyCost<<"\t;";
	costManager.getStageCableCost(costManager.RedundancyCoeff).toVectorStream(paretoStream,";");

	costManager.save();
	
	paretoStream.close();
	
	return 0;
}

/************* random number generators ****************/

const long m = 2147483647;
const long m2 = 2145483479;
const long a12 = 63308;
const long q12 = 33921;
const long r12 = 12979;
const long a13 = -183326;
const long q13 = 11714;
const long r13 = 2883;
const long a21 = 86098;
const long q21 = 24919;
const long r21 = 7417;
const long a23 = -539608;
const long q23 = 3976;
const long r23 = 2071;
const double invm = 4.656612873077393e-10;
long x10 = 12345, x11 = 67890, x12 = 13579, x20 = 24680, x21 = 98765, x22 =
		43210;

double my_rand() {
	long h, p12, p13, p21, p23;
	h = x10 / q13;
	p13 = -a13 * (x10 - h * q13) - h * r13;
	h = x11 / q12;
	p12 = a12 * (x11 - h * q12) - h * r12;
	if (p13 < 0)
		p13 = p13 + m;
	if (p12 < 0)
		p12 = p12 + m;
	x10 = x11;
	x11 = x12;
	x12 = p12 - p13;
	if (x12 < 0)
		x12 = x12 + m;
	h = x20 / q23;
	p23 = -a23 * (x20 - h * q23) - h * r23;
	h = x22 / q21;
	p21 = a21 * (x22 - h * q21) - h * r21;
	if (p23 < 0)
		p23 = p23 + m2;
	if (p21 < 0)
		p21 = p21 + m2;
	x20 = x21;
	x21 = x22;
	x22 = p21 - p23;
	if (x22 < 0)
		x22 = x22 + m2;
	if (x12 < x22)
		h = x12 - x22 + m;
	else
		h = x12 - x22;
	if (h == 0)
		return (1.0);
	else
		return (h * invm);
}

long unif(long low, long high) {
	return (low + long(double(high - low + 1) * my_rand()));
}

/******************************************************/

