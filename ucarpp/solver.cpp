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


/*** Solver ***/

Solver::Solver( Graph graph, uint depot, uint M, uint Q, uint tMax ):
	graph( graph ), depot( depot ), M( M ), Q( Q ), tMax( tMax ),
	currentSolution( createBaseSolution() ) {}

Solution Solver::createBaseSolution()
{
#ifdef DEBUG
	cerr << endl << "Stampo la soluzione di base:" << endl;
	cerr << strcmp( "ciao", "ciao" ) << endl;
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
			if ( isFeasible( baseSolution, vehicle ) )
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

Solution Solver::vnasd( int nIter, Solution baseSolution, int repetition )
{
	float iterations = nIter / ( 2 * repetition );

	for( int i = 0; i < repetition; i++ )
		baseSolution = vnd( floor( iterations ), vns( ceil( iterations ), baseSolution ) );

	return baseSolution;
}

Solution Solver::vns( int nIter, Solution baseSolution )
{
	// Inizializzo il generatore di numeri casuali
	srand( (uint)time( NULL ) );
	int k = 1;
	// Creo una copia della soluzione iniziale sulla quale applicare la vns
	Solution shakedSolution = baseSolution;
	Solution* optimalSolution = new Solution( baseSolution );

	// Se richiesto, stampo i risultati su un file esterno
	if( output_file.is_open() )
	{
		printToFile( &shakedSolution );
		printToFile( &baseSolution );
	}
	
	// Ciclo fino a quando la stopping rule me lo consente o prima se trovo una soluzione migliore di quella iniziale
	while ( nIter-- > 0 )
	{
		// Copio la soluzione di base su una soluzione che elaborerò nella vns
		shakedSolution = Solution( baseSolution );
		
		/*** Shaking ***/
		// Estraggo un veicolo ed un lato iniziale casuali
		// Tengo traccia anche dei nodi sorgente e destinazione di tale lato
		uint vehicle = rand() % M;

		// Non mi interesso del valore di ritorno perché pressoché inutile. :D
		mutateSolution( &shakedSolution, vehicle, ceil( XI * ( k + 1 ) ) );

#ifdef DEBUG
		cerr << "Soluzioni:" << endl;
		cerr << "Base: " << baseSolution.toString();
		cerr << "Shaked: " << shakedSolution.toString();
		cerr << "Optimal: " << optimalSolution->toString();
#endif
		
		/*** Ricerca locale ***/
		// Adottiamo il criterio di best improvement e non first.
		// Per prima cosa dobbiamo copiare la soluzione corrente in una temporanea che indicherà la soluzione con massimo profitto trovato.
		Solution maxSolution = Solution( baseSolution );

		for ( int v = 0; v < M; v++ )
		{
			// Cerco di ottimizzare il veicolo appena shakerato
			mrBeanBeanBinPacking( &shakedSolution, v );

#ifdef DEBUG
			cerr << "Ricerco localmente su " << v << ": " << shakedSolution.toString( v );
#endif

			uint previous = depot;
			uint next;
			for ( int i = 0; i < shakedSolution.size( v ); i++ )
			{
				// Elimino almeno un lato
				list <Edge*> removedEdges;
				removedEdges.push_back( shakedSolution.getEdge( v, i )->getEdge() );
				shakedSolution.removeEdge( v, i );
				next = removedEdges.front()->getDst( previous );

				// Elimino lati dalla soluzione fintanto che questi non ne aumentano il profitto e fintanto che sono presenti nella soluzione
				while( i <  shakedSolution.size( v ) )
				{
					// Calcolo la differenza di profitto che abbiamo nel togliere un lato alla soluzione
					int diffProfit = shakedSolution.getProfit( v );

					// Rimuovo il lato i
					Edge* temp = shakedSolution.getEdge( v, i )->getEdge();
					shakedSolution.removeEdge( v, i );

					diffProfit -= shakedSolution.getProfit( v );

					// Se non ho differenze di profitto, tolgo quel lato dalla soluzione
					if( diffProfit == 0 )
					{
						// Inserisco il lato tolto nella lista
						removedEdges.push_back( temp );
						// Sposto il nodo di partenza
						next = temp->getDst( next );
					}
					else
					{
						// Altrimenti lo riaggiungo
						shakedSolution.addEdge( temp, v, i );
						break;
					}

				}

#ifdef DEBUG
				cerr << "Creato un buco di " << removedEdges.size() << " lati" << endl;
#endif

				// Chiedo a Dijkstra di calcolarmi la chiusura migliore
				list<Edge*> closure = closeSolutionDijkstra( shakedSolution, v, previous, next, i );
				previous = next;

				// Se questo porta un miglioramento, effettuo la chiusura, altrimenti riaggiungo il lato i
				for ( auto it = closure.rbegin(); it != closure.rend(); ++it )
					shakedSolution.addEdge( *it, v, i );

				// Controllo se ho trovato una soluzione migliore della massima trovata in precedenza
				if ( shakedSolution > maxSolution )
				{
#ifdef DEBUG
					cerr << "Migliorato" << endl;
#endif
					maxSolution = Solution( shakedSolution );
					//break; // SFANCULATI BEST
				}

				// Resetto la shakedSolution per effettuare una nuova ricerca
				for( int j = 0; j < closure.size(); j++ )
					shakedSolution.removeEdge( v, i );

				for( auto it = removedEdges.rbegin(); it != removedEdges.rend(); ++it )
					shakedSolution.addEdge( *it, v, i );

				i += removedEdges.size() - 1;
			}
		}
		
		
		/*** Move or not ***/
		// Soluzione migliore: maggior profitto o stesso profitto con minori risorse
		// Aggiorno la soluzione con quella più profittevole => mi sposto
		if ( maxSolution > baseSolution )
		{
#ifdef DEBUG
			cerr << "Soluzione migliorata: " << baseSolution.getProfit() << " => " << maxSolution.getProfit() << endl;
#endif
			// La prendo come soluzione ottima se il miglioramento è assoluto
			if ( maxSolution > *optimalSolution )
			{
#ifdef DEBUG
				cerr << "Nuovo massimo: " << optimalSolution->getProfit() << " => " << maxSolution.getProfit() << endl;
#endif
				// Cancello l'oggetto per liberare memoria e poi lo ricreo
				delete optimalSolution;
				optimalSolution = new Solution( maxSolution );
			}
			
			// Salvo la nuova soluzione come soluzione di base per i cicli successivi
			baseSolution = maxSolution;
			
			k = 0;
		}

		// Come all'inizio, se richiesto stampo su file i risultati
		if( output_file.is_open() )
		{
			printToFile( &shakedSolution );
			printToFile( &baseSolution );
		}

		// Incremento il numero di iterazioni svolte
		k = 1 + k % K_MAX;
	}
	
#ifdef DEBUG
	cerr << "VNS" << optimalSolution->toString();
#endif
	return *optimalSolution;
}

Solution Solver::vnd( int nIter, Solution baseSolution )
{
	// Inizializzo il generatore di numeri casuali
	srand( (uint)time( NULL ) );
	int k = 1;
	// Creo una copia della soluzione iniziale sulla quale applicare la vns
	Solution shakedSolution = baseSolution;
	Solution* optimalSolution = new Solution( baseSolution );
	
	// Se richiesto, stampo i risultati su un file esterno
	if( output_file.is_open() )
	{
		printToFile( &shakedSolution );
		printToFile( &baseSolution );
	}

	// Ciclo fino a quando la stopping rule me lo consente o prima se trovo una soluzione migliore di quella iniziale
	while ( nIter-- > 0 )
	{
		shakedSolution = Solution( baseSolution );
		
		/*** Shaking ***/
		// Estraggo un veicolo ed un lato iniziale casuali
		// Tengo traccia anche dei nodi sorgente e destinazione di tale lato
		uint vehicle = rand() % M,
			 edge,
			 src,
			 dst;
		
		edge = openSolutionRandom( &shakedSolution, vehicle, k, &src, &dst );
		
#ifdef DEBUG
		cerr << shakedSolution.toString();
		cerr << "Buco " << src << " " << dst << "\tindice " << edge << endl;
#endif
		
		// Cerco di ottimizzare il veicolo appena shakerato
		mrBeanBeanBinPacking( &shakedSolution, vehicle );

		list<Edge*> closure = closeSolutionDijkstra( shakedSolution, vehicle, src, dst, edge );
		for ( auto it = closure.rbegin(); it != closure.rend(); ++it )
			shakedSolution.addEdge( *it, vehicle, edge );
		
#ifdef DEBUG
		cerr << "Soluzioni:" << endl;
		cerr << "Base: " << baseSolution.toString();
		cerr << "Shaked: " << shakedSolution.toString();
		cerr << "Optimal: " << optimalSolution->toString();
#endif
		
		/*** Move or not ***/
		// Soluzione migliore: maggior profitto o stesso profitto con minori risorse
		// Aggiorno la soluzione con quella più profittevole => mi sposto
		if ( shakedSolution > baseSolution )
		{
#ifdef DEBUG
			cerr << "Soluzione migliorata: " << baseSolution.getProfit() << " => " << shakedSolution.getProfit() << endl;
#endif
			if ( shakedSolution > *optimalSolution )
			{
#ifdef DEBUG
				cerr << "Nuovo massimo: " << optimalSolution->getProfit() << " => " << shakedSolution.getProfit() << endl;
#endif
				optimalSolution = new Solution( shakedSolution );
			}
			
			baseSolution = shakedSolution;
			
			k = 0;
		}
		
		// Come all'inizio, se richiesto stampo su file i risultati
		if( output_file.is_open() )
		{
			printToFile( &shakedSolution );
			printToFile( &baseSolution );
		}

		k = 1 + k % K_MAX;
	}
	
#ifdef DEBUG
	cerr << "VND" << optimalSolution->toString();
#endif
	return *optimalSolution;
}

uint Solver::mutateSolution( Solution *solution, uint vehicle, int k )
{
	// Inizializzo la funzione random
	srand( (uint)time( NULL ) );

	// Barbatrucco: puntatore a funzioni per essere più efficienti nella scrittura del codice.
	// Definisco un puntatore alle funzioni usate per aprire o chiudere la soluzione corrente.
	// Questo verrà istanziato a seconda della casualità ad una delle due funzioni.
	bool (solver::Solver::* ninjaTurtle)( Solution*, uint, int );

	// Devo modificare la soluzione passatami per k volte
	// Ciclo sul numero di volte calcolato
	for( int i = 0; i < k; i++ )
	{
		// Scelgo un lato sul quale operare
		uint edge = rand() % solution->size( vehicle );

		// Casualmente scelgo se aprire o chiudere un lato
		// Non mi interesso del valore di ritorno delle funzioni usate perchè so già dove il buco è stato creato, essendo io a passarlo come parametro.
		float x = ( (float)( solution->getDemand( vehicle ) / Q + solution->getCost( vehicle ) / tMax ) / 2 );
		float p_close = atan( 50 * ( x - .75 ) ) / M_PI + .5;
		// Provo a mutare la soluzione in chiusura solo se possibile, ovvero se essa ha almeno due lati
		// oppure casualmente seguendo una funzione sigmoidale basata su una media di costo e domanda.
		if( solution->size( vehicle ) <= 2 || (float) rand() / RAND_MAX > p_close )
			ninjaTurtle = &solver::Solver::mutateSolutionOpen;
		else
			ninjaTurtle = &solver::Solver::mutateSolutionClose;

		// Effettuo un ciclo finchè non viene realmente effettuata una mutazione nella soluzione
		uint current_edge = edge;
		bool mutate;
		int exterminate = 2;
		do{
			mutate = (*this.*ninjaTurtle)( solution, vehicle, current_edge );
			// Passo al successivo lato nel caso in cui non ci sia stata una mutazione
			current_edge = ( current_edge + 1 ) % solution->size( vehicle );

			// Se ho finito un giro senza aver effettuato mutazioni nella soluzione, allora cambio funzione e ricomincio da capo.
			// Se anche l'altra funzione non ha alcun effetto, allora termino la funzione.
			if( current_edge == edge )
			{
				if( ninjaTurtle == &solver::Solver::mutateSolutionOpen )
				{
					if( solution->size( vehicle ) > 2 )
					{
						ninjaTurtle = &solver::Solver::mutateSolutionClose;
#ifdef DEBUG
						cerr << "Cambio a close" << endl;
#endif
					}
					else
					{
						exterminate = 0;
#ifdef DEBUG
						cerr << "Non cambio a close" << endl;
#endif
					}

				}
				else
				{
					ninjaTurtle = &solver::Solver::mutateSolutionOpen;
#ifdef DEBUG
					cerr << "Cambio a open" << endl;
#endif
				}

				exterminate--;
			}

		}while( exterminate > 0 && !mutate );

		// Teoricamente servirebbe qualcosa di questo tipo
		// Saremmo in una soluzione inamovibile, per cui sarebbe insensato cercare di continuare a modificarla.
		if( exterminate <= 0 )
		{
#ifdef DEBUG
			cerr << "Non ho fatto una mazza" << endl;
#endif
			return false;
		}
		

	}

	return (uint)solution->size();
}

bool Solver::mutateSolutionClose( Solution *solution, uint vehicle, int edge )
{
#ifdef DEBUG
	cerr << "Close " << edge << endl;
#endif
	// Estraggo un lato casuale dalla soluzione se non differentemente indicato
	if( edge == -1 )
		edge = (uint) rand() % ( solution->size( vehicle ) - 1 );
	else
		if( edge == solution->size( vehicle ) - 1 )
			return false;

	uint src = solution->getEdge( vehicle, edge )->getSrc();
	uint dst = solution->getEdge( vehicle, edge )->getDst();

	if( !solution->getDirection( vehicle, edge ) )
		dst ^= src ^= dst ^= src;

	// Ricavo la destinazione della destinazione dal lato scelto
	uint final_dst = solution->getEdge( vehicle, edge + 1 )->getDst( dst );
	Edge* first = graph.getEdge( src, dst );
	Edge* second = graph.getEdge( dst, final_dst );

	// Elimino i lati src->dst e dst->final_dst
	solution->removeEdge( vehicle, edge );
	solution->removeEdge( vehicle, edge );

	// Inserisco il lato che unisce src->final_dst
	if( src != final_dst )
		solution->addEdge( graph.getEdge( src, final_dst ), vehicle, edge );

	// Devo controllare la feasibility per eventuali problemi di domanda
	if( isFeasible( solution, vehicle ) )
	{
#ifdef DEBUG
		cerr << "Close completato sul lato ( " << src << ", " << dst << " ) " << endl;
#endif
		return true;
	}
	else
	{
		solution->removeEdge( vehicle, edge );
		solution->addEdge( second, vehicle, edge );
		solution->addEdge( first, vehicle, edge );
	}

	return false;
}

bool Solver::mutateSolutionOpen( Solution *solution, uint vehicle, int edge )
{
#ifdef DEBUG
	cerr << "Open " << edge << endl;
#endif
	// Estraggo un lato casuale dalla soluzione se non già indicato
	if( edge == -1 )
		edge = (uint) rand() % solution->size( vehicle );

	uint src = solution->getEdge( vehicle, edge )->getSrc();
	uint dst = solution->getEdge( vehicle, edge )->getDst();

	if( !solution->getDirection( vehicle, edge ) )
		dst ^= src ^= dst ^= src;

	// Lista di adiacenza del nodo sorgente
	vector<Edge*> adj = graph.getAdjList( src );
	// Prendo i nodi adiacenti al lato estratto casualmente
	bool* edgeTested = (bool*)calloc( adj.size(), sizeof( bool ) );
	int testables = (int)adj.size();
	// Dalla lista elimino il nodo dst
	for( int i = 0; i < adj.size(); i++ )
	{
		if( adj[ i ]->getDst( src ) == dst )
		{
			edgeTested[ i ] = 1;
			testables--;
			break;
		}
	}

	while ( testables > 0 )
	{
		uint closer;
		// Cerco un nodo non ancora testato
		while( edgeTested[ closer = (uint) rand() % adj.size() ] );
		edgeTested[ closer ] = true;
		testables--;

		// Prendo il nodo di chiusura corrispondente a closer
		closer = adj[ closer ]->getDst( src );

#ifdef DEBUG
		cerr << "Test: " << testables;
		cerr << ". Ho scelto te: " << closer;
		cerr << " per aprire ( " << src << ", " << dst << " ) " << endl;
#endif
		// Salvo il lato nel caso in cui lo debba reinserire
		Edge* removed = graph.getEdge( src, dst );
		
		// Rimuovo il lato dalla soluzione
		solution->removeEdge( vehicle, edge );
		

		// Chiudo la soluzione e verifico se la soluzione è feasible
		// Inserimento al "contrario" perché l'inserimento viene effettuato prima dell'indice del lato scelto.
		solution->addEdge( graph.getEdge( closer, dst ), vehicle, edge );
		solution->addEdge( graph.getEdge( src, closer ), vehicle, edge );

		// Controllo la feasibilità della nuova soluzione
		if( isFeasible( solution, vehicle ) )
		{
#ifdef DEBUG
			cerr << "Open completato con nodo " << closer << endl;
#endif
			free( edgeTested );
			return true;
		}
		else
		{
			// Ripristino la soluzione iniziale
			solution->removeEdge( vehicle, edge );
			solution->removeEdge( vehicle, edge );
			solution->addEdge( removed, vehicle, edge );
		}
	}

	free( edgeTested );
	return false;
}

uint Solver::openSolutionRandom( Solution *solution, uint vehicle, int k, uint* src, uint* dst )
{
	uint edge = (uint)( rand() % solution->size( vehicle ) );
	*src = solution->getEdge( vehicle, edge )->getSrc();
	*dst = solution->getEdge( vehicle, edge )->getDst();

	// Barbascambio di variabili a seconda del verso in cui tale lato viene percorso dal veicolo
	if ( !solution->getDirection( vehicle, edge ) )
		*dst ^= *src ^= *dst ^= *src;

	// Rimuovo k+1 lati
	// Itero sul minimo valore tra k e la lunghezza attuale della soluzione (k+1<s => k<s-1!!!)
	//  così da non togliere più lati di quanti la soluzione non ne abbia
	int ktemp = ( k < solution->size( vehicle ) - 1 ?
				 k : (uint)solution->size( vehicle ) - 1 );
	// Rimuovo 1 lato
	solution->removeEdge( vehicle, edge );

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
		if( edge >= solution->size( vehicle ) )
		{
			edge = (uint)( solution->size( vehicle ) - 1 );
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
			*src = solution->getEdge( vehicle, edge )->getDst( *src );
		else
			*dst = solution->getEdge( vehicle, edge )->getDst( *dst );
		
		// Rimuovo il lato scelto dalla soluzione
		solution->removeEdge( vehicle, edge );
	}

	return edge;
}

bool Solver::closeSolutionRandom( Solution* solution, int vehicle, uint src, uint dst, int k, int edgeIndex )
{
	srand( (uint)time( NULL ) );
	/*
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

		if( !isFeasible( solution, vehicle ) )
		{
			solution->removeEdge( vehicle, edgeIndex );
			return false;
		}

		return true;
	}

	// Inizio con la chiusura della soluzione, partendo dal nodo sorgente, ovvero dove ha inizio il buco
	vector <Edge*> edges = graph.getAdjList( src );
	bool* tried = (bool*)calloc( edges.size(), sizeof( bool ) );
	
	// TODO: decidere se 1 o proporzionale o tutto o cosa.
	uint tries = ceil( (float)edges.size() / 10 );//(uint)edges.size();
	
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
		if( !isFeasible( solution, vehicle ) ||
		    !closeSolutionRandom( solution, vehicle, victim->getDst( src ), dst, k - 1, edgeIndex + 1 ) )
		{
#ifdef DEBUG
			cerr << "Unfeasible: k = " << k << " tries = " << tries << " => " << solution->toString();
#endif
			solution->removeEdge( vehicle, edgeIndex );
			continue; // return false ?
		}
		else
		{
			free( tried );
			return true;
		}
	}
	
	// Tento (con probabilità) un'ultima chiusura secca se tutte le precedenti sono andate male.
	if ( ( (float)rand() / RAND_MAX ) <= P_CLOSE )
	{
#ifdef DEBUG
		cerr << "Lancio una moneta. " << endl;
#endif
		if ( src == dst )
		{
#ifdef DEBUG
			cerr << "Non dovevo fare niente. " << endl;
#endif
			free( tried );
			return true;
		}
		
		solution->addEdge( graph.getEdge( src, dst ), vehicle, edgeIndex );
		
		if( isFeasible( solution, vehicle ) )
		{
#ifdef DEBUG
			cerr << "Ce l'ho fatta. " << endl;
#endif
			free( tried );
			return true;
		}
		
		solution->removeEdge( vehicle, edgeIndex );
	}
	
#ifdef DEBUG
	cerr << "Mi arrendo. " << endl;
#endif

	free( tried );
	return false;
*/
	
	/**
	 * Versione tonta di Bellman-Ford
	 * 	Cerco tutti i percorsi di lunghezza al più k che conducano a dst:
	 *		Se sorgente e destinazione coincidono, aggiungo la soluzione vuota alle soluzioni.
	 *		Ogni volta che trovo un percorso feasible, lo aggiungo a quelli da estendere.
	 *		Se il percorso termina su dst, lo aggiungo al vettore delle soluzioni.
	 */
	vector< vector< list<Edge*> > > paths = vector< vector< list<Edge*> > >( graph.size() );
	vector< list<Edge*> > sol = vector< list<Edge*> >();
	uint pathsFound = 0;

	// Inizializzo i percorsi:
	//	Parto dal nodo sorgente e guardo tutte le adiacenze feasible.
	vector<Edge*> edges = graph.getAdjList( src );
	for ( Edge* edge : edges )
	{
		// "Peso" il lato nel caso in cui questo venga inserito nella soluzione
		list<Edge*> initPath;
		initPath.push_back( edge );
		
		solution->addEdge( edge, vehicle, edgeIndex );

		if ( isFeasible( solution, vehicle ) )
		{
			pathsFound++;
			paths[ edge->getDst( src ) ].push_back( initPath );

			if ( edge->getDst( src ) == dst )
				sol.push_back( initPath );
		}

		solution->removeEdge( vehicle, edgeIndex );
	}
	// Soluzione vuota (~autopercorso gratutio)
	if ( src == dst )
		sol.push_back( list<Edge*>() );
	
#ifdef DEBUG
	cerr << "BOZO Inizializzazione: " << endl;
	for ( int i = 0; i < graph.size(); i++ )
	{
		cerr << i << "] ";
		if ( !paths[ i ].empty() )
		{
			for ( auto edge : paths[ i ].front() )
				cerr << "( " << edge->getSrc() << ", " << edge->getDst() << " ) " << endl;
		}
		else
			cerr << "vuoto" << endl;
	}
#endif
	
	// Lunghezza massima: k => itero ancora k - 1 volte ( la prima è sopra )
	for ( int i = 0; i < k - 1; i++ )
	{
		// Ad ogni iterazione analizzo tutti i percorsi trovati, suddivisi per nodo finale
		for ( int attuale = 0; attuale < graph.size(); attuale++ )
		{
#ifdef DEBUG
			cerr << attuale << ", percorsi: " << pathsFound << endl;
#endif
			// Per ogni nodo finale, analizzo tutti i percorsi trovati dalla precedente iterazione
			while ( paths[ attuale ].size() )
			{
				list<Edge*> actSol = paths[ attuale ].back();
				
				edges = graph.getAdjList( attuale );
				for ( Edge* edge : edges )
				{
//					// Evito di iterare sullo stesso lato
//					uint previous = src;
//					bool inSolution = false;
//					for ( auto it = actSol.begin(); it != actSol.end(); ++it )
//					{
//						if ( ( attuale == previous && edge->getDst( attuale ) == (*it)->getDst( previous ) ) ||
//							 ( attuale == (*it)->getDst( previous ) && edge->getDst( attuale ) == previous ) )
//						{
//							inSolution = true;
//							break;
//						}
//						
//						previous = edge->getDst( previous );
//					}
//					if ( inSolution )
//						continue;
				 
					list<Edge*> newSol( actSol );
					newSol.push_back( edge );

					for ( auto it = newSol.rbegin(); it != newSol.rend(); ++it )
						solution->addEdge( *it, vehicle, edgeIndex );
					
					if ( isFeasible( solution, vehicle ) )
					{
						pathsFound++;
						paths[ edge->getDst( attuale ) ].push_back( newSol );

						if ( edge->getDst( src ) == dst )
							sol.push_back( newSol );
						
						// Faccio un'estrazione ogni tot percorsi analizzati
						if ( pathsFound == 2000 )
						{
							pathsFound = 0;
							if ( (float)rand() / RAND_MAX < P_CLOSE )
							{
								list<Edge*> closure = sol[ rand() % sol.size() ];
#ifdef DEBUG
								cerr << endl << "BOZO Fine prematura: " << endl;
								for ( auto edge : sol.back() )
									cerr << "( " << edge->getSrc() << ", " << edge->getDst() << " ) ";
								cerr << endl;
#endif
								for ( auto it = closure.rbegin(); it != closure.rend(); ++it )
									solution->addEdge( *it, vehicle, edgeIndex );
								
								return true;
							}
						}
					}

					for ( int i = 0; i < newSol.size(); i++ )
						solution->removeEdge( vehicle, edgeIndex );
				}

				// Rimuovo il percorso analizzato
				paths[ attuale ].pop_back();
			}
		}
#ifdef DEBUG
		cerr << "Trovati " << sol.size() << " percorsi." << endl;
#endif
	}
	
#ifdef DEBUG
	cerr << endl << "BOZO Fine: " << endl;
#endif

	// Ritorno un percorso a caso
	if ( sol.size() )
	{
		list<Edge*> closure = sol[ rand() % sol.size() ];
#ifdef DEBUG
		for ( auto edge : sol.back() )
			cerr << "( " << edge->getSrc() << ", " << edge->getDst() << " ) ";
		cerr << endl;
#endif
		for ( auto it = closure.rbegin(); it != closure.rend(); ++it )
			solution->addEdge( *it, vehicle, edgeIndex );

		return true;
	}
	else
		return false;
}

list<Edge*> Solver::closeSolutionDijkstra( Solution solution, int vehicle, uint src, uint dst, int edgeIndex )
{
	/**
	 * Basato sull'algoritmo di Bellman-Ford,
	 *  Per ogni lato tiene una pila delle liste dei lati per cui è passato.
	 *  Ad ogni miglioria feasible, la soluzione viene posta in cima alla pila ed
	 *   è quella che verrà utilizzata per massimizzare i percorsi successivi.
	 *  Se durante una massimizzazione viene rifiutata una soluzione per infeasiblità,
	 *   viene rieseguita tutta la massimizzazione con la soluzione successiva nella pila.
	 * L'implementazione iniziale evita cicli (ad occhio l'algoritmo così cercherebbe
	 *  il ciclo a profitto massimo, risolvendo all'ottimo il problema => tempo esponenziale).
	 */
	
	vector< list< list<Edge*> > > sol = vector< list< list<Edge*> > >( graph.size() );
	vector< list< int* > > val = vector< list< int* > >( graph.size() );	// P, T, D
	
	vector<Edge*> edges = graph.getAdjList( src );
	for ( Edge* edge : edges )
	{
		// "Peso" il lato nel caso in cui questo venga inserito nella soluzione
		int* initVal = (int*)malloc( 3 * sizeof( int ) );
		list<Edge*> initSol;
		initSol.push_back( edge );
		
		solution.addEdge( edge, vehicle, edgeIndex );
		if ( !isFeasible( &solution, vehicle ) )
		{
			solution.removeEdge( vehicle, edgeIndex );
			free( initVal );
			continue;
		}

		initVal[ 0 ] = solution.getProfit( vehicle );
		initVal[ 1 ] = solution.getCost( vehicle );
		initVal[ 2 ] = solution.getDemand( vehicle );
		
		solution.removeEdge( vehicle, edgeIndex );
		initVal[ 0 ] -= solution.getProfit( vehicle );
		initVal[ 1 ] -= solution.getCost( vehicle );
		initVal[ 2 ] -= solution.getDemand( vehicle );
		
		sol[ edge->getDst( src ) ].push_front( initSol );
		val[ edge->getDst( src ) ].push_front( initVal );
	}
	
#ifdef DEBUG
	cerr << "Inizializzazione: " << endl;
	for ( int i = 0; i < graph.size(); i++ )
	{
		if ( !sol[ i ].empty() )
		{
			cerr << i << "] ";
			for ( auto edge : sol[ i ].front() )
				cerr << "( " << edge->getSrc() << ", " << edge->getDst() << " ) ";
			int* valori = val[ i ].front();
			cerr <<  " " << valori[ 0 ] << " " << valori[ 1 ] << " " << valori[ 2 ] << endl;
		}
		else
			cerr << i << "] vuoto" << endl;
	}
#endif
	
	bool improved = true;
	while ( improved )
	{
		improved = false;
		for ( int attuale = 0; attuale < graph.size(); attuale++ )
		{
//			if ( sol[ attuale ].empty() )
//				continue;
			
			bool unfeasible = true;
			for ( auto actSol = sol[ attuale ].begin();
				  unfeasible && actSol != sol[ attuale ].end();
				  ++actSol )
			{
				unfeasible = false;
				edges = graph.getAdjList( attuale );
				for ( Edge* edge : edges )
				{
//					// Teoricamente dovrei iterare solo sui NODI non ancora in soluzione..
//					// ( Esclusa magari la destinazione )
//					uint previous = src;
//					bool inSolution = false;
//					for ( auto it = (*actSol).begin(); it != (*actSol).end(); ++it )
//					{
//						if ( dst != (*it)->getSrc() && dst != (*it)->getDst() &&
//							!~	( edge->getDst( attuale ) == (*it)->getSrc() ||
//							~!	  edge->getDst( attuale ) == (*it)->getDst() ) )
//						{
//							inSolution = true;
//							break;
//						{
//						previous = edge->getDst( previous );
//					}
//					if ( inSolution )
//						continue;
					
					list<Edge*> newSol( *actSol );
					newSol.push_back( edge );
					
					for ( auto it = newSol.rbegin(); it != newSol.rend(); ++it )
						solution.addEdge( *it, vehicle, edgeIndex );
					
					if ( !isFeasible( &solution, vehicle ) )
					{
#ifdef DEBUG
						//cerr << "U";
#endif
						unfeasible = true;
						for ( int i = 0; i < newSol.size(); i++ )
							solution.removeEdge( vehicle, edgeIndex );
						continue;
					}
					
					int* newVal = (int*)malloc( 3 * sizeof( int ) );
					newVal[ 0 ] = solution.getProfit( vehicle );
					newVal[ 1 ] = solution.getCost( vehicle );
					newVal[ 2 ] = solution.getDemand( vehicle );
					
					for ( int i = 0; i < newSol.size(); i++ )
						solution.removeEdge( vehicle, edgeIndex );

					newVal[ 0 ] -= solution.getProfit( vehicle );
					newVal[ 1 ] -= solution.getCost( vehicle );
					newVal[ 2 ] -= solution.getDemand( vehicle );
					
					if ( val[ edge->getDst( attuale ) ].empty() ||
						newVal[ 0 ] > val[ edge->getDst( attuale ) ].front()[ 0 ] ||
						( newVal[ 0 ] == val[ edge->getDst( attuale ) ].front()[ 0 ] &&
						 ( ( newVal[ 1 ] <= val[ edge->getDst( attuale ) ].front()[ 1 ] &&
							 newVal[ 2 ] <  val[ edge->getDst( attuale ) ].front()[ 2 ] ) ||
						   ( newVal[ 1 ] <  val[ edge->getDst( attuale ) ].front()[ 1 ] &&
						 	 newVal[ 2 ] <= val[ edge->getDst( attuale ) ].front()[ 2 ] ) ) ) )
					{
#ifdef DEBUG
						if ( !val[ edge->getDst( attuale ) ].empty() )
							cerr << "M";
						else
							cerr << "C";
#endif
						sol[ edge->getDst( attuale ) ].push_front( newSol );
						val[ edge->getDst( attuale ) ].push_front( newVal );
						
						improved = true;
					}
					else
						free( newVal );
				}
			}
		}
	}
	
#ifdef DEBUG
	cerr << endl << "Fine: " << endl;
	for ( int i = 0; i < graph.size(); i++ )
	{
		cerr << i << "] ";
		if ( !sol[ i ].empty() )
		{
			for ( auto edge : sol[ i ].front() )
				cerr << "( " << edge->getSrc() << ", " << edge->getDst() << " ) ";
			cerr << endl;
			int* valori = val[ i ].front();
			cerr << ": " << valori[ 0 ] << " " << valori[ 1 ] << " " << valori[ 2 ] << endl;
		}
		else
			cerr << "vuoto: " << endl;
	}
#endif
	
	// Teoricamente non dovrei mai entrare qui.
	if ( sol[ dst ].empty() )
	{
#ifdef DEBUG
		cerr << "Ho fallito." << endl;
#endif
		return list<Edge*>();
	}
	
#ifdef DEBUG
	cerr << "Chiudo " << val[ dst ].front()[ 0 ] << endl;
#endif
	return sol[ dst ].front();
}

solver::Solution Solver::solve( string method, int repetition )
{
	// Numero di iterazioni
	// VNASD
	if( output_file.is_open() )
	{
		if( repetition != -1 )
			output_file << method << " " << repetition << " " << M << endl;
		else
			output_file << method << " " << M << endl;
	}

	// A seconda del metodo richiesto, calcolo la soluzione in modi diversi.
	if( !method.compare( "VNS" ) )
		currentSolution = vns( N_ITER, currentSolution );
	else
	{
		if( !method.compare( "VND" ) )
			currentSolution = vnd( N_ITER, currentSolution );
		else
		{
			if( !method.compare( "VNASD" ) )
				currentSolution = vnasd( N_ITER, currentSolution, repetition );
		}
	}

#ifdef DEBUG
	cerr << "Solve" << currentSolution.toString();
#endif
	return currentSolution;
}

bool Solver::isFeasible( const Solution* solution, int vehicle ) const
{
	return	solution->getDemand( vehicle ) < Q &&
			solution->getCost( vehicle ) < tMax;
}

bool Solver::setOutputFile( string filename )
{
	// Apro il file
	output_file.open( OUTPUT_FILE_DIR + filename + OUTPUT_FILE_EXTENSION, std::ofstream::out );
	return output_file.is_open();
}

void Solver::printToFile( Solution* solution )
{
	// Stampo le informazione sulla solution
	output_file << solution->getProfit() << " ( ";
	for ( int i = 0; i < M; i++ )
		output_file << solution->getProfit( i ) << " ";
	output_file << ") ";
	
	output_file << solution->getCost() << " ( ";
	for ( int i = 0; i < M; i++ )
		output_file << solution->getCost( i ) << " ";
	output_file << ") ";
	
	output_file << solution->getDemand() << " ( ";
	for ( int i = 0; i < M; i++ )
		output_file << solution->getDemand( i ) << " ";
	output_file << ")" << endl;

	return;
}

int Solver::mrBeanBeanBinPacking( Solution* solution, uint vehicle )
{
	// Il metodo cerca di ottimizzare la domanda del veicolo passato come parametro,
	// cercando di spostare la domanda su altri veicoli su lati comuni.

#ifdef DEBUG
	cerr << "MrBean" << endl;
	cerr << "Soluzione: " << solution->toString() << endl;
	cerr << "Soluzione veicolo: " << solution->toString( vehicle ) << endl;
#endif

	Vehicle* optimizationVehicle = solution->getVehicle( vehicle );

	set<MetaEdge*> served;
	for ( int i = 0; i < solution->size( vehicle ); i++ )
	{
		MetaEdge* edge = solution->getEdge( vehicle, i );

		if ( edge->isServer( optimizationVehicle ) )
			served.insert( edge );
	}


	int swaps = 0;

	// Ciclo sull'intera soluzione del veicolo
	for ( auto it = served.begin(); it != served.end(); ++it )
	{
#ifdef DEBUG
		cerr << "Lo sto servendo io: " << vehicle << endl;
#endif
		// Prendo i veicoli che passano dal lato
		vector<const Vehicle*> takers = (*it)->getTakers();

#ifdef DEBUG
		cerr << "Altri " << takers.size() - 1 << " veicoli passano da questo lato";
		cerr << " ( " << (*it)->getSrc() << ", ";
		cerr << (*it)->getDst() << " )" << endl;
#endif
		// Ciclo sui takers per cercare di attribuire la domanda ad un altro veicolo
		for( int j = 0; j < takers.size(); j++ )
		{
			if( takers[ j ] != solution->getVehicle( vehicle ) )
			{
#ifdef DEBUG
				cerr << "Provo a mettere " << takers[ j ]->getId() << " come server" << endl;
#endif
				// Unsetto il veicolo da ottimizzare come taker
				(*it)->setServer( takers[ j ] );
				// Controllo se questo spostamento lascia la soluzione feasible
				if( isFeasible( solution, solution->getVehicleIndex( takers[ j ] ) ) )
				{
					swaps++;
#ifdef DEBUG
					cerr << "Scambio fatto con successo!" << endl;
#endif
					break;
				}
				else
				{
					(*it)->setServer( optimizationVehicle );
#ifdef DEBUG
					cerr << "Non ha funzionato, ripristino" << endl;
#endif
				}

			}
		}
	}

#ifdef DEBUG
	cerr << "Swaps, lati ottimizzati: " << swaps << endl;
#endif

	return swaps;
}
