/*
 * CostManager.h
 *
 *  Created on: Feb 24, 2014
 *      Author: chaix
 */

#ifndef COSTMANAGER_H_
#define COSTMANAGER_H_

#include "Config.h"


class CostManager: public Config {
public:

	const float CostCoeff;
	const float EnergyCoeff;
	const float RedundancyCoeff;
	
	
CostManager(float CostCoeff, float EnergyCoeff, float RedundancyCoeff, char* inputFileName) :
			CostCoeff(CostCoeff), EnergyCoeff(EnergyCoeff),RedundancyCoeff(RedundancyCoeff), bestCost(0), currentCost(0)
			{
				assert(load(inputFileName));
			}

	virtual bool load(const char* filename ){

		if(!Config::load(filename))
			return false;

		unsigned int years=getNYears();
		unsigned int NMax = A.back().Size;
		for(unsigned int y=0;y<years;y++)
			partialCableCost.push_back(FloatMat(NMax,NMax,0,true));
		currentCableCost=FloatMat(years);

		return true;
	}

	void save(){

		Config::save();
		
		//Save edge sets for random topologies generation
		for(unsigned int y=0;y<A.size();y++){
			std::stringstream ss2; ss2<<"all_"<<y+1<<".edges";
			std::ofstream outFile2(ss2.str().c_str());
			const unsigned int NRacks=A[y].Size;
			for (unsigned int i = 0; i < NRacks; i++)
				for (unsigned int j = i; j < NRacks; j++)
				{	
					float ncables=_getPairCableCount(convertId(i,y),convertId(j,y),T.size(),0,0,2,RedundancyCoeff);
					if(ncables>0)
						outFile2 << i <<"\t"<<j <<"\t"<<ncables<<std::endl;
				}
			outFile2.close();

			std::stringstream ss3; ss3<<"histo_"<<y+1<<".dat";
			std::ofstream outFile3(ss3.str().c_str());
			FloatMat values=_getHistogram(y);
			outFile3 << "#Distance\t,\tCount\t,\tBackup"<<std::endl;	
			for(unsigned int l=0;l+1<values.getRows();l++){
					outFile3 << values(l,0) <<"\t,\t"<< values(l,1)  <<"\t,\t"<<  values(l,2)  <<std::endl;	
			}
			outFile3 << "#Total:"<< values(values.getRows()-1,0) <<"\t,\t"<< values(values.getRows()-1,1)  <<"\t,\t"<<  values(values.getRows()-1,2)  <<std::endl;	
			outFile3.close();			
			
		}
		
		//Write (approximate) edge files foreach input topology
		const unsigned int NSwitches=4;
		for(unsigned int t=0;t<T.size();t++)
		{
			Topology& topo=T[t];
			sortConversionList(topo.year);
			CabinetYear& cab=A[topo.year];
			std::stringstream ssTmp; ssTmp<<topo.name<<".alloc_"<<currentYear<<".edges";
			std::ofstream outFileTop(ssTmp.str().c_str());
			const unsigned int NRacks=cab.Size;
			float IntraCabinetLength=Parameters::cableLength(0);
			for (unsigned int i = 0; i < NRacks; i++)
			{
				//Full-connectivity in cabinets
				for (unsigned int k = 0; k < NSwitches; k++)
					for (unsigned int l = k+1; l < NSwitches; l++)
						outFileTop<<i*NSwitches+k<<"\t"<<i*NSwitches+l<<"\t"<<IntraCabinetLength<<std::endl;

				Position posi=conversionList[i].position;				
				for (unsigned int j = i; j < NRacks; j++)
				{
					unsigned int nconnect=topo.finalConnectivity(topo.permutation(i),topo.permutation(j));
					if(nconnect==0)
						continue;
					
					Position posj=conversionList[j].position;				
					float rackDistance=(posi-posj).getNorm();					

					unsigned int start1=rand();
					unsigned int start2=rand();
					for (unsigned int k = 0; k < nconnect; k++)
					
					outFileTop<< i*NSwitches+ ((k+start1)%NSwitches) <<"\t"<<j*NSwitches+ ((k+start2)%NSwitches) <<"\t"<<Parameters::cableLength(rackDistance)<<std::endl;
					
					
				}

			}
			
			outFileTop.close();		
		}
		 
		
		//Write a ruby file defining all the obtained results 
		std::ofstream outFile4("_result.rb");
		outFile4<<"def results\n\tres=Hash.new\n\t";
		writeVariable("totalEnergy", getEnergyCost(),outFile4);
		writeVariable("totalCost", getCableCost(),outFile4);
		writeVariable("totalLength", getCableLength(RedundancyCoeff),outFile4);
		writeVariable("stageCost", getStageCableCost(RedundancyCoeff) ,outFile4);
		writeVariable("stageBackupCost", getStageCableCost(RedundancyCoeff) - getStageCableCost(0) ,outFile4);
		writeVariable("stageCostHisto", _getHistogram(currentYear),outFile4);		
		outFile4<<"\tres\nend\n";
		outFile4.close();
	}

	float getEnergyCost() {
		float tmp,res=0;
		for (unsigned int ti = 0; ti < T.size(); ti++){
			Topology& t = T[ti];
			const unsigned int Ntop = A[t.year].Size;
			for (unsigned int i = 0; i < Ntop; i++)
				for (unsigned int j = 0; j < Ntop; j++){				
					tmp= _E(convertId(i,t.year),convertId(j,t.year))	* t.finalConnectivity(t.permutation(i),t.permutation(j));
					if(tmp>0){
						//std::cout<<"Energy:"<<i<<"<->"<<j<<"gives "<<tmp<<std::endl;
					}		
					res+=tmp;
				}
		}
		return res;
	}

	//Return the aggregated cable length in meters 
	float getCableLength(float redundancyCoeff) {

		unsigned int NMax = getN();
		unsigned int NYears = getNYears();
		FloatMat maxi(NYears);
		float totalLength=0;

		sortConversionList(A.size()-1);
		
		//i,j represent the position of a couple of cabinets
		unsigned int immediate;
		for (unsigned int i = 0; i < NMax; i++) {
			for (unsigned int j = i ; j < NMax; j++) {
				immediate = (conversionList[i].year <= currentYear) ? 1 : 0;
				immediate += (conversionList[j].year <= currentYear) ? 1 : 0;

				totalLength+=_getPairCableCount(i,j,T.size(),0,0, immediate, redundancyCoeff)* _L(i,j);

			}
		}
	
		return totalLength;

	}


	const FloatMat& getStageCableCost(float redundancyCoeff) {
		
		unsigned int NMax = getN();
		unsigned int NYears = getNYears();
		FloatMat maxi(NYears);
		currentCableCost.reset(0);
		
		//i,j represent the position of a couple of cabinets
		unsigned int immediate;
		for (unsigned int i = 0; i < NMax; i++) {
			for (unsigned int j = i ; j < NMax; j++) {
				immediate = (conversionList[i].year <= currentYear) ? 1 : 0;
				immediate += (conversionList[j].year <= currentYear) ? 1 : 0;

				 _getPairCableCost(maxi,conversionList[i].finalId,conversionList[j].finalId, T.size(),0,0,immediate,redundancyCoeff);

				//Save value for future computations	
				for(unsigned int y=0;y<NYears;y++){
					partialCableCost[y](conversionList[i].finalId,conversionList[j].finalId)=maxi(y);
					currentCableCost(y) += maxi(y);
				}
			}
		}
	
		return currentCableCost;

	}


	float getCableCost() {
		
		getStageCableCost(RedundancyCoeff);
		return currentCableCost.getNorm(CableNormDegree);

	}


