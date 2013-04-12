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
	// Numero di Vertici
	uint V;
	// Matrice dei Costi
	uint** costs;
	// Lista di Adiacenza
	vector<Edge>* edges;
	
public:
	Graph( int );
	
	void addEdge( uint, uint, uint, uint, float );
	void completeCosts();
	
	uint getCost( uint, uint );
};

#endif /* defined(__ucarpp__graph__) */
