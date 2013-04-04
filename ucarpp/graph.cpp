//
//  graph.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 03/25/13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "graph.h"

// Costruttore
graph::graph( int V )
{
	this->V = V;
	this->costs = new int[ V ][ V ];
}

void graph::addEdge( int src, int dst, int cost, int demand, float profit )
{
	return;
}