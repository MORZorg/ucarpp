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
#include "solver.h"

namespace model
{
	class Edge
	{
	private:
		uint src,
			 dst;
	
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

namespace solver
{
	
	class MetaEdge: public model::Edge
	{
	private:
		Edge* actualEdge;
		std::vector<const Vehicle*> takers;
		
	public:
		MetaEdge( model::Edge* );
		
		uint getSrc() const;
		uint getDst() const;
		uint getDst( uint ) const;
		
		uint getCost() const;
		uint getDemand() const;
		float getProfit() const;
		//float getProfitDemandRatio() const;
		
		uint setTaken( const Vehicle* );
		uint unsetTaken( const Vehicle*, int );
		uint getTaken() const;
		bool isServer( const Vehicle* ) const;
		const Vehicle* getServer() const;
	};
}

#endif /* defined(__ucarpp__node__) */
