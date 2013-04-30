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

namespace solver
{
	class Vehicle
	{
	private:
		//std::unordered_set<MetaEdge*> path;
		std::list<model::MetaEdge*> path;

	public:
		Vehicle();
		
		void addEdge( model::MetaEdge*, int = -1 );
		void removeEdge( int = -1 );

		unsigned long size();
		
		uint getCost();
		uint getDemand();

		std::string toString();
	};

	class Solution
	{
	private:
		int M;
		Vehicle* vehicles;
		
	public:
		Solution( int );

		void addEdge( model::MetaEdge*, int, int = -1 );
		void removeEdge( int, int = -1 );

		unsigned long size();
		
		uint getCost( model::Graph );
		uint getDemand();

		std::string toString();
	};

	class Solver
	{
	private:
		model::Graph graph;
		uint depot,
		M,
		Q,
		tMax;
		Solution* currentSolution;
		
		Solution createBaseSolution();
		
		struct compareRatioDescending
		{
			const model::Graph* graph;
			
			compareRatioDescending( model::Graph* g ): graph( g ) {};
			
			bool operator()( const model::MetaEdge* lhs, const model::MetaEdge* rhs ) const
			{
				// Ratio se lato non preso, -1 altrimenti
				float lhsRatio = ( lhs->getTaken() == 0 ? lhs->getProfitDemandRatio() : -1 ),
				rhsRatio = ( rhs->getTaken() == 0 ? rhs->getProfitDemandRatio() : -1 );
				
				if ( lhsRatio == rhsRatio )
					return graph->getCost( lhs ) > graph->getCost( rhs );
					
					return lhsRatio > rhsRatio;
			}
		};
		
		compareRatioDescending greedyCompare;
	public:
		Solver( model::Graph, uint, uint, uint, uint );
		
		Solution* solve();
		
		bool isFeasible( Solution );
	};
}

#endif /* defined(__ucarpp__solver__) */
