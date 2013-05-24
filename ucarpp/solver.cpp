//
//  solver.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "solver.h"

#ifndef DEBUG
//#define DEBUG
#endif

using namespace std;
using namespace solver;
using namespace model;

/*** Vehicle ***/

Vehicle::Vehicle( int _id ) : id( _id )
{
	path = list<MetaEdge*>();
}

Vehicle::Vehicle( const Vehicle& source ) : id( source.id )
{
	path = source.path;
}

MetaEdge Vehicle::getEdge( int index ) const
{
	return **next( path.begin(), index );
}

void Vehicle::addEdge( MetaEdge* edge, long index )
{
	list <MetaEdge*>::iterator it;
	if( index == -1 )
		it = path.end();
	else
		it = next( path.begin(), index );

	// Controllo in che punto della lista devo inserire il passaggio del veicolo nel metalato 
	int occurence = 0;
	for( auto i = path.begin(); i != it; ++i )
		if( **i == *edge )
			occurence++;

	edge->setTaken( this, occurence );
	
	if ( index == - 1 || index == path.size() )
		path.push_back( edge );
	else
		path.insert( it, edge );
}

void Vehicle::removeEdge( long index )
{
	// Rimuovo l'ultimo lato
	if ( index == -1 )
		index = path.size() -1;
	
	int occurrence = 0;
	// Creo l'iteratore alla posizione richiesta e conto le occorrenze del lato da rimuovere
	list<MetaEdge*>::iterator it = next( path.begin(), index );
	for ( auto i = path.begin(); i != it; ++i )
		if ( **i == **it )
			occurrence++;
	
	(*it)->unsetTaken( this, occurrence );
	path.erase( it );
}

unsigned long Vehicle::size() const
{
	return path.size();
}

uint Vehicle::getCost() const
{
	uint result = 0;
	for ( auto it = path.begin(); it != path.end(); it++ )
		result += (*it)->getCost();
	
	return result;
}

uint Vehicle::getDemand() const
{
	uint result = 0;
	for ( auto it = path.begin(); it != path.end(); it++ )
	{
		auto jt = path.begin();
		while ( **(jt++) != **it );
		jt--;
		
		result += (*it)->getDemand() * ( (*it)->isServer( this ) && jt == it );
	}
	
	return result;
}

uint Vehicle::getProfit() const
{
	uint result = 0;
	for ( auto it = path.begin(); it != path.end(); it++ )
	{
		auto jt = path.begin();
		while ( **(jt++) != **it );
		jt--;
		
		result += (*it)->getProfit() * ( (*it)->isServer( this ) && jt == it );
	}
	
	return result;
}

// true se la direzione di percorrenza è da src a dst, false altrimenti
bool Vehicle::getDirection( int edge ) const
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

string Vehicle::toString() const
{
	stringstream ss;
	auto it = path.begin();
	if ( it == path.end() )
		return "";
#ifndef DEBUG
	uint previous = (*it)->getSrc();
#endif

	for ( ; it != path.end(); it++ )
	{
#ifndef DEBUG
		ss << "(" << previous + 1 << " ";
		previous = (*it)->getDst( previous );
		ss << previous + 1 << ") ";
#else
		ss << "(" << (*it)->getSrc() << " " << (*it)->getDst() << ") ";
#endif
	}
	ss << " Profitto: " << getProfit() << endl;
	
	return ss.str();
}

// Ritorna l'identificativo del veicolo
int Vehicle::getId() const
{
	return id;
}

/**
 * Comparatore di uguaglianza tra veicoli
 */
bool Vehicle::operator ==( const Vehicle& other ) const
{
	return this->equals( other );
}
bool Vehicle::equals( const Vehicle& other ) const
{
	return getId() == other.getId();
}

/*** Solution ***/


Solution::Solution( int M, Graph graph ):
	M( M ), graph( graph ), compareRatioGreedy( &this->graph )
{
////	vehicles = (Vehicle*)calloc( M, sizeof( Vehicle ) );
//	vehicles = static_cast<Vehicle*> (::operator new ( sizeof( Vehicle ) * M ) );
//	for ( int i = 0; i < M; i++ )
//		new ( &vehicles[ i ] ) Vehicle( i );
	vehicles = vector<Vehicle>();
	for ( int i = 0; i < M; i++ )
		vehicles.push_back( Vehicle( i ) );
}

