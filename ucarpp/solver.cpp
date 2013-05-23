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

Vehicle::Vehicle( int _id ) : id( _id )
{
	path = *new list<MetaEdge*>();
}

MetaEdge* Vehicle::getEdge( int index ) const
{
	return *next( path.begin(), index );
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
		if( *i == edge )
			occurence++;

	edge->setTaken( this, occurence );
	
	if ( index == ( path.size() - 1 ) )
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
		
	//// Ben formattata
	//for ( ; it != path.end(); it++ )
	//{
	//	ss << "(" << previous + 1 << " ";
	//	previous = (*it)->getDst( previous );
	//	ss << previous + 1 << ") ";
	//}
	//ss << " Profitto: " << getProfit() << endl;

	// Mal formattata
	for ( ; it != path.end(); it++ )
	{
		ss << "(" << (*it)->getSrc() << " ";
		ss << (*it)->getDst() << ") ";
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
bool Vehicle::equals( const Vehicle* other ) const
{
	return getId() == other->getId();
}

/*** Solution ***/

Solution::Solution( int _M, MetaGraph _graph, Vehicle** _vehicles ):
	M( _M ), graph( *(_graph.clone() ) ), compareRatioGreedy( &this->graph )
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
		vehicles[ i ] = new Vehicle( i );
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
				bool addedEdge = currentNode != depot;
				baseSolution->addEdge( edge, i );
				if ( addedEdge )
				{
					Edge* returnEdge = graph.getEdge( currentNode, depot );
					baseSolution->addEdge( returnEdge, i );

#ifdef DEBUG
					fprintf( stderr, "*** ADDED %d ***\n", currentNode );
#endif
				}

#ifdef DEBUG
				cerr << "After added: " << baseSolution->toString( i ) << endl;
#endif
				
				/**
				 * Se la soluzione resta feasible avendo preso
				 * il lato selezionato ed il lato di ritorno,
				 * accetto il nuovo lato e proseguo al successivo.
				 */
				if ( isFeasible( baseSolution, i ) )
				{
#ifdef DEBUG
					fprintf( stderr, "\t\tPreso %d (r: % 3.2f)\n\n",
							currentNode + 1, edge->getProfitDemandRatio() );
#endif
					
					if ( addedEdge )
						baseSolution->removeEdge( i );

					full = false;
					break;
				}
				else
				{
					// Annullo la mossa
					currentNode = edge->getDst( currentNode );
					baseSolution->removeEdge( i );

					if ( addedEdge )
						baseSolution->removeEdge( i );
				}
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
	// Inizializzo il generatore di numeri casuali
	srand( (uint)time( NULL ) );
	int k = 1;
	// Creo una copia della soluzione iniziale sulla quale applicare la vns
	Solution* shakedSolution; 
	Solution* optimalSolution = baseSolution->clone();
	
	// Ciclo fino a quando la stopping rule me lo consente o prima se trovo una soluzione migliore di quella iniziale
	while ( nIter > 0 )
	{
		shakedSolution = baseSolution->clone();
		nIter--;
		
		// Estraggo un veicolo ed un lato iniziale casuali
		// Tengo traccia anche dei nodi sorgente e destinazione di tale lato
		uint vehicle = rand() % M,
			 edge = (uint)( rand() % shakedSolution->size( vehicle ) ),
			 src = shakedSolution->getEdge( vehicle, edge )->getSrc(),
			 dst = shakedSolution->getEdge( vehicle, edge )->getDst();

		// Barbascambio di variabili a seconda del verso in cui tale lato viene percorso dal veicolo
		if ( !shakedSolution->getDirection( vehicle, edge ) )
			dst ^= src ^= dst ^= src;
		
		// Rimuovo k+1 lati
		// Itero sul minimo valore tra k e la lunghezza attuale della soluzione
		//  così da non togliere più lati di quanti la soluzione non ne abbia
		int ktemp = ( k < shakedSolution->size( vehicle ) ?
						k : shakedSolution->size( vehicle ) );
		// Rimuovo 1 lato
		shakedSolution->removeEdge( vehicle, edge );
/*
#ifdef DEBUG
			cerr << " ***********" << endl;
			cerr << "Edge: " << edge << endl;
			cerr << "Src: " << src << endl;
			cerr << "Dst: " << dst << endl;
			cerr << "Soluzione: " << shakedSolution->toString() << endl;
#endif
*/
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
			if( edge >= shakedSolution->size( vehicle ) )
			{
				edge = shakedSolution->size( vehicle ) - 1;
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
				src = shakedSolution->getEdge( vehicle, edge )->getDst( src );
			else
				dst = shakedSolution->getEdge( vehicle, edge )->getDst( dst );

			// Rimuovo il lato scelto dalla soluzione
			shakedSolution->removeEdge( vehicle, edge );
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
		cerr << shakedSolution->toString() << endl;
		cerr << "Buco " << src << " " << dst << endl;
#endif
		
		// Inizio con la chiusura della soluzione, partendo dal nodo sorgente, ovvero dove ha inizio il buco
		int kvns = XI * ( k + 1 );

/*
 *		uint currentNode;
 *		vector <Edge*> edges;
 *		// Indica lo stato di evoluzione della soluzione
 *		//  - true: soluzione trovata
 *		//  - 1: soluzione feasible non chiusa
 *		//  - 2: soluzione unfeasible
 *		uint solutionNotFound = 1;
 *
 *		// Ciclo fino a quando non trovo una nuova soluzione
 *		while( solutionNotFound )
 *		{
 *			currentNode = src;
 *
 *			// Creo un ciclo nel lungo al più kvns passi all'interno del quale cerco di richiudere casualmente la soluzione
 *			for( int i = 0; i < ( kvns - 1 ); i++ )
 *			{
 *#ifdef DEBUG
 *				cerr << "i: " << i << endl;
 *#endif
 *
 *				// Prendo i nodi adiacenti al nodo di partenza
 *				edges = graph.getAdjList( currentNode );
 *				// Casualmente prelevo un nodo da inserire nella soluzione
 *				Edge* victim = edges[ rand() % edges.size() ];
 *				shakedSolution->addEdge( victim, vehicle, edge );
 *
 *#ifdef DEBUG
 *				cerr << "Lato selezionato: (" << victim->getSrc() << "," << victim->getDst() << ")" << endl;
 *#endif
 *
 *				// Controllo subito se il lato inserito mi porta ad una situazione di soluzione non feasible
 *				if ( !isFeasible( shakedSolution, vehicle ) )
 *				{
 *					cerr << "BEFORE: current node unfeasible: " << currentNode << " => " << src << " : src" << " => " << shakedSolution->toString() << endl;
 *
 *					for( int j = 0; j <= i; j++ )
 *						shakedSolution->removeEdge( vehicle, edge - j );
 *
 *					edge = edge - i;
 *					currentNode = src;
 *
 *					cerr << "AFTER: current node unfeasible: " << currentNode << " => " << src << " : src" << " => " << shakedSolution->toString() << endl;
 *
 *					// Blocco il ciclo for e inizierò da capo la procedura casuale per trovare una nuova soluzione
 *					solutionNotFound = 2;
 *
 *#ifdef DEBUG
 *					cerr << "Soluzione non feasible, ricomincio" << endl;
 *#endif
 *
 *					break;
 *				}
 *
 *				// Aggiorno il nuovo nodo sorgente
 *				currentNode = victim->getDst( currentNode );
 *				edge++;
 *
 *				// La soluzione è feasible: controllo se la soluzione forma un ciclo, verificando se l'ultimo lato inserito termina sul mio nodo destinazione.
 *				if( currentNode == dst )
 *				{
 *					solutionNotFound = false; 
 *#ifdef DEBUG
 *					cerr << "Soluzione chiusa random: " << shakedSolution->toString() << endl;
 *#endif
 *					break;
 *				}
 *
 *#ifdef DEBUG
 *				cerr << "Solution state: " << solutionNotFound << endl;
 *#endif
 *			}
 *			
 *
 *			// A seconda dello stato in cui mi trovo elaboro la soluzione corrente in modo differente
 *			if( solutionNotFound == 1 )
 *			{
 *				// La soluzione è corretta, ma non chiusa.
 *				// Casualmente decido se chiuderla con il nodo di destinazione o se ripetere tutto casualmente
 *				if( ( (float) rand() / RAND_MAX ) < P_CLOSE )
 *				{
 *					// Chiudo con il nodo destinazione
 *					Edge* finalEdge = graph.getEdge( currentNode, dst );
 *					shakedSolution->addEdge( finalEdge, vehicle, currentNode );
 *
 *#ifdef DEBUG
 *					cerr << "Chiudo direttamente" << endl;
 *#endif
 *
 *					// Controllo che la soluzione sia feasible
 *					if( !isFeasible( shakedSolution, vehicle ) )
 *					{
 *						for( int j = 1; j <= kvns; j++ )
 *							shakedSolution->removeEdge( vehicle, edge - j );
 *
 *						currentNode = src;
 *						edge = edge - kvns;
 *						solutionNotFound = 2;
 *					}
 *					else
 *					{
 *						solutionNotFound = false;
 *						currentNode = finalEdge->getDst();
 *
 *#ifdef DEBUG
 *						cerr << "Soluzione chiusa diretta: " << shakedSolution->toString() << endl;
 *#endif
 *
 *					}
 *				}
 *				else
 *				{
 *					for( int j = 1; j <= kvns - 1; j++ )
 *						shakedSolution->removeEdge( vehicle, edge - j );
 *
 *					currentNode = src;
 *					edge = edge - kvns + 1;
 *					solutionNotFound = true;
 *#ifdef DEBUG
 *					cerr << "Ripristino la soluzione iniziale: " << shakedSolution->toString() << endl;
 *#endif
 *				}
 *
 *			}
 *		}
 *		
 *#ifdef DEBUG
 *		cerr << "Closed: " << shakedSolution->toString() << endl;
 *#endif
 */

		while( !closeSolutionRandom( shakedSolution, vehicle, src, dst, kvns, edge ) );

#ifdef DEBUG
	cerr << "Soluzioni:" << endl;
	cerr << "Base: " << baseSolution->toString();
	cerr << "Shaked: " << shakedSolution->toString();
	cerr << "Optimal: " << optimalSolution->toString();
#endif

		// Se sbar  dasukh  > asfh 
		// Aggiorno la soluzione con quella più profittevole
		if( shakedSolution->getProfit() > baseSolution->getProfit() )
		{
#ifdef DEBUG
			cerr << "Soluzione migliorata: " << optimalSolution->getProfit() << " => " << shakedSolution->getProfit() << endl;
#endif
			optimalSolution = shakedSolution;
			k = 0;
		}

#ifdef DEBUG
		cerr << "Chiusura " << shakedSolution->toString();
#endif
		
		k = 1 + k % K_MAX;
	}
	
	return optimalSolution;
}

bool Solver::closeSolutionRandom( Solution* solution, int vehicle, uint src, uint dst, int k, int edgeIndex )
{
#ifdef DEBUG
	cerr << "k: " << k << endl;
#endif

	// Piede della ricorsione: se src == dst ho chiuso 
	if( src == dst )
		return true;

	// Non posso aggiungere altri lati
	if( k == 0 )
		return false;

	// Controllo se devo chiudere il ciclo direttamente o meno
	if( ( k == 1 ) && ( (float) rand() / RAND_MAX ) < P_CLOSE )
	{
		solution->addEdge( graph.getEdge( src, dst ), vehicle, edgeIndex );

		if( !isFeasible( solution, vehicle ) )
		{
			solution->removeEdge( vehicle, edgeIndex );
			return false;
		}

		return true;
	}

	// Inizio con la chiusura della soluzione, partendo dal nodo sorgente, ovvero dove ha inizio il buco
	vector <Edge*> edges = graph.getAdjList( src );
	// Casualmente prelevo un nodo da inserire nella soluzione
	Edge* victim = edges[ rand() % edges.size() ];
	solution->addEdge( victim, vehicle, edgeIndex );

#ifdef DEBUG
	cerr << "Lato selezionato: (" << victim->getSrc() << "," << victim->getDst() << ")" << endl;
#endif

	// Controllo subito se il lato inserito mi porta ad una situazione di soluzione non feasible
	if ( !isFeasible( solution, vehicle ) )
	{
		cerr << "BEFORE: current node unfeasible: " << src << " => " << solution->toString();

		solution->removeEdge( vehicle, edgeIndex );
		return false;
	}

	// Se i miei figli non trovano alcun lato buono, allora elimino il lato inserito fino a tornare alla soluzione iniziale
	if( !closeSolutionRandom( solution, vehicle, victim->getDst( src ), dst, k - 1, edgeIndex + 1 ) )
	{
		solution->removeEdge( vehicle, edgeIndex );
		return false;
	}

	return true;

}

Solution* Solver::solve()
{
	// Numero di iterazioni
	return vns( 10, currentSolution );
	//return currentSolution;
}

bool Solver::isFeasible( Solution* solution, int vehicle ) const
{
	return	solution->getDemand( vehicle ) < Q &&
			solution->getCost( vehicle ) < tMax;
}

