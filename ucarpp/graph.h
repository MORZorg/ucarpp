//
//  graph.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 03/25/13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__graph__
#define __ucarpp__graph__

#include <iostream>

class graph
{
	int V;
	int[][] costs;
	
public:
	graph( int );
	void addEdge( int, int, int, int, float );
};

#endif /* defined(__ucarpp__graph__) */
