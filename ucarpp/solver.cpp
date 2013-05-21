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

MetaEdge* Vehicle::getEdge( int index ) const
{
	return *next( path.begin(), index );
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

// true se la direzione di percorrenza è da src a dst, false altrimenti
bool Vehicle::getDirection( int edge )
{
	auto desired = next( path.begin(), edge );
	uint previous;
	auto it = path.begin();
	if ( it == path.end() )
		throw 404;
	previous = (*it)->getSrc();
		
	for ( ; it != desired; it++ )
		previous = (*it)->getDst( previous );
	
	return previous == (*it)->getSrc();
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

Solution::Solution( int _M, MetaGraph _graph, Vehicle** _vehicles ):
	M( _M ), graph( _graph ), compareRatioGreedy( &this->graph )
{
	vehicles = (Vehicle**)calloc( M, sizeof( Vehicle* ) );
	for ( int i = 0; i < M; i++ )
		vehicles[ i ] = new Vehicle( *_vehicles[ i ] );
}

Solution::Solution( int M, Graph graph ):
	M( M ), graph( graph ), compareRatioGreedy( &this->graph )
{
	/*
	 * TODO: Rendere profitto e domanda variabili associate ad ogni soluzione,
	 * da calcolare ogni volta che la soluzione viene modificata.
	 */
	vehicles = (Vehicle**)calloc( M, sizeof( Vehicle* ) );
	for ( int i = 0; i < M; i++ )
		vehicles[ i ] = new Vehicle();
}

Solution* Solution::clone() const
{
	return new Solution( M, graph, vehicles );
}

MetaEdge* Solution::getEdge( int vehicle, int index ) const
{
	return vehicles[ vehicle ]->getEdge( index );
}

void Solution::addEdge( Edge* edge, int vehicle, int index )
{
	vehicles[ vehicle ]->addEdge( graph.getEdge( edge ), index );
}

void Solution::removeEdge( int vehicle, int index )
{
	vehicles[ vehicle ]->removeEdge( index );
}

unsigned long Solution::size() const
{
	unsigned long result = 0;
	for ( int i = 0; i < M; i++ )
		result += size( i );
	
	return result;
}

unsigned long Solution::size( int vehicle ) const
{
	return vehicles[ vehicle ]->size();
}

uint Solution::getProfit() const
{
	uint result = 0;
	for ( int i = 0; i < M; i++ )
		result += getProfit( i );
	
	return result;
}

uint Solution::getProfit( int vehicle ) const
{
	return vehicles[ vehicle ]->getProfit();
}

uint Solution::getCost() const
{
	uint result = 0;
	for ( int i = 0; i < M; i++ )
		result += getCost( i );
	
	return result;
}

uint Solution::getCost( int vehicle ) const
{
	return vehicles[ vehicle ]->getCost();
}

uint Solution::getDemand() const
{
	uint result = 0;
	for ( int i = 0; i < M; i++ )
		result += getDemand( i );
	
	return result;
}

uint Solution::getDemand( int vehicle ) const
{
	return vehicles[ vehicle ]->getDemand();
}

bool Solution::getDirection( int vehicle, int index ) const
{
	return vehicles[ vehicle ]->getDirection( index );
}

string Solution::toString() const
{
	stringstream ss;
	for ( int i = 0; i < M; i++ )
		ss << i + 1 << ":\t" << toString( i ) << endl;
	
	return ss.str();
}

string Solution::toString( int vehicle ) const
{
	return vehicles[ vehicle ]->toString();
}

/*** Solver ***/

Solver::Solver( Graph graph, uint depot, uint M, uint Q, uint tMax ):
	graph( graph ), depot( depot ), M( M ), Q( Q ), tMax( tMax ),
	currentSolution( createBaseSolution() ) {}

Solution* Solver::createBaseSolution()
{
#ifdef DEBUG
	cerr << endl << "Stampo la soluzione di base:" << endl;
#endif
	Solution* baseSolution = new Solution( M, graph );
	uint currentNode;
	
	for ( int i = 0; i < M; i++ )
	{
#ifdef DEBUG
		cerr << "\tVeicolo " << i + 1 << endl;
#endif
		
		currentNode = depot;
		
		// Aggiungo lati finché la soluzione è accettabile ed è possibile tornare al deposito
		vector<Edge*> edges;
		bool full = false;
		while ( !full )
		{
			// Ordino i lati uscenti dal nodo corrente
			edges = graph.getAdjList( currentNode );
			sort( edges.begin(), edges.end(), baseSolution->compareRatioGreedy );
			
			// Prendo il lato ammissibile migliore, se esiste
			full = true;
			for( Edge* edge : edges )
			{
				currentNode = edge->getDst( currentNode );
				
				// Aggiungo il lato selezionato e, in caso non sia tornato al deposito,
				//  il lato necessario alla chiusura.
				bool addedEdge;
				baseSolution->addEdge( edge, i );
				if ( ( addedEdge = ( currentNode != depot ) ) )
				{
					Edge* returnEdge = graph.getEdge( currentNode, depot );
					baseSolution->addEdge( returnEdge, i );
				}
				
				/**
				 * Se la soluzione resta feasible avendo preso
				 * il lato selezionato ed il lato di ritorno,
				 * accetto il nuovo lato e proseguo al successivo.
				 */
				if (	baseSolution->getDemand( i ) < Q &&
						baseSolution->getCost( i ) < tMax )
				{
#ifdef DEBUG
					fprintf( stderr, "\t\tPreso %d (r: % 3.2f)\n",
							currentNode + 1, edge->getProfitDemandRatio() );
#endif
					
					full = false;
					break;
				}
				else
				{
					// Annullo la mossa
					currentNode = edge->getDst( currentNode );
					baseSolution->removeEdge( i );
				}
				
				if ( addedEdge )
					baseSolution->removeEdge( i );
			}
		}
		
		if ( currentNode != depot )
			baseSolution->addEdge( graph.getEdge( currentNode, depot ), i );
	}
	
#ifdef DEBUG
	cerr << "Soluzione iniziale:\n" << baseSolution->toString() << endl;
#endif
	
	return baseSolution;
}

Solution* Solver::vns( int nIter, Solution* baseSolution )
{
	srand( (uint)time( NULL ) );
	int kMax = 10,
		k = 1;
	Solution* shakedSolution = baseSolution->clone();
	
	while ( nIter > 0 )
	{
		nIter--;
		
		// Estraggo un veicolo ed un lato iniziale casuali
		uint vehicle = rand() % M,
			 edge = (uint)( rand() % shakedSolution->size( vehicle ) ),
			 src = shakedSolution->getEdge( vehicle, edge )->getSrc(),
			 dst = shakedSolution->getEdge( vehicle, edge )->getDst();

		if ( !shakedSolution->getDirection( vehicle, edge ) )
			dst ^= src ^= dst ^= src;
		
		// Rimuovo k+1 lati
		// Itero sul minimo valore tra k e la lunghezza attuale della soluzione, così da non togliere più lati di quanti la soluzione non ne abbia
		uint ktemp = ( k < shakedSolution->size( vehicle ) ? k : shakedSolution->size( vehicle ) );
		// Rimuovo 1 lato
		shakedSolution->removeEdge( vehicle, edge );
		// Rimuovo al più k lati
		for ( int i = 0; i < ktemp; i++ )
		{
			// Decido se rimuovere il lato all'inizio (edge-1) o alla fine (edge) del buco creato
			// ( edge > 0 ) serve a garantire che il deposito non venga estromesso dalla soluzione ( se edge è il deposito allora è già stato rimosso un lato a lui connesso e non ne possono essere rimossi altri ), il controllo esterno a verificare circa quasi la stessa cosa
			// ls -alF
			bool lsaf = rand() & 2;
			edge -= lsaf;
			if( edge >= shakedSolution->size( vehicle ) )
			{
				edge = shakedSolution->size( vehicle ) - 1;
				lsaf = false;
			}
			else if ( edge <= 0 )
			{
				edge = 0;
				lsaf = false;
			}

			if ( lsaf )
				src = shakedSolution->getEdge( vehicle, edge )->getDst( src );
			else
				dst = shakedSolution->getEdge( vehicle, edge )->getDst( dst );

			shakedSolution->removeEdge( vehicle, edge );
		}
		
#ifdef DEBUG
		cerr << shakedSolution->toString() << endl;
		cerr << "Buco " << src << " " << dst << endl;
#endif
		
		k = 1 + k % kMax;
	}
	
	return baseSolution;
}

Solution* Solver::solve()
{
	// Numero di iterazioni
	return vns( 1, currentSolution );
	//return currentSolution;
}

