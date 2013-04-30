//
//  solver.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "solver.h"

using namespace std;
using namespace solver;
using namespace model;

/*** Vehicle ***/

Vehicle::Vehicle()
{
	path = *new list<MetaEdge*>();
}

void Vehicle::addEdge( MetaEdge* edge, int index )
{
	edge->setTaken( this );
	
	if ( index == -1 )
		path.push_back( edge );
	else
	{
		// Creo l'iteratore alla posizione richiesta
		list<MetaEdge*>::iterator it = path.begin();
		while ( index-- > 0 )
			++it;
		
		path.insert( it, edge );
	}
}

void Vehicle::removeEdge( int index )
{
	// Rimuovo l'ultimo lato
	if ( index == -1 )
		index = path.size() -1;
	
	int occurrence = 0;
	// Creo l'iteratore alla posizione richiesta
	// e conto le occorrenze del lato da rimuovere
	list<MetaEdge*>::iterator it = next( path.begin(), index );
	for ( auto i = path.begin(); i != it; ++i )
		if ( *i == *it )
			occurrence++;
	
	(*it)->unsetTaken( this, occurrence );
	path.erase( it );
}

unsigned long Vehicle::size()
{
	return path.size();
}

uint Vehicle::getCost()
{
	uint result = 0;
	for ( auto it = path.begin(); it != path.end(); it++ )
		result += (*it)->getCost();
	
	return result;
}

uint Vehicle::getDemand()
{
	uint result = 0;
	for ( auto it = path.begin(); it != path.end(); it++ )
	{
		auto jt = path.begin();
		while ( *(jt++) != *it );

		result += (*it)->getDemand() * ( (*it)->isServer( this ) && jt == it );
	}
	
	return result;
}

string Vehicle::toString()
{
	stringstream ss;
	for ( auto it = path.begin(); it != path.end(); it++ )
		ss << "(" << (*it)->getSrc() + 1 << " " << (*it)->getDst() + 1 << ") ";
	
	return ss.str();
}

/*** Solution ***/

Solution::Solution( int M ):
	M( M )
{
	/*
	 * TODO: Rendere profitto e domanda variabili associate ad ogni soluzione,
	 * da calcolare ogni volta che la soluzione viene modificata.
	 */
	vehicles = (Vehicle*)calloc( M, sizeof( Vehicle ) );
}

void Solution::addEdge( MetaEdge* edge, int vehicle, int index )
{
	vehicles[ vehicle ].addEdge( edge, index );
}

void Solution::removeEdge( int vehicle, int index )
{
	vehicles[ vehicle ].removeEdge( index );
}

unsigned long Solution::size()
{
	return path.size();
}

uint Solution::getCost( Graph g )
{
	uint result = 0;
	for ( list<MetaEdge*>::iterator it = path.begin(); it != path.end(); it++ )
		result += g.getCost( *it );
	
	return result;
}

uint Solution::getDemand()
{
	uint result = 0;
	for ( list<MetaEdge*>::iterator it = path.begin(); it != path.end(); it++ )
		result += (*it)->getDemand();
	
	return result;
}

string Solution::toString()
{
	stringstream ss;
	for ( auto it = path.begin(); it != path.end(); it++ )
		ss << "(" << (*it)->getSrc() + 1 << " " << (*it)->getDst() + 1 << ") ";
	
	return ss.str();
}


/*** Solver ***/

Solver::Solver( Graph graph, uint depot, uint M, uint Q, uint tMax ):
graph( graph ), depot( depot ), M( M ), Q( Q ), tMax( tMax ),
greedyCompare( &graph )
{
	currentSolution = (Solution*)calloc( M, sizeof( Solution ) );
	for ( int i = 0; i < M; i++ )
		currentSolution[ i ] = createBaseSolution();
}

Solution Solver::createBaseSolution()
{
	cerr << endl << "Stampo la soluzione di base:" << endl;
	Solution baseSolution;
	uint currentNode = depot;
	
	// Aggiungo lati finché la soluzione è accettabile ed è possibile tornare al deposito
	vector<MetaEdge*> edges;
	bool full = false;
	while ( !full )
	{
		// Ordino i lati uscenti dal nodo corrente
		edges = graph.getAdjList( currentNode );
		sort( edges.begin(), edges.end(), greedyCompare );
		
		/*
		 for ( MetaEdge* edge : edges )
		 cerr << edge->getSrc() + 1 << " " << edge->getDst() + 1 << ": "
		 << edge->getProfitDemandRatio() << endl;
		 */
		
		// Prendo il lato ammissibile migliore, se esiste
		full = true;
		//for ( int i = 0; i < edges.size(); i++ )
		for( MetaEdge* edge : edges )
		{
			baseSolution.addEdge( edge );
			if ( baseSolution.getDemand() < Q &&
				( baseSolution.getCost( graph ) + graph.getCost( currentNode, depot ) ) < tMax )
			{
				currentNode = edge->getDst( currentNode );
				full = false;
				fprintf( stderr, "Preso %d (%.2f)\n", currentNode + 1, edge->getProfitDemandRatio() );
				break;
			}
			else
				baseSolution.removeEdge();
		}
	}
	baseSolution.addEdge( (MetaEdge*)graph.getEdge( currentNode, depot ) );
	
	cerr << "Soluzione finale: " << baseSolution.toString() << endl;
	
	return baseSolution;
}


Solution* Solver::solve()
{
	/*
	 for ( int i = 0; i < M; i++ )
	 currentSolution[ i ] = createBaseSolution();
	 */
	
	return currentSolution;
}


bool Solver::isFeasible( Solution s )
{
	return s.getCost( graph ) < tMax && s.getDemand() < Q;
}


