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

void Vehicle::addEdge( MetaEdge* edge, long index )
{
	// TODO: fare tutta la roba del removeEdge
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

void Vehicle::removeEdge( long index )
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
		jt--;
		
		result += (*it)->getDemand() * ( (*it)->isServer( this ) && jt == it );
	}
	
	return result;
}

uint Vehicle::getProfit()
{
	uint result = 0;
	for ( auto it = path.begin(); it != path.end(); it++ )
	{
		auto jt = path.begin();
		while ( *(jt++) != *it );
		jt--;
		
		result += (*it)->getProfit() * ( (*it)->isServer( this ) && jt == it );
	}
	
	return result;
}

string Vehicle::toString()
{
	stringstream ss;
	uint previous;
	auto it = path.begin();
	if ( it == path.end() )
		return "";
	previous = (*it)->getSrc();
		
	for ( ; it != path.end(); it++ )
	{
		ss << "(" << previous + 1 << " ";
		previous = (*it)->getDst( previous );
		ss << previous + 1 << ") ";
	}
	ss << getProfit() << endl;
	
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
	vehicles = (Vehicle**)calloc( M, sizeof( Vehicle* ) );
	for ( int i = 0; i < M; i++ )
		vehicles[ i ] = new Vehicle();
}

void Solution::addEdge( MetaEdge* edge, int vehicle, int index )
{
	vehicles[ vehicle ]->addEdge( edge, index );
}

void Solution::removeEdge( int vehicle, int index )
{
	vehicles[ vehicle ]->removeEdge( index );
}

unsigned long Solution::size()
{
	int result = 0;
	for ( int i = 0; i < M; i++ )
		result += vehicles[ i ]->size();
	return result;
}

uint Solution::getCost()
{
	uint result = 0;
	for ( int i = 0; i < M; i++ )
		result += getCost( i );
	
	return result;
}

uint Solution::getCost( int vehicle )
{
	return vehicles[ vehicle ]->getCost();
}

uint Solution::getDemand()
{
	uint result = 0;
	for ( int i = 0; i < M; i++ )
		result += getDemand( i );
	
	return result;
}

uint Solution::getDemand( int vehicle )
{
	return vehicles[ vehicle ]->getDemand();
}

string Solution::toString()
{
	stringstream ss;
	for ( int i = 0; i < M; i++ )
		ss << i + 1 << ":\t" << toString( i ) << endl;
	
	return ss.str();
}

string Solution::toString( int vehicle )
{
	return vehicles[ vehicle ]->toString();
}


/*** Solver ***/

Solver::Solver( MetaGraph graph, uint depot, uint M, uint Q, uint tMax ):
	graph( graph ), depot( depot ), M( M ), Q( Q ), tMax( tMax ),
	greedyCompare(), currentSolution( createBaseSolution() ) {}

Solution Solver::createBaseSolution()
{
#ifdef DEBUG
	cerr << endl << "Stampo la soluzione di base:" << endl;
#endif
	Solution baseSolution( M );
	uint currentNode = depot;
	
	for ( int i = 0; i < M; i++ )
	{
#ifdef DEBUG
		cerr << "\tVeicolo " << i + 1 << endl;
#endif
		// Aggiungo lati finché la soluzione è accettabile ed è possibile tornare al deposito
		vector<MetaEdge*> edges;
		bool full = false;
		while ( !full )
		{
			// Ordino i lati uscenti dal nodo corrente
			edges = graph.getAdjList( currentNode );
			sort( edges.begin(), edges.end(), greedyCompare );
			
			/**
			 *
			 * for ( MetaEdge* edge : edges )
			 * cerr << edge->getSrc() + 1 << " " << edge->getDst() + 1 << ": "
			 * << edge->getProfitDemandRatio() << endl;
			 */
			
			// Prendo il lato ammissibile migliore, se esiste
			full = true;
			//for ( int i = 0; i < edges.size(); i++ )
			for( MetaEdge* edge : edges )
			{
				currentNode = edge->getDst( currentNode );
				
				uint returnCost;
				if ( currentNode == depot )
					returnCost = 0;
				else
					returnCost = graph.getEdge( currentNode, depot )->getCost();
				
				baseSolution.addEdge( edge, i );
				if ( baseSolution.getDemand( i ) < Q &&
				   ( baseSolution.getCost( i ) + returnCost ) < tMax )
				{
					full = false;
#ifdef DEBUG
					fprintf( stderr, "\t\tPreso %d (%.2f)\n", currentNode + 1, edge->getProfitDemandRatio() );
#endif
					break;
				}
				else
				{
					// Annullo la mossa
					currentNode = edge->getDst( currentNode );
					baseSolution.removeEdge( i );
				}
			}
		}
		
		if ( currentNode != depot )
			baseSolution.addEdge( graph.getEdge( currentNode, depot ), i );
	}
	
	cerr << "Soluzione finale:\n" << baseSolution.toString() << endl;
	
	return baseSolution;
}


Solution Solver::solve()
{
	return currentSolution;
}


