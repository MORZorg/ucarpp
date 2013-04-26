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

/*** Solution ***/

Solution::Solution()
{
	/*
	 * TODO: Rendere profitto e domanda variabili associate ad ogni soluzione,
	 * da calcolare ogni volta che la soluzione viene modificata.
	 */
	path = *new list<MetaEdge*>();
}

void Solution::addEdge( MetaEdge* edge, int index )
{
	edge->setTaken();
	
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

void Solution::removeEdge( int index )
{
	if ( index == -1 )
	{
		// Rimuovo l'ultimo lato
		path.back()->unsetTaken();
		path.pop_back();
	}
	else
	{
		// Creo l'iteratore alla posizione richiesta
		list<MetaEdge*>::iterator it = path.begin();
		while ( index-- > 0 )
			++it;
		
		(*it)->unsetTaken();
		path.erase( it );
	}

	/*
	if ( it != path.end() )
		if ( (*it)->unsetTaken() == 0 )
			path.erase( it );
	*/
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