Solution::Solution( const Solution& source ):
M( source.M ), graph( source.graph ), compareRatioGreedy( &this->graph )
{
	vehicles = vector<Vehicle>();
	for ( int i = 0; i < M; i++ )
		vehicles.push_back( source.vehicles[ i ] );
}

MetaEdge Solution::getEdge( int vehicle, int index ) const
{
	return vehicles[ vehicle ].getEdge( index );
}

void Solution::addEdge( Edge* edge, int vehicle, int index )
{
	vehicles[ vehicle ].addEdge( graph.getEdge( edge ), index );
}

void Solution::removeEdge( int vehicle, int index )
{
	vehicles[ vehicle ].removeEdge( index );
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
	return vehicles[ vehicle ].size();
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
	return vehicles[ vehicle ].getProfit();
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
	return vehicles[ vehicle ].getCost();
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
	return vehicles[ vehicle ].getDemand();
}

bool Solution::getDirection( int vehicle, int index ) const
{
	return vehicles[ vehicle ].getDirection( index );
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
	return vehicles[ vehicle ].toString();
}

/*** Solver ***/

Solver::Solver( Graph graph, uint depot, uint M, uint Q, uint tMax ):
	graph( graph ), depot( depot ), M( M ), Q( Q ), tMax( tMax ),
	currentSolution( createBaseSolution() ) {}

Solution Solver::createBaseSolution()
{
#ifdef DEBUG
	cerr << endl << "Stampo la soluzione di base:" << endl;
#endif
	Solution baseSolution( M, graph );
	
	for ( int i = 0; i < M; i++ )
	{
#ifdef DEBUG
		cerr << "\tVeicolo " << i + 1 << endl;
#endif
		createBaseSolution( &baseSolution, i );
	}
	
#ifdef DEBUG
	cerr << "Soluzione iniziale:\n" << baseSolution.toString() << endl;
#endif
	
	return baseSolution;
}

void Solver::createBaseSolution( Solution* baseSolution, int vehicle )
{
	uint currentNode = depot;
	
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
			bool addedEdge = currentNode != depot;
			baseSolution->addEdge( edge, vehicle );
			if ( addedEdge )
			{
				Edge* returnEdge = graph.getEdge( currentNode, depot );
				baseSolution->addEdge( returnEdge, vehicle );
				
#ifdef DEBUG
				fprintf( stderr, "*** ADDED %d ***\n", currentNode );
#endif
			}
			
#ifdef DEBUG
			cerr << "After added: " << baseSolution->toString( vehicle ) << endl;
#endif
			
			/**
			 * Se la soluzione resta feasible avendo preso
			 * il lato selezionato ed il lato di ritorno,
			 * accetto il nuovo lato e proseguo al successivo.
			 */
			if ( isFeasible( *baseSolution, vehicle ) )
			{
#ifdef DEBUG
				fprintf( stderr, "\t\tPreso %d (r: % 3.2f)\n\n",
						currentNode + 1, edge->getProfitDemandRatio() );
#endif
				
				if ( addedEdge )
					baseSolution->removeEdge( vehicle );
				
				full = false;
				break;
			}
			else
			{
				// Annullo la mossa
				currentNode = edge->getDst( currentNode );
				baseSolution->removeEdge( vehicle );
				
				if ( addedEdge )
					baseSolution->removeEdge( vehicle );
			}
		}
	}
	
	if ( currentNode != depot )
		baseSolution->addEdge( graph.getEdge( currentNode, depot ), vehicle );
}

