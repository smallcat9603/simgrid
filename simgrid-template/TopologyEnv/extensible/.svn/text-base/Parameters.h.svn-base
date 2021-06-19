/*
 * Costs.h
 *
 *  Created on: Mar 5, 2014
 *      Author: chaix
 *
 *  Based on Ikki's scripts lay.rb and allocXX.rb
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_


/*
	References for the parameters cost

 	 [Mudigonda "Taming the flying cable monster" 2011]
	 スイッチ
	 	$500/port
	 カスタムケーブル
	 	Single-channel (server <-> switch)
			copper $6/m + $40  (<=5m)
	 	Quad-channel (switch <-> switch)
	 		copper $16/m + $40 (<=5m)
	 		fiber  $5/m + $376 (>5m)
	 	これに製作・運搬費用として25%上乗せ
	 ストックケーブル
	 	SFP copper (server <-> switch)
	 		1m  $45
	 		2m  $52
	 		3m  $66
	 		5m  $74
	 		10m $101
	 		12m $117
	 	QSFP copper (switch <-> switch, <=5m)
	 		1m  $55
	 		2m  $74
	 		3m  $87
	 		5m  $116
	 	QSFP+ fiber (switch <-> switch, >5m)
	 		10m  $418
	 		15m  $448
	 		20m  $465
	 		30m  $508
	 		50m  $618
	 		100m $883
	 ケーブル設置費用
	 	intra-rack $2.50
	 	inter-rack $6.25

	[Curtis "REWIRE" 2012]
	 ケーブル
	 	short (10G) $60   :lower  25%
	 	mid   (10G) $120  :middle 50%
	 	long  (10G) $250  :higher 25%
	 スイッチ
	 	4p (10G) + 48p (1G) $5000
	 	24p (10G)  $6000
	 	48p (10G)  $10000
	*/


struct Position{
	 int x;
	 int y;	
	 
	 Position operator-(const Position& b)
	 {
	 	Position tmp;
	 	tmp.x=x-b.x;
	 	tmp.y=y-b.y;
	 	return tmp;
	 }
	 
	 int getNorm() const
	 {
		return std::abs(x)+std::abs(y);	 
	 }
};


//Used for POST computation
typedef std::pair<unsigned int,float> VertexStepCost;

struct Parameters{

	static const float OPTICAL_CABLE_START_LENGTH = 300; //Distance beyound which optical cables are used [cm]

	static const float OVERHEAD_OUTER = 400; // Inter-rack cabling overhead [cm]
	static const float OVERHEAD_INNER = 200; // Intra-rack cabling overhead [cm]
	static const float RACK_WIDTH     =  60; // Rack width [cm]
	static const float RACK_DEPTH     = 210; // Rack depth including aisle [cm]


	static Position RackPositionToIndex(Position position)
	{
		Position res; res.x=position.x/RACK_WIDTH; res.y=position.y/RACK_DEPTH;
		return res;
	}	

	static Position RackIndexToPosition(Position position)
	{
		Position res; res.x=position.x*RACK_WIDTH; res.y=position.y*RACK_DEPTH;
		return res;
	}	

	static float cableLength(float rackDistance){
		if(rackDistance == 0)
			return OVERHEAD_INNER;
		return OVERHEAD_OUTER + rackDistance;
	}


	// Energy consumption per packet [pJ]
	static float cableEnergy(float cableLength){
		if(cableLength <= OPTICAL_CABLE_START_LENGTH)
			return 2;
	    return 60;
	}


	// Power consumption per switch port [mW]
	static float portPower(float cableLength){
	if(cableLength <= OPTICAL_CABLE_START_LENGTH)
		return 6600;
    return 12400;
	}

	static float cableCost(float cableLength){
		float dollar;
		if (cableLength <= OVERHEAD_INNER)
			dollar = (cableLength / 100 * 16 + 40) * 1.25 + 2.50;
			else if(cableLength <= OPTICAL_CABLE_START_LENGTH )
			dollar = (cableLength / 100 * 16 + 40) * 1.25 + 6.25;
		else
			dollar = (cableLength / 100 * 5 + 376) * 1.25 + 6.25;
		return std::floor( (float)(dollar * 100.0+.5) )/ 100 ;
	}

	static float switchCost(unsigned int nports){
		return nports * 500;
	}

	//Linearized patch-panel cost
	static float patchPanelCost(unsigned int nports){
		return nports * 10;
	}

	static std::vector<VertexStepCost> getSwitchSteps(){
		//So far, switches are not capable to forward cables, so empty step vector is OK
		return std::vector<VertexStepCost>();
	}


	static std::vector<VertexStepCost> getPatchPanelSteps(){
		std::vector<VertexStepCost> res;
		res.push_back(VertexStepCost(25,250.0));
		res.push_back(VertexStepCost(50,500.0));
		res.push_back(VertexStepCost(100,800.0));
		return res;
	}



};


#endif /* PARAMETERS_H_ */
