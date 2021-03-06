//
//  solution.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__solution__
#define __ucarpp__solution__

#include <list>
#include <vector>
#include <set>
#include <sstream>

#include "headings.h"
#include "edge.h"
#include "graph.h"
#include "meta.h"
#include "vehicle.h"

namespace solver
{
	class Solution
	{
		private:
			int M;
			MetaGraph graph;
			std::vector<Vehicle*> vehicles;
			
		public:
			Solution( int, model::Graph );
			Solution( const Solution& );
			~Solution();

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

			Vehicle* getVehicle( uint ) const;
			uint getVehicleIndex( const Vehicle* ) const;

			bool getDirection( int, int ) const;

			std::string toString() const;
			std::string toString( int ) const;

			std::string toServicesSequence( int ) const;
			std::string toVertexSequence( int ) const;
			
			bool operator> ( const Solution& ) const;
			
			struct compareGreedy
			{
				MetaGraph* graph;
				
				compareGreedy( MetaGraph* graph ): graph( graph ) {}
				
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
			} compareGreedy;
			
			struct compareStingy
			{
				bool operator() ( const model::Edge* lhs, const model::Edge* rhs ) const
				{
					return lhs->getCost() < rhs->getCost();
				}
			} compareStingy;
			
			struct comparePacking
			{
				bool operator() ( const MetaEdge* lhs, const MetaEdge* rhs ) const
				{
					return lhs->getDemand() > rhs->getDemand();
				}
			} comparePacking;
	};
}

#endif /* defined(__ucarpp__solution__) */