Solution Solver::vns( int nIter, Solution baseSolution )
{
	// Inizializzo il generatore di numeri casuali
	srand( (uint)time( NULL ) );
	int k = 1;
	// Creo una copia della soluzione iniziale sulla quale applicare la vns
	Solution shakedSolution = baseSolution;
	Solution optimalSolution = baseSolution;
	
	// Ciclo fino a quando la stopping rule me lo consente o prima se trovo una soluzione migliore di quella iniziale
	while ( nIter-- > 0 )
	{
		shakedSolution = baseSolution;
		
		// Estraggo un veicolo ed un lato iniziale casuali
		// Tengo traccia anche dei nodi sorgente e destinazione di tale lato
		uint vehicle = rand() % M,
			 edge,
			 src,
			 dst;
		
		// Se il veicolo è vuoto, prima lo riempio
		if ( shakedSolution.size( vehicle ) == 0 )
			createBaseSolution( &shakedSolution, vehicle );
		
		edge = (uint)( rand() % shakedSolution.size( vehicle ) );
		src = shakedSolution.getEdge( vehicle, edge ).getSrc();
		dst = shakedSolution.getEdge( vehicle, edge ).getDst();
		
		// Barbascambio di variabili a seconda del verso in cui tale lato viene percorso dal veicolo
		if ( !shakedSolution.getDirection( vehicle, edge ) )
			dst ^= src ^= dst ^= src;
		
		// Rimuovo k+1 lati
		// Itero sul minimo valore tra k e la lunghezza attuale della soluzione (k+1<s => k<s-1!!!)
		//  così da non togliere più lati di quanti la soluzione non ne abbia
		int ktemp = ( k < shakedSolution.size( vehicle ) -1 ?
					 k : (uint)shakedSolution.size( vehicle ) -1 );
		// Rimuovo 1 lato
		shakedSolution.removeEdge( vehicle, edge );
		
		// Rimuovo al più k lati
		for ( int i = 0; i < ktemp; i++ )
		{
			// Decido se rimuovere il lato all'inizio (edge-1) o alla fine (edge) del buco creato
			// ( edge > 0 ) serve a garantire che il deposito non venga estromesso dalla soluzione ( se edge è il deposito allora è già stato rimosso un lato a lui connesso e non ne possono essere rimossi altri ), il controllo esterno a verificare circa quasi la stessa cosa
			
			// holeDirection indica in che direzione evolve il buco inserito nella soluzione
			//  - false	:	buco evolve in avanti
			//  - true	:	buco evolve in dietro
			bool holeDirection;
			
			// Controlliamo di non eliminare lati oltre la lunghezza della soluzione corrente
			if( edge >= shakedSolution.size( vehicle ) )
			{
				edge = (uint)( shakedSolution.size( vehicle ) - 1 );
				holeDirection = true;
			}
			// Controlliamo di non eliminare lati precedenti al deposito iniziale
			else if ( edge <= 0 )
			{
				edge = 0;
				holeDirection = false;
			}
			// Nessun problema sull'arco da rimuovere
			else
			{
				holeDirection = rand() & 2;
				edge -= holeDirection;
			}
			
			// Controllo se devo aggiornare il nodo di partenza o destinazione, a seconda del lato rimosso
			if ( holeDirection )
				src = shakedSolution.getEdge( vehicle, edge ).getDst( src );
			else
				dst = shakedSolution.getEdge( vehicle, edge ).getDst( dst );
			
			// Rimuovo il lato scelto dalla soluzione
			shakedSolution.removeEdge( vehicle, edge );
			/*
			 #ifdef DEBUG
			 cerr << " ***********" << endl;
			 cerr << "Edge: " << edge << endl;
			 cerr << "Src: " << src << endl;
			 cerr << "Dst: " << dst << endl;
			 cerr << "Soluzione: " << shakedSolution->toString() << endl;
			 #endif
			 */
		}

#ifdef DEBUG
		cerr << shakedSolution.toString();
		cerr << "Buco " << src << " " << dst << "\tindice " << edge << endl;
#endif
		
		// Inizio con la chiusura della soluzione, partendo dal nodo sorgente, ovvero dove ha inizio il buco
		int kvns = XI * ( k + 1 );
		
		while( !closeSolutionRandom( &shakedSolution, vehicle, src, dst, kvns, edge ) );
//		if ( !closeSolutionRandom( shakedSolution, vehicle, src, dst, kvns, edge ) )
//			throw 200;

#ifdef DEBUG
	cerr << "Soluzioni:" << endl;
	cerr << "Base: " << baseSolution.toString();
	cerr << "Shaked: " << shakedSolution.toString();
	cerr << "Optimal: " << optimalSolution.toString();
#endif

		// Soluzione migliore: maggior profitto o stesso profitto con minori risorse
		// Aggiorno la soluzione con quella più profittevole => mi sposto
		if ( shakedSolution.getProfit() > optimalSolution.getProfit() ||
		   ( shakedSolution.getProfit() == optimalSolution.getProfit() &&
				( shakedSolution.getDemand() < optimalSolution.getDemand() ||
				( shakedSolution.getDemand() == optimalSolution.getDemand() &&
					shakedSolution.getCost() < optimalSolution.getCost() ) ) ) )
		{
#ifdef DEBUG
			cerr << "Soluzione migliorata: " << optimalSolution.getProfit() << " => " << shakedSolution.getProfit() << endl;
#endif
			optimalSolution = shakedSolution;
			
			// ~VND
			baseSolution = shakedSolution;
			
			k = 0;
		}
		
		k = 1 + k % K_MAX;
	}
	
	return optimalSolution;
}

