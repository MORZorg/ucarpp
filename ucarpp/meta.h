//
//  meta.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-05-09.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__meta__
#define __ucarpp__meta__

#include "headings.h"
#include "edge.h"
#include "graph.h"
#include <unordered_map>	// If not working: Boost
//#include "solver.h"

namespace solver
{
	// Forward declaration to avoid cyclic includes
	class Vehicle;
	
	class MetaEdge
	{
		private:
			model::Edge* actualEdge;
			std::vector<const Vehicle*> takers;
		
			bool equals( const MetaEdge& ) const;
		
		public:
			MetaEdge( model::Edge* );
			MetaEdge( const MetaEdge& );
			
			uint getSrc() const;
			uint getDst() const;
			uint getDst( uint ) const;
			
			uint getCost() const;
			uint getDemand() const;
			float getProfit() const;
			float getProfitDemandRatio() const;
			
			unsigned long setTaken( const Vehicle*, int );
			unsigned long unsetTaken( const Vehicle*, int );
			unsigned long getTaken() const;
			bool isServer( const Vehicle* ) const;
			const Vehicle* getServer() const;
		
			bool operator ==( MetaEdge& ) const;
			bool operator !=( MetaEdge& ) const;
	};
	
	class MetaGraph
	{
		private:
			// Vettore dei Lati
			//std::vector<MetaEdge*> edges;
			std::unordered_map<model::Edge*, MetaEdge*> edges;
			// Lista di Adiacenza
			//std::unordered_map<uint, MetaEdge*>* adjList;
			
		public:
			MetaGraph( model::Graph );
			MetaGraph( const MetaGraph& );
			
//			MetaEdge* getEdge( uint, uint ) const throw( int );
			MetaEdge* getEdge( const model::Edge* ) const;
//			std::unordered_map<model::Edge*, MetaEdge*> getEdges() const;
//			std::unordered_map<uint, MetaEdge*> getAdjList( uint ) const;
	};
}

#include "solver.h"

#endif /* defined(__ucarpp__meta__) */
