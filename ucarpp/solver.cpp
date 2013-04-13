//
//  solver.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "solver.h"

/*** Solver ***/

Solver::Solver( Graph graph, int M ):
	graph( graph ), M( M )
{
	currentSolution = (Solution*)calloc( M, sizeof( Solution ) );
	for ( int i = 0; i < M; i++ )
		currentSolution[ i ] = *new Solution();
}

void Solver::createBaseSolution()
{
	// Ordino i lati
	vector<Edge> edges = graph.getEdges();
	sort( edges.begin(), edges.end(), compareEdges );
	
	// 
}


/*** Solution ***/
Solution::Solution()
{
	path = *new vector<Edge>();
}