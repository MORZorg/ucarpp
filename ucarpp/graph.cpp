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
	
	this->costs = (uint**)calloc( V, sizeof( uint* ) );
	for ( int i = 0; i < V; i++ )
		this->costs[ i ] = (uint*)calloc( V, sizeof( uint ) );
	
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
	ProfitEdge newEdge( src, dst, demand, profit );
	this->edges[ src ].push_back( newEdge );
	this->edges[ dst ].push_back( newEdge );
	
	// Aggiungo il costo del lato alla matrice dei costi.
	costs[ src ][ dst ] = costs[ dst ][ src ] = cost;
}

// Funzione per il confronto tra nodi in base al loro costo.
class dijkyNodeComparison
{
	uint* d;
public:
	dijkyNodeComparison( uint* distances ): d( distances ) {}
	bool operator() ( const uint& lhs, const uint& rhs ) const
	{
		return ( d[ lhs ] > d[ rhs ] );
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
	uint* d;
	vector<int> Q;
	for ( uint source = 0; source < V; source++ )
	{
		// Lavoro direttamente sulla matrice dei costi.
		dijkyNodeComparison comp( d = costs[ source ] );
		for ( uint u = 0; u < V; u++ )
			if ( d[ u ] == 0 )
				d[ u ] = INT_MAX;
		d[ source ] = 0;
		
		// Costruisco un min-heap dei nodi.
		for ( uint u = 0; u < V; u++ )
			Q.push_back( u );
		make_heap( Q.begin(), Q.end(), comp );
		
		while ( Q.size() > 0 )
		{
			// Prendo il primo elemento e lo rimuovo dallo heap.
			uint u = Q.front();
			pop_heap( Q.begin(), Q.end(), comp );
			Q.pop_back();
			
			// Se l'elemento migliore ha distanza infinita, non c'è più niente da fare...
			if ( d[ u ] == INT_MAX )
				break;
			
			bool found = false;
			uint v, alt;
			for ( vector<Edge>::iterator it = edges[ u ].begin(); it != edges[ u ].end(); it++ )
			{
				v = it->getDst( u );
				
				// Uso i getter per essere sicuro che il costo sia già definito ( src < dst )
				alt = d[ u ] + this->costs[ it->getSrc() ][ it->getDst() ];
				if ( alt < d[ v ] )
				{
					d[ v ] = alt;
					
					// Aggiorno la posizione di v.
					// TODO: Teoricamente da fare in modo più efficiente.
					make_heap( Q.begin(), Q.end(), comp );
				}
				
				// Flag per indicare se esiste già un lato tra source ed u.
				if ( v == source )
					found = true;
			}
			
			// Se necessario, creo il lato associato a ( source, u ) e lo aggiungo alla lista
			if ( !found )
			{
				Edge newEdge( source, u );
				this->edges[ source ].push_back( newEdge );
				this->edges[ u ].push_back( newEdge );
			}
		}
	}
}

// Getter della matrice dei costi
uint Graph::getCost( uint src, uint dst )
{
	return this->costs[ src ][ dst ];
}
