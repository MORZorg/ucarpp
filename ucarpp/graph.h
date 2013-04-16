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

#include <boost/heap/fibonacci_heap.hpp>

#ifdef DEBUG
#include <iostream>
#endif

#include "edge.h"

using namespace std;
using namespace boost;

typedef unsigned int uint;

class Graph
{
private:
	// Numero di Vertici
	uint V;
	// Matrice dei Costi
	uint** costs;
	// Vettore dei Lati
	vector<Edge*> edges;
	// Lista di Adiacenza
	vector<Edge*>* adjList;
	
public:
	Graph( int );
	
	void addEdge( uint, uint, uint, uint, float );
	void completeCosts();
	
	uint getCost( uint, uint ) const;
	uint getCost( const Edge* ) const;
	
	Edge* getEdge( uint, uint ) const;
	vector<Edge*> getEdges() const;
	vector<Edge*> getAdjList( uint ) const;
};

#endif /* defined(__ucarpp__graph__) */
