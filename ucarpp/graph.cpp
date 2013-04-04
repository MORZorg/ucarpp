//
//  graph.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 03/25/13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "graph.h"

/**
 * Costruttore.
 * 
 * @param V	numero di vertici
 *
 */
Graph::Graph( int V )
{
	this->V = V;
	
	this->costs = (int**)calloc( V, sizeof( int* ) );
	for ( int i = 0; i < V; i++ )
		this->costs[ i ] = (int*)calloc( V, sizeof( int ) );
	
	this->edges = (vector<Edge>*)calloc( V, sizeof( vector<Edge> ) );
	for ( int i = 0; i< V; i++ )
		this->edges[ i ] = *new vector<Edge>();
}

/**
 * Aggiunge un lato al grafo.
 *
 * @param src		nodo sorgente
 * @param dst		nodo destinazione
 * @param cost		costo del lato
 * @param demand	domanda del lato
 * @param profit	profitto del lato
 */
void Graph::addEdge( uint src, uint dst, uint cost, uint demand, float profit )
{
	// Creo un nuovo lato e lo aggiungo alla lista in entrambe le direzioni.
	Edge newEdge( src, dst, demand, profit );
	this->edges[ src ].push_back( newEdge );
	this->edges[ dst ].push_back( newEdge );
	
	// Aggiungo il costo del lato alla matrice dei costi.
	costs[ src ][ dst ] = costs[ dst ][ src ] = cost;
}

class nodeComparison
{
	uint* d;
public:
	nodeComparison( uint* distances )
    { d = distances; }
	bool operator() ( const uint& lhs, const uint& rhs ) const
	{
		return ( d[ lhs ] < d[ rhs ] );
	}
};

/**
 * Completa la magliatura del grafo aggiungendo lati con costo minimo e
 * domanda e profitto nulli.
 */
void Graph::completeCosts()
{
	// Uso Dijkstra applicato ad ogni nodo del grafo.
	// Essendo il nostro grafo sparso, non esiste algoritmo migliore.
	uint d[ V ], p[ V ];
	priority_queue<uint, vector<uint>, nodeComparison> Q;
	for ( uint source = 0; source < V; source++ )
	{
		for ( uint u = 0; u < V; u++ )
		{
			d[ u ] = INT_MAX;
			p[ u ] = u;
		}
		
		d[ source ] = 0;
		for ( uint u = 0; u < V; u++ )
			Q.push( u );
		
		while ( Q.size() > 0 )
		{
			uint u = Q.top();
			Q.pop();
			if ( d[ u ] == INT_MAX )
				break;
			
			uint v, alt;
			for ( vector<Edge>::iterator it = edges[ u ].begin(); it != edges[ u ].end(); it++ )
			{
				v = it->getDst( u );
				
				alt = d[ u ] + this->costs[ u ][ v ];
				if ( alt < d[ v ] )
				{
					d[ v ] = alt;
					p[ v ] = u;
					//Q.decrease-key( v );
				}
			}
		}
	}
}