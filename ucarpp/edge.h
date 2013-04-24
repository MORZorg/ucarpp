//
//  node.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 04/04/13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__edge__
#define __ucarpp__edge__

#include "headings.h"

namespace model
{
	class Edge
	{
	private:
		uint src,
			 dst;
		uint taken;
		
	public:
		Edge( uint, uint );
		
		uint setTaken();
		uint unsetTaken();
		uint getTaken() const;
		
		uint getSrc() const;
		uint getDst() const;
		uint getDst( uint ) const;
		
		virtual uint getDemand() const;
		virtual float getProfit() const;
		
		float getProfitDemandRatio() const;
	};

	class ProfitableEdge: public Edge
	{
	private:
		uint demand;
		float profit;
		
	public:
		ProfitableEdge( uint, uint, uint, float );
		
		uint getDemand() const;
		float getProfit() const;
	};
}

#endif /* defined(__ucarpp__node__) */