	void init() {

		float totalDensity = 0;
		bestP.clear();
		topologyDensity.clear();
		const unsigned int NPastCabinets=getNPastCabinets();
		for (unsigned int ti = 0; ti < T.size(); ti++) {
			Topology& t = T[ti];
			assert(t.year < A.size());
			CabinetYear& c = A[t.year ];
			assert(
					 t.finalConnectivity.isSquare()
							&& c.Size == t.nativeConnectivity.getRows());
			const unsigned int Ntop = c.Size;
			t.dEnergy = t.dCost = FloatMat(Ntop, Ntop,
					(std::numeric_limits<float>::max)());


			//Define the density criteria for choosing a topology
			//Account for number of nodes still movable and the probability of the topology (that is it's more important to optimize most probable topologies)
			if(t.year<currentYear)
				topologyDensity.push_back(0);//avoid to change this topology
			else{
				totalDensity += (Ntop-NPastCabinets) * t.probability;
				topologyDensity.push_back(totalDensity);
			}
			bestP.push_back(t.permutation);
		}
		assert(totalDensity>0);
		
		//Compute initial cost
		currentCost = 0;
		//Energy cost
		if (EnergyCoeff > 0) {
			currentCost += EnergyCoeff*getEnergyCost();
		}


		//Compute cable cost only if necessary
		if (CostCoeff > 0) {
			currentCost += CostCoeff * getCableCost();
		}


		bestCost = currentCost;
		//std::cout << "initial cost is "<<currentCost<<std::endl;

		//Normalize density vector
		for (unsigned int ti = 0; ti < T.size(); ti++)
			topologyDensity[ti] /= totalDensity;
	}

	//input native ID 
	float computeDelta(unsigned int tIndex, unsigned int r, unsigned int s) {

		//std::cout<<" Switching "<<r <<" and "<<s <<" in topology "<<tIndex<<" from "<<currentCost<<std::endl;

		Topology& t = T[tIndex];
		assert(t.year < A.size());
		float dEnergy = 0, dCost=0;
		FloatMat dCostYear(getNYears());
		unsigned int fr=convertId(r,t.year);
		unsigned int fs=convertId(s,t.year);

		
		if (EnergyCoeff > 0) {
			Mat& p = t.permutation;
			Mat& b = t.finalConnectivity;
			assert(b.mirrorMode);
 			dEnergy = (_E(fr, fr) - _E(fs, fs))
					* (b(p(s), p(s)) - b(p(r), p(r)))
					+ (_E(fr, fs) - _E(fs, fr))
							* (b(p(s), p(r)) - b(p(r), p(s)));
			for (unsigned int k = 0; k < p.getRows(); k = k + 1)
				if (k != r && k != s && (b(p(k), p(s))!=0 ||  b(p(k), p(r))!=0)){
					unsigned int fk=convertId(k,t.year);
					dEnergy += (_E(fk, fr) - _E(fk, fs))
						* (b(p(k), p(s)) - b(p(k), p(r)));
					//Removed since b is mirrored
                    //		+ (_E(fr, fk) - _E(fs, fk))
					//				* (b(p(s), p(k)) - b(p(r), p(k)));
				}
			//assert(dEnergy>0);
			
		}

		if (CostCoeff > 0) {
			_computeCableCostDelta(dCostYear,false, tIndex,r,s);
			dCostYear+=currentCableCost;
			assert(dCostYear.getNorm(CableNormDegree)>=0);
			dCost=dCostYear.getNorm(CableNormDegree)-currentCableCost.getNorm(CableNormDegree);
		}
				

		float d = dEnergy * EnergyCoeff + dCost * CostCoeff;
		//std::cout<<" dEnergy:"<<dEnergy <<" dCost:"<<dCost<<" dTotal"<<d<<std::endl;
		assert(currentCost + d > 0);

		//store for future readings
		t.dCost(r, s) = dCost;
		t.dEnergy(r, s) = dEnergy;

		return d;
	}

	
	//input native ID 
	float readDelta(unsigned int tIndex, unsigned int r, unsigned int s) {
		return T[tIndex].dEnergy(r, s) * EnergyCoeff
				+ T[tIndex].dCost(r, s) * CostCoeff;
	}

