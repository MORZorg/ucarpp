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
#include <vector>

namespace model
{
	class Edge
	{
	private:
		uint src,
			 dst;
		
		bool equals( const Edge& ) const;
		
	protected:
		uint cost;
		
	public:
		Edge( uint, uint );
		
		uint getSrc() const;
		uint getDst() const;
		uint getDst( uint ) const;
		
		uint getCost() const;
		virtual uint getDemand() const = 0;
		virtual float getProfit() const = 0;
		
		float getProfitDemandRatio() const;
		
		bool operator ==( const Edge& ) const;
	};
	
	class DijkyEdge: public Edge
	{
	public:
		DijkyEdge( uint, uint );

		void setCost( uint );

		uint getDemand() const;
		float getProfit() const;
	};

	class ProfitableEdge: public Edge
	{
	private:
		uint demand;
		float profit;
		
	public:
		ProfitableEdge( uint, uint, uint, uint, float );
		
		uint getDemand() const;
		float getProfit() const;
	};
}

#endif /* defined(__ucarpp__node__) */
