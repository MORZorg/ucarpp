//
//  solver.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__solver__
#define __ucarpp__solver__

//#include <unordered_set>
#include <list>
#include <vector>
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
		int id;
		std::list<MetaEdge*> path;

		int getId() const;
		bool equals( const Vehicle& ) const;

	public:
		Vehicle( int );
		Vehicle( const Vehicle& );
		
		MetaEdge* getEdge( int ) const;
		void addEdge( MetaEdge*, long = -1 );
		void removeEdge( long = -1 );

		unsigned long size() const;
		
		uint getCost() const;
		uint getDemand() const;
		uint getProfit() const;

		bool getDirection( int ) const;

		std::string toString() const;
		
		bool operator ==( const Vehicle & ) const;

	};

	class Solution
	{
	private:
		int M;
		MetaGraph graph;
		std::vector<Vehicle> vehicles;
		
	public:
		Solution( int, model::Graph );
		Solution( const Solution& );

		MetaEdge* getEdge( int, int ) const;
		void addEdge( model::Edge*, int, int = -1 );
		void removeEdge( int, int = -1 );

		unsigned long size() const;
		unsigned long size( int ) const;
		
		uint getProfit() const;
		uint getProfit( int ) const;
		uint getCost() const;
		uint getCost( int ) const;
		uint getDemand() const;
		uint getDemand( int ) const;

		bool getDirection( int, int ) const;

		std::string toString() const;
		std::string toString( int ) const;
		
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
		const int	K_MAX		= 10;
		const float	XI			= 2;
		const float	P_CLOSE		= .10;
		const float	P_ACCEPT	= .90;

		model::Graph graph;
		uint depot,
		M,
		Q,
		tMax;
		Solution* currentSolution;
		
		Solution createBaseSolution();
		void createBaseSolution( Solution*, int );
		Solution vns( int, Solution );
		bool closeSolutionRandom( Solution*, int, uint, uint, int, int );

		bool isFeasible( Solution, int ) const;
	public:
		Solver( model::Graph, uint, uint, uint, uint );
		
		Solution* solve();
	};
}

#endif /* defined(__ucarpp__solver__) */