	//Commit change in the solution, and save the new solution if it is better
	//Returns true if new solution is better
	//input native ID 
	bool commitSwap(unsigned int tIndex, unsigned int r, unsigned int s,
			bool remember = true) {

		//Update the partialCableCost matrix
		FloatMat dCost(getNYears());
		_computeCableCostDelta(dCost,true,tIndex,r,s);		

		Mat& P = T[tIndex].permutation;
		myswap<Val>(P(r), P(s));
		currentCost += readDelta(tIndex, r, s);
	

		//std::cout << "updated cost is "<<currentCost<<", full cost from scratch is  "<<getCableCost()<<" cable share is "<<CostCoeff*currentCableCost.getNorm(CableNormDegree) <<" and best total is "<< bestCost<< (remember?"REM":"NOT")<<std::endl;
		//assert( (currentCost+1) / ( CostCoeff * getCableCost())<1.1);
		//assert( (currentCost+1) / ( CostCoeff * getCableCost())>0.9);
		

		if (remember || currentCost < bestCost) {
			bestCost = currentCost;
			bestP[tIndex] = P;
			return true;
		};

		return false;
	}

	void setPermutations(const std::vector<Mat>& newPermut){
		for (unsigned int ti = 0; ti < T.size(); ti++){
			Topology& t = T[ti];
			t.permutation=newPermut[ti];
		}
	}

	float getCurrentCost() const {
		return currentCost;
	}

	float getBestCost() const {
		return bestCost;
	}

	const std::vector<Mat>& getBestSolution() const {
		return bestP;
	}

	//Get randomly a topology index from a number x in [0,1], following the topology density vector
	unsigned int getRandomTopologyIndex(float x) {
		assert(x >= 0 && x <= 1);
		unsigned int index = 0;
		while (index + 1 < topologyDensity.size() && x >= topologyDensity[index] )
			index++;
		return index;
	}

private:

	
	float bestCost, currentCost;
	
	FloatMat currentCableCost;
	
	//Contains the total cable cost for each cabinet pair, one matrix per year, with final Ids.
	std::vector<FloatMat> partialCableCost;
	
	std::vector<Mat> bestP;

	std::vector<float> topologyDensity;


	inline Position _patchPanel(Position firstPos, unsigned int year, Position secondPos)
	{
			bool horizontalMapping= (secondPos.x>secondPos.y);
		
			Position res=A[year].cornerPosition;

			if(horizontalMapping)
				res.y=firstPos.y;
			else					
				res.x=firstPos.x;
			return res;
	}

	//Input final ID
	inline void _C(FloatMat& costs, unsigned int i, unsigned int j) {
		
		
		unsigned int firstYear=_Y(_rackIndex(i));
		unsigned int secondYear=_Y(_rackIndex(j));

		if(secondYear<firstYear){
			myswap(j,i);
			myswap(secondYear,firstYear);
		}
		
		unsigned int first=i;
		Position firstPos=A.back().getRackPosition(first);

		unsigned int second=j;
		Position secondPos=A.back().getRackPosition(second);
	
		assert(firstYear!=Inf && secondYear!=Inf);
		
		for (unsigned int y = 0; y < getNYears(); y++)
				costs(y)=0;		

		if(UsePatchPanel)
		{

			if(firstYear==secondYear)
			{
				float rackDistance=(secondPos-firstPos).getNorm();
				costs(firstYear)= Parameters::cableCost((Parameters::cableLength(rackDistance)));					
			}else{
				costs(firstYear)=  Parameters::cableCost(  (_patchPanel(firstPos,firstYear,secondPos) - firstPos                                  ) .getNorm()   );					
				costs(secondYear)= Parameters::cableCost(  (secondPos                                 - _patchPanel(firstPos,firstYear,secondPos) ) .getNorm()   ) + Parameters::patchPanelCost(1);					
			}

			//Intermediate connections						
			for (unsigned int y = firstYear; y+1 < secondYear; y++)
				costs(y+1)= Parameters::cableCost(  (_patchPanel(firstPos,y+1,secondPos)     - _patchPanel(firstPos,y,secondPos) ) .getNorm()   ) + Parameters::patchPanelCost(1);					
		}else
		{
			//Direct set-up case
			float rackDistance=(secondPos-firstPos).getNorm();
			costs(firstYear)= Parameters::cableCost((Parameters::cableLength(rackDistance)));
		}
		
	}
	
	
	//Input final ID	
	inline float _E( unsigned int i, unsigned int j) {
		Position firstPos=A.back().getRackPosition(i);
		Position secondPos=A.back().getRackPosition(j);
		 		
		return Parameters::portPower((Parameters::cableLength((secondPos-firstPos).getNorm())));
		
	}
	
