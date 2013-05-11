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
		MetaGraph graph;
		Vehicle** vehicles;
		
	public:
		Solution( int, model::Graph );

		void addEdge( model::Edge*, int, int = -1 );
		void removeEdge( int, int = -1 );

		unsigned long size();
		
		uint getCost();
		uint getCost( int );
		uint getDemand();
		uint getDemand( int );

		std::string toString();
		std::string toString( int );
		
		struct compareRatioGreedy
		{
			MetaGraph* graph;
			
			compareRatioGreedy( MetaGraph* graph ): graph( graph ) {}
			
			bool operator() ( const model::Edge* lhs, const model::Edge* rhs ) const
			{
				MetaEdge* metaLhs = graph->getEdge( lhs ),
						* metaRhs = graph->getEdge( rhs );
				
				// Ratio se lato non preso, -1 altrimenti
				float lhsRatio = ( metaLhs->getTaken() == 0 ? metaLhs->getProfitDemandRatio() : -1 ),
				rhsRatio = ( metaRhs->getTaken() == 0 ? metaRhs->getProfitDemandRatio() : -1 );
				
				if ( lhsRatio == rhsRatio )
					return metaLhs->getCost() > metaRhs->getCost();
					
					return lhsRatio > rhsRatio;
			}
		} compareRatioGreedy;
	};

	class Solver
	{
	private:
		model::Graph graph;
		uint depot,
		M,
		Q,
		tMax;
		Solution currentSolution;
		
		Solution createBaseSolution();
		
//		struct compareRatioDescending
//		{
//			bool operator()( const MetaEdge* lhs, const MetaEdge* rhs ) const
//			{
//				// Ratio se lato non preso, -1 altrimenti
//				float lhsRatio = ( lhs->getTaken() == 0 ? lhs->getProfitDemandRatio() : -1 ),
//				rhsRatio = ( rhs->getTaken() == 0 ? rhs->getProfitDemandRatio() : -1 );
//				
//				if ( lhsRatio == rhsRatio )
//					return lhs->getCost() > rhs->getCost();
//					
//					return lhsRatio > rhsRatio;
//			}
//		};
//		
//		compareRatioDescending greedyCompare;
	public:
		Solver( model::Graph, uint, uint, uint, uint );
		
		Solution solve();
	};
}

#endif /* defined(__ucarpp__solver__) */
