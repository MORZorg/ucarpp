//
//  solver.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "solver.h"

using namespace std;

/*** Solver ***/

Solver::Solver( Graph graph, uint depot, uint M, uint Q, uint tMax ):
	graph( graph ), depot( depot ), M( M ), Q( Q ), tMax( tMax ),
	greedyCompare( &graph )
{
	currentSolution = (Solution*)calloc( M, sizeof( Solution ) );
	for ( int i = 0; i < M; i++ )
		currentSolution[ i ] = *new Solution();
}

Solution Solver::createBaseSolution()
{
	cerr << endl << "Stampo la soluzione di base:" << endl;
	Solution baseSolution;
	uint currentNode = depot;
	
	// Aggiungo lati finché la soluzione è accettabile ed è possibile tornare al deposito
	vector<Edge*> edges;
	bool full = false;
	while ( !full )
	{
		// Ordino i lati uscenti dal nodo corrente
		edges = graph.getAdjList( currentNode );
		sort( edges.begin(), edges.end(), greedyCompare );
		
		/*
		for ( Edge* edge : edges )
			cerr << edge->getSrc() + 1 << " " << edge->getDst() + 1 << ": "
				 << edge->getProfitDemandRatio() << endl;
		*/

		// Prendo il lato ammissibile migliore, se esiste
		full = true;
		//for ( int i = 0; i < edges.size(); i++ )
		for( Edge* edge : edges )
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
				baseSolution.removeEdge( edge );
		}
	}
	baseSolution.addEdge( graph.getEdge( currentNode, depot ) );
	
	cerr << "Soluzione finale: " << baseSolution.toString() << endl;
	
	return baseSolution;
}

Solution* Solver::solve()
{
	for ( int i = 0; i < M; i++ )
		currentSolution[ i ] = createBaseSolution();
	
	return currentSolution;
}

bool Solver::isFeasible( Solution s )
{
	return s.getCost( graph ) < tMax && s.getDemand() < Q;
}


/*** Solution ***/
Solution::Solution()
{
	path = *new unordered_set<Edge*>();
}

void Solution::addEdge( Edge* edge )
{
	edge->setTaken();
	path.insert( edge );
}

void Solution::removeEdge( Edge* edge )
{
	unordered_set<Edge*>::iterator it = path.find( edge );
	
	if ( it != path.end() )
		if ( (*it)->unsetTaken() == 0 )
			path.erase( it );
}

uint Solution::getCost( Graph g )
{
	uint result = 0;
	for ( unordered_set<Edge*>::iterator it = path.begin(); it != path.end(); it++ )
		result += g.getCost( *it ) * (*it)->getTaken();
	
	return result;
}

uint Solution::getDemand()
{
	uint result = 0;
	for ( unordered_set<Edge*>::iterator it = path.begin(); it != path.end(); it++ )
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