	//Input final ID	
	inline float _L( unsigned int i, unsigned int j) {
		Position firstPos=A.back().getRackPosition(i);
		Position secondPos=A.back().getRackPosition(j);
		 		
		//size in meters
		return (Parameters::cableLength((secondPos-firstPos).getNorm())/100);
		
	}
	
	
	//Input Final ID
	float _getPairCableCount(unsigned int i, unsigned int j, unsigned int perm_ti, unsigned int perm_i, unsigned int perm_j, unsigned int immediate, float redundancyCoeff)
	{
		//immediate=2 -> deterministic maximum amongst topologies
		//immediate=1 -> deterministic maximum amongst topologies. Groups may apply.
		//immediate=0 -> probabilistic cost

		
		//First compute the number of cables between these cabinets
		double ncables=0;
		unsigned int ni,nj, maxYear=std::max(_Y(_rackIndex(i)),_Y(_rackIndex(j)));
		for (unsigned int ti = 0; ti < T.size(); ti++) {
			Topology& t = T[ti];
			if (t.year < maxYear )
				continue;
			if(t.probability==0)
				continue;

			assert(t.year <= A.size());

			//Need the native values for addressing permutation
			if(ti == perm_ti){
				ni=convertId(perm_i,A.size()-1,t.year);
				nj=convertId(perm_j,A.size()-1,t.year);
				//std::cout<<"Year "<<t.year<<", "<<i<<"->"<<ni<<", "<<j<<"->"<<nj<<std::endl;
			}else{
				ni=convertId(i,A.size()-1,t.year);
				nj=convertId(j,A.size()-1,t.year);
				//std::cout<<"Year "<<t.year<<", "<<i<<"->"<<ni<<", "<<j<<"->"<<nj<<std::endl;
			}

			if(ni>=t.permutation.getRows()  ||   nj>=t.permutation.getRows() )
				continue;
			
			double tmp=t.finalConnectivity(
							t.permutation(ni),t.permutation(nj));

			if(tmp==0)
				continue;
		
			if (immediate > 0)
				ncables = mymax<Val>(ncables, tmp);
			else
				//Compute expectancy
				ncables += tmp * t.probability;
		}
	
		return std::ceil(ncables*(1.0+redundancyCoeff));
		
	}
	
		
	//Input Final ID
	void _getPairCableCost(FloatMat& maxi, unsigned int i, unsigned int j, unsigned int perm_ti, unsigned int perm_i, unsigned int perm_j, unsigned int immediate, unsigned int redundancyCoeff)
	{
		//First compute the number of cables between these cabinets
		double ncables=_getPairCableCount(i,j,perm_ti,perm_i, perm_j, immediate, redundancyCoeff);
		
		//Compute the unitary cost of cables for each year
		_C(maxi,i, j);
		//Consider the cost of redundant cables									
		maxi=maxi*ncables;
		
		if(ncables>0){
			//std::cout<<"Cable:"<<i<<"<->"<<j<<"gives ncable:"<<ncables<<" cost:";maxi.toVectorStream(std::cout);
		}		
						
	}
	
