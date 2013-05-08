	//
	//  graph.cpp
	//  ucarpp
	//
	//  Created by Maurizio Zucchelli on 03/25/13.
	//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
	//

#include "graph.h"

using namespace std;
using namespace model;
using namespace solver;

/*** Graph ***/

/**
 * Costruttore.
 * 
 * @param V	numero di vertici
 *
 */
Graph::Graph( int V ):
	V( V )
{
	//this->costs = (uint**)calloc( V, sizeof( uint* ) );
	//for ( int i = 0; i < V; i++ )
	//	this->costs[ i ] = (uint*)calloc( V, sizeof( uint ) );
	
	this->edges = *new vector<Edge*>();
	
	this->adjList = (vector<Edge*>*)calloc( V, sizeof( vector<Edge*> ) );
	for ( int i = 0; i < V; i++ )
		this->adjList[ i ] = *new vector<Edge*>();
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
	edges.push_back( new ProfitableEdge( src, dst, cost, demand, profit ) );
	adjList[ src ].push_back( edges.back() );
	adjList[ dst ].push_back( edges.back() );
}

// Funzione per il confronto tra nodi in base al loro costo.
namespace model
{
	class dijkyNodeComparison
	{
	private:
		Graph* g;
		uint src;
	public:
		dijkyNodeComparison( Graph* graph, uint source ):
			g( graph ), src( source ) {}
		bool operator() ( const uint& lhs, const uint& rhs ) const
		{
			return ( g->getEdge( src, lhs )->getCost() >
					 g->getEdge( src, rhs )->getCost() );
		}
	};
}

/**
 * Completa la magliatura del grafo aggiungendo lati con costo minimo e
 * domanda e profitto nulli.
 */
void Graph::completeCosts()
{
	// Uso Dijkstra applicato ad ogni nodo del grafo.
	// Essendo il nostro grafo sparso, non esiste algoritmo migliore.
	vector<int> Q;
	for ( uint source = 0; source < V; source++ )
	{
		// Da cambiare: se non esiste, crea con costo max
		dijkyNodeComparison comp( this, source );
		for ( uint u = 0; u < V; u++ )
			if ( u != source )
				try
				{
					getEdge( source, u );
				}
				catch ( exception& e )
				{
					edges.push_back( new DijkyEdge( source, u ) );
					adjList[ source ].push_back( edges.back() );
					adjList[ u ].push_back( edges.back() );
					( (DijkyEdge*)edges.back() )->setCost( INT_MAX );
				}
		//d[ source ] = 0;
		
		// Costruisco un min-heap dei nodi.
		for ( uint u = 0; u < V; u++ )
			if ( u != source )
				Q.push_back( u );
		make_heap( Q.begin(), Q.end(), comp );
		
		while ( Q.size() > 0 )
		{
			// Prendo il primo elemento e lo rimuovo dallo heap.
			uint u = Q.front();
			pop_heap( Q.begin(), Q.end(), comp );
			Q.pop_back();
			
			// Se l'elemento migliore ha distanza infinita, non c'è più niente da fare...
			int costo = getEdge( source, u )->getCost();
			if ( costo == INT_MAX )
				break;
			
			bool found = false;
			uint v, alt;
			for ( auto it = adjList[ u ].begin(); it != adjList[ u ].end(); it++ )
			{
				v = (*it)->getDst( u );
				
				alt = costo + getEdge( u, v )->getCost();
				if ( alt < getEdge( source, v )->getCost() )
				{
					( (DijkyEdge*)getEdge( source, v ) )->setCost( alt );
					
					// Aggiorno la posizione di v.
					// TODO: Teoricamente da fare in modo più efficiente.
					make_heap( Q.begin(), Q.end(), comp );
				}
				
				//// Flag per indicare se esiste già un lato tra source ed u.
				//if ( v == source )
				//	found = true;
			}
			
			//// Se necessario, creo il lato associato a ( source, u ) e lo aggiungo alla lista
			//// Evito di creare autoarchi
			//if ( !found && source != u )
			//{
			//	edges.push_back( new DijkyEdge( source, u ) );
			//	adjList[ source ].push_back( edges.back() );
			//	adjList[ u ].push_back( edges.back() );
			//}
		}
	}
}

// Getter della lista dei lati
vector<Edge*> Graph::getEdges() const
{
	return edges;
}

// Getter della lista di adiacenza di un nodo
vector<Edge*> Graph::getAdjList( uint src ) const
{
	return adjList[ src ];
}

// Getter dei lati
Edge* Graph::getEdge( uint src, uint dst ) const
{
	for ( int i = 0; i < adjList[ src ].size(); i++ )
		if ( adjList[ src ][ i ]->getDst( src ) == dst )
			return adjList[ src ][ i ];
	
	throw;
}

/*** MetaGraph ***/

/**
 * Costruttore
 */
MetaGraph::MetaGraph( Graph g )
{
	this->edges = *new vector<MetaEdge*>();

	int V = g.getEdges().size();
	this->adjList = (vector<MetaEdge*>*)calloc( V, sizeof( vector<MetaEdge*> ) );
	for ( int i = 0; i < V; i++ )
		this->adjList[ i ] = *new vector<MetaEdge*>();

	for ( Edge* edge : g.getEdges() )
		edges.push_back( new MetaEdge( edge ) );


	// Per ogni lato della lista di adiacenza,
	// cerchiamo il suo metalato corrispondente e
	// lo inseriamo alla posizione adeguata della nostra lista di adiacenza	
	for ( int i = 0; i < V; i++ )
		for( Edge* edge : g.getAdjList(i) ){
			MetaEdge* metaEdge;
			for( metaEdge : edges )
				if( metaEdge.getSrc() == edge.getSrc() && 
					metaEdge.getDst() == edge.getDst() )
						break;

			adjList[i].push_back(metaEdge);
		}
}

// Getter dei lati
MetaEdge* MetaGraph::getEdge( uint src, uint dst ) const
{
	for ( int i = 0; i < adjList[ src ].size(); i++ )
		if ( adjList[ src ][ i ]->getDst( src ) == dst )
			return adjList[ src ][ i ];
	
	throw;
}

// Getter della lista dei lati
vector<MetaEdge*> MetaGraph::getEdges() const
{
	return edges;
}

// Getter della lista di adiacenza di un nodo
vector<MetaEdge*> MetaGraph::getAdjList( uint src ) const
{
	return adjList[ src ];
}
