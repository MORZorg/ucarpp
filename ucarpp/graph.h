//
//  graph.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 03/25/13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__graph__
#define __ucarpp__graph__

#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <climits>

#include "headings.h"
#include "edge.h"

namespace model
{
	class MetaGraph
	{
	private:
		// Numero di Vertici
		uint V;
		// Vettore dei Lati
		std::vector<Edge*> edges;
		// Lista di Adiacenza
		std::vector<Edge*>* adjList;
		
	public:
		MetaGraph( Graph );
		
		void addEdge( uint, uint, uint, uint, float );
		void completeCosts();
		
		Edge* getEdge( uint, uint ) const;
		std::vector<Edge*> getEdges() const;
		std::vector<Edge*> getAdjList( uint ) const;
	};

	class Graph
	{
		private:
			// Numero di Vertici
			uint V;
			// Vettore dei Lati
			std::vector<Edge*> edges;
			// Lista di Adiacenza
			std::vector<Edge*> adjList;

		protected:
			std::vector<Edge*> getAdjList( uint ) const;

		

	};
}

#endif /* defined(__ucarpp__graph__) */