	//Input Native Id
	void _computeCableCostDelta(FloatMat& delta, bool save, unsigned int ti,unsigned int r, unsigned int s) {
				
		unsigned int NMax = getN();
		unsigned int NYears=getNYears();
		
		FloatMat maxiR(NYears), maxiS(NYears);
		unsigned int fr=convertId(r,T[ti].year);
		unsigned int fs=convertId(s,T[ti].year);


		//i,j represent the position of a couple of cabinets
		unsigned int immediateR= ( _Y(_rackIndex(fr)) <= currentYear) ? 1 : 0;
		unsigned int immediateS= ( _Y(_rackIndex(fs)) <= currentYear) ? 1 : 0;
		
		
		//std::cout<<" Switching (Cable) "<<r <<" and "<<s <<" in topology "<<ti<<" from ";currentCableCost.toVectorStream(std::cout);

		_getPairCableCost(maxiR,fr,fs,ti,fs,fr,immediateR+immediateS,RedundancyCoeff);
		for (unsigned int j = 0; j < NYears; j++){
			delta(j) = (maxiR(j)-partialCableCost[j](fr,fs));
			if(save)
				partialCableCost[j](fr,fs)=maxiR(j);		
		}
		
				
		unsigned int immediateI;
		for (unsigned int i = 0; i < NMax; i++) {
			if(i==fs || i==fr)
				continue;

			immediateI = ( _Y(_rackIndex(i)) <= currentYear) ? 1 : 0;

			_getPairCableCost(maxiR,i,fr,ti,i,fs,immediateI+immediateR,RedundancyCoeff);
			_getPairCableCost(maxiS,i,fs,ti,i,fr,immediateI+immediateS,RedundancyCoeff);

			for (unsigned int j = 0; j < NYears; j++)
			{
				delta(j) += (maxiR(j)+maxiS(j)) -(partialCableCost[j](i,fr) + partialCableCost[j](i,fs));
				if(save)
				{
					partialCableCost[j](i,fr)=maxiR(j);
					partialCableCost[j](i,fs)=maxiS(j);					
				}
			}
		}
		
		//std::cout<<" By delta ";delta.toVectorStream(std::cout);

		if(save)
			currentCableCost+=delta;
		
		
	}
	
	
	struct HistoValue{
		float distance;
		float ncables;
		float nbackups;	
	};
	
	
	FloatMat _getHistogram( unsigned int year){

		year=mymin<unsigned int>(year,getNYears()-1);
		
		std::vector<HistoValue> values;		

		std::vector<HistoValue>::iterator it;		

		unsigned int NYears=getNYears();
		FloatMat maxi(NYears),maxi0(NYears);
		
		
		unsigned int NMax = A[NYears-1].Size;
		
		//i,j represent the final ID of a couple of cabinets
		unsigned int immediate;
		for (unsigned int i = 0; i < NMax; i++) {
			for (unsigned int j = i ; j < NMax; j++) {
				Position posi=_rackIndex(i);
				Position posj=_rackIndex(j);
				immediate = (_Y(posi) <= year ) ? 1 : 0;
				immediate += (_Y(posj) <= year ) ? 1 : 0;

				_getPairCableCost(maxi,i,j, T.size(),0,0,immediate,RedundancyCoeff);
				float ncables=maxi.getNorm(1);

				_getPairCableCost(maxi0,i,j, T.size(),0,0,immediate,0);
				float nbackups=ncables-maxi0.getNorm(1);


				if(ncables==0)
					continue;

				float distance = (posi-posj).getNorm();

				//Now, insert the pair (distance,ncables) to the values vector.
				for(it=values.begin();it!=values.end();it++){

					if(it->distance==distance)
					{
						//Found exact match
						it->ncables+=ncables;		
						it->nbackups+=nbackups;		
						break;
					}					

					if(it->distance<distance)
					{
						//Overpassed the the insert place
						HistoValue h;
						h.distance=distance;
						h.ncables=ncables;
						h.nbackups=nbackups;
						values.insert(it,h);
						it=values.begin();
						break;
					}
				}
				
				if(it==values.end())
				{
						HistoValue h;
						h.distance=distance;
						h.ncables=ncables;
						h.nbackups=nbackups;
					values.push_back(h);					
				}
			}
		}
		
		//Compute total
		HistoValue total;total.distance=total.ncables=total.nbackups=0;
		for(it=values.begin();it!=values.end();it++){
			total.distance+=it->distance*it->ncables;
			total.ncables+=it->ncables;
			total.nbackups+=it->nbackups;
		}
		total.distance/=total.ncables;
		values.push_back(total);		

		FloatMat result(values.size(),3);
		unsigned int index=0;
		for(it=values.begin();it!=values.end();it++){
			result(index,0)=it->distance;
			result(index,1)=it->ncables;
			result(index++,2)=it->nbackups;
		}
		
		return result;		
		
		}


};

#endif /* COSTMANAGER_H_ */
