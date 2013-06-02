//
//  vehicle.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__vehicle__
#define __ucarpp__vehicle__

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
			//Vehicle( const Vehicle& );
			//~Vehicle();
			
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
}

#endif /* defined(__ucarpp__vehicle__) */
