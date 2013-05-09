//
//  solver.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__solver__
#define __ucarpp__solver__

#ifndef DEBUG
#define DEBUG
#endif

//#include <unordered_set>
#include <list>
#include <sstream>

#include "headings.h"
#include "edge.h"
#include "graph.h"
#include "meta.h"

namespace solver
{
	class Vehicle
	{
	private:
		//std::unordered_set<MetaEdge*> path;
		std::list<MetaEdge*> path;

	public:
		Vehicle();
		
		void addEdge( MetaEdge*, long = -1 );
		void removeEdge( long = -1 );

		unsigned long size();
		
		uint getCost();
		uint getDemand();
		uint getProfit();

		std::string toString();
	};

	class Solution
	{
	private:
		int M;
		Vehicle** vehicles;
		
	public:
		Solution( int );

		void addEdge( MetaEdge*, int, int = -1 );
		void removeEdge( int, int = -1 );

		unsigned long size();
		
		uint getCost();
		uint getCost( int );
		uint getDemand();
		uint getDemand( int );

		std::string toString();
		std::string toString( int );
	};

	class Solver
	{
	private:
		MetaGraph graph;
		uint depot,
		M,
		Q,
		tMax;
		Solution currentSolution;
		
		Solution createBaseSolution();
		
		struct compareRatioDescending
		{
			bool operator()( const MetaEdge* lhs, const MetaEdge* rhs ) const
			{
				// Ratio se lato non preso, -1 altrimenti
				float lhsRatio = ( lhs->getTaken() == 0 ? lhs->getProfitDemandRatio() : -1 ),
				rhsRatio = ( rhs->getTaken() == 0 ? rhs->getProfitDemandRatio() : -1 );
				
				if ( lhsRatio == rhsRatio )
					return lhs->getCost() > rhs->getCost();
					
					return lhsRatio > rhsRatio;
			}
		};
		
		compareRatioDescending greedyCompare;
	public:
		Solver( MetaGraph, uint, uint, uint, uint );
		
		Solution solve();
	};
}

#endif /* defined(__ucarpp__solver__) */