bool Solver::closeSolutionRandom( Solution* solution, int vehicle, uint src, uint dst, int k, int edgeIndex )
{
#ifdef DEBUG
	cerr << "iter: " << k << "\tindice: " << edgeIndex << endl;
#endif

	// Piede della ricorsione: se src == dst ho chiuso ( con probabilità => ammetto ulteriori cicli )
	if ( src == dst && ( ( (float)rand() / RAND_MAX ) <= P_ACCEPT || k <= 1 ) )
		return true;

	// Non posso aggiungere altri lati
	if ( k == 0 )
		return false;

	// Controllo se devo chiudere il ciclo direttamente o meno
	if ( ( k == 1 ) && ( (float)rand() / RAND_MAX ) <= P_CLOSE )
	{
		solution->addEdge( graph.getEdge( src, dst ), vehicle, edgeIndex );

		if( !isFeasible( *solution, vehicle ) )
		{
			solution->removeEdge( vehicle, edgeIndex );
			return false;
		}

		return true;
	}

	// Inizio con la chiusura della soluzione, partendo dal nodo sorgente, ovvero dove ha inizio il buco
	vector <Edge*> edges = graph.getAdjList( src );
	bool* tried = (bool*)calloc( edges.size(), sizeof( bool ) );
	
	uint tries = 1;//(uint)edges.size();
	
	while ( tries-- > 0 )
	{
		// Casualmente prelevo un nodo da inserire nella soluzione che non sia già stato provato
		uint v;
		while ( tried[ v = ( rand() % edges.size() ) ] );
		tried[ v ] = true;
		Edge* victim = edges[ v ];
		solution->addEdge( victim, vehicle, edgeIndex );
		
#ifdef DEBUG
		cerr << "Lato selezionato: (" << victim->getSrc() << "," << victim->getDst() << ")" << endl;
#endif
		
		// Controllo subito se il lato inserito mi porta ad una situazione di soluzione non feasible
		// Se i miei figli non trovano alcun lato buono,
		//  allora elimino il lato inserito fino a tornare alla soluzione iniziale
		if( !isFeasible( *solution, vehicle ) ||
		    !closeSolutionRandom( solution, vehicle, victim->getDst( src ), dst, k - 1, edgeIndex + 1 ) )
		{
#ifdef DEBUG
			cerr << "Unfeasible: " << edgeIndex << " => " << solution->toString();
#endif
			solution->removeEdge( vehicle, edgeIndex );
			continue;
		}
		else
			return true;
	}
	
#ifdef DEBUG
	cerr << "Mi arrendo. " << endl;
	for ( int i = 0; i < edges.size(); i++ )
		cerr << tried[ i ] << " ";
	cerr << endl;
#endif

	return false;

}

Solution Solver::solve()
{
	// Numero di iterazioni
	currentSolution = vns( 200, currentSolution );
	
	return currentSolution;
}

bool Solver::isFeasible( Solution solution, int vehicle ) const
{
	return	solution.getDemand( vehicle ) < Q &&
			solution.getCost( vehicle ) < tMax;
}

