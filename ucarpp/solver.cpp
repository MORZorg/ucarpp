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
#endif
	Solution baseSolution( M, graph );
	
//	// Creazione sequenziale
//	for ( int i = 0; i < M; i++ )
//	{
//#ifdef DEBUG
//		cerr << "\tVeicolo " << i + 1 << endl;
//#endif
//		createBaseSolution( &baseSolution, i );
//	}
	
	// Creazione parallela
	int toBeFilled = M;
	bool* filled = (bool*)calloc( M, sizeof( bool ) );
	int* last = (int*)calloc( M, sizeof( int ) );
	for ( int i = 0; i < M; i++ )
		last[ i ] = depot;
	
	while ( ( toBeFilled -= extendBaseSolution( &baseSolution, M, filled, last ) ) > 0 );
	
#ifdef DEBUG
	cerr << "Soluzione iniziale:\n" << baseSolution.toString() << endl;
#endif
	
	free( filled );
	free( last );
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
		sort( edges.begin(), edges.end(), baseSolution->compareGreedy );
		
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

/**
 * Funzione per creare una soluzione di base in parallelo su ogni veicolo.
 *
 * @param baseSolution	La soluzione da estendere.
 * @param M				Il numero di veicoli.
 * @param filled		Vettore indicante quali veicoli sono già stati riempiti.
 * @param last			Vettore indicante il nodo finale di ogni veicolo.
 * @return				Numero di veicoli riempiti durante l'iterazione.
 */
int Solver::extendBaseSolution( Solution* baseSolution, int M, bool* filled, int* last )
{
	vector<Edge*> edges;
	int filledCount = 0;
	for ( int i = 0; i < M; i++ )
	{
		// Ho già riempito il veicolo. Passo al successivo
		if ( filled[ i ] )
			continue;
		
		// Ordino i lati uscenti dal nodo corrente
		edges = graph.getAdjList( last[ i ] );
		sort( edges.begin(), edges.end(), baseSolution->compareGreedy );
		
		// Prendo il lato ammissibile migliore, se esiste
		filled[ i ] = true;
		for( Edge* edge : edges )
		{
			last[ i ] = edge->getDst( last[ i ] );
			
			// Aggiungo il lato selezionato ed il lato necessario alla chiusura.
			bool addedEdge = last[ i ] != depot;
			baseSolution->addEdge( edge, i );
			if ( addedEdge )
			{
				Edge* returnEdge = graph.getEdge( last[ i ], depot );
				baseSolution->addEdge( returnEdge, i );
			}
			
#ifdef DEBUG
			cerr << "After added: " << baseSolution->toString( i ) << endl;
#endif
			
			/**
			 * Se la soluzione resta feasible avendo preso
			 * il lato selezionato ed il lato di ritorno,
			 * accetto il nuovo lato.
			 */
			if ( isFeasible( baseSolution, i ) )
			{
#ifdef DEBUG
				fprintf( stderr, "\t\tPreso %d (r: % 3.2f)\n\n",
						last[ i ] + 1, edge->getProfitDemandRatio() );
#endif
				
				if ( addedEdge )
					baseSolution->removeEdge( i );
				
				filled[ i ] = false;
				break;
			}
			else
			{
				// Annullo la mossa
				last[ i ] = edge->getDst( last[ i ] );
				
				baseSolution->removeEdge( i );
				if ( addedEdge )
					baseSolution->removeEdge( i );
			}
		}
		
		if ( filled[ i ] )
		{
			if ( last[ i ] != depot )
			{
				Edge* returnEdge = graph.getEdge( last[ i ], depot );
				baseSolution->addEdge( returnEdge, i );
			}
			filledCount++;
		}
	}

	return filledCount;
}

Solution Solver::vnasd( int nIter, Solution baseSolution, int repetition )
{
	float iterations = nIter / ( 2 * repetition );

	for( int i = 0; i < repetition; i++ )
	{
		baseSolution = vns( ceil( iterations ), baseSolution );
		baseSolution = vnd( floor( iterations ), baseSolution );

		for ( int i = 0; i < M; i++ )
			if ( !isFeasible( &baseSolution, i ) )
				throw 2;
	}

	return baseSolution;
}

Solution Solver::vnaasd( int nIter, Solution baseSolution, int repetition )
{
	float iterations = nIter / ( 4 * repetition );

	for( int i = 0; i < repetition; i++ )
	{
		baseSolution = vns( floor( 3 * iterations ), baseSolution );

		baseSolution = vnd( ceil( iterations ), baseSolution );

		for ( int i = 0; i < M; i++ )
			if ( !isFeasible( &baseSolution, i ) )
				throw 2;
	}

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

#ifdef DEBUG
		cerr << "shakedSolution = Solution( baseSolution ): " << endl << "Base: " << baseSolution.toString() << "Shaked: " << shakedSolution.toString() << endl;
#endif

		
		/*** Shaking ***/
		// Estraggo un veicolo ed un lato iniziale casuali
		// Tengo traccia anche dei nodi sorgente e destinazione di tale lato
		uint vehicle = rand() % M;

#ifdef DEBUG
		cerr << "Iterazione " << nIter << " sul veicolo " << vehicle << endl;
#endif

		// MrBean colpisce ancora
		// Utile per essere sicuri di fare tutte le ottimizzazioni possibili
		mrBeanBeanBinPacking( &shakedSolution, vehicle );

#ifdef DEBUG
		cerr << "infra shake " << shakedSolution.toString();
#endif

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
		Solution localSearchSolution( shakedSolution );

		for ( int v = 0; v < M; v++ )
		{
			// Cerco di ottimizzare il veicolo appena shakerato
			localSearchSolution = Solution( shakedSolution );
			mrBeanBeanBinPacking( &localSearchSolution, v );
			cleanVehicle( &localSearchSolution, v );

#ifdef DEBUG
			cerr << "Ricerco localmente su " << v << ": " << localSearchSolution.toString( v );
#endif

			uint previous = depot;
			uint next;
			for ( int i = 0; i < localSearchSolution.size( v ); i++ )
			{
#ifdef DEBUG
				cerr << "Lavoro sul veicolo " << v << " lato " << i;
				cerr << " ( " << localSearchSolution.getEdge( v, i )->getSrc() << " " << localSearchSolution.getEdge( v, i )->getDst() << " ) " << endl;
				cerr << "Parto da: " << localSearchSolution.toString();
#endif
				// Elimino almeno un lato
				list <Edge*> removedEdges;
				bool wasServer;

				Vehicle* tempVehicle = localSearchSolution.getVehicle( v );
				
				MetaEdge* tempMeta = localSearchSolution.getEdge( v, i );
				wasServer = tempMeta->isServer( tempVehicle );
				// Pro thinking:
				// Se MrBean non è riuscito a riassegnare questo lato ad altri veicoli ed io non sono l'unico che lo attraversa,
				// è inutile cercare di toglierlo dalla soluzione in quanto renderebbe infeasible un altro veicolo, per cui salto.
				if( !isRemovable( &localSearchSolution, v, i ) )
				{
#ifdef DEBUG
					cerr << "Il lato " << i << " non è rimovibile. Passo al lato successivo." << endl;
#endif
					previous = tempMeta->getDst( previous );
					continue;
				}

				removedEdges.push_back( tempMeta->getEdge() );

				localSearchSolution.removeEdge( v, i );
				next = tempMeta->getDst( previous );

				// Elimino lati dalla soluzione fintanto che questi non ne aumentano il profitto e fintanto che sono presenti nella soluzione
				while( i < localSearchSolution.size( v ) && isRemovable( &localSearchSolution, v, i ) )
				{
					// Calcolo la differenza di profitto che abbiamo nel togliere un lato alla soluzione
					int diffProfit = localSearchSolution.getProfit( v );

					// Rimuovo il lato i
					MetaEdge* temp = localSearchSolution.getEdge( v, i );
					localSearchSolution.removeEdge( v, i );

					diffProfit -= localSearchSolution.getProfit( v );

					// Se non ho differenze di profitto, tolgo quel lato dalla soluzione
					if( diffProfit == 0 )
					{
						// Sposto il nodo di partenza
						next = temp->getDst( next );
						// Inserisco il lato tolto nella lista
						removedEdges.push_back( temp->getEdge() );
					}
					else
					{
						// Altrimenti lo riaggiungo
						localSearchSolution.addEdge( temp->getEdge(), v, i );
						//temp->setServer( tempVehicle );
						break;
					}

				}

#ifdef DEBUG
				cerr << "Creato un buco di " << removedEdges.size() << " lati su ( " << previous << " " << next << " ) " << endl;
				cerr << "Cristo: " << localSearchSolution.toString() << endl;
#endif

				// Chiedo a Dijkstra di calcolarmi la chiusura migliore
				list<Edge*> closure = closeSolutionDijkstra( localSearchSolution, v, previous, next, i );
				previous = next;

				if ( !closure.size() )
				{
					// Quell'incapace del Sig. Bellman-Ford-Zucchelli ha fallito: ripristino.
					for( auto it = removedEdges.rbegin(); it != removedEdges.rend(); ++it )
						localSearchSolution.addEdge( *it, v, i );
	
					if( wasServer )
					{
#ifdef DEBUG
						cerr << "PIPPIRIPACCHIO1 questa variabile è utile" << endl;
#endif
						localSearchSolution.getEdge( v, i )->setServer( tempVehicle );
					}

					i += removedEdges.size() - 1;
					continue;
				}

				// Se questo porta un miglioramento, effettuo la chiusura, altrimenti riaggiungo il lato i
				for ( auto it = closure.rbegin(); it != closure.rend(); ++it )
					localSearchSolution.addEdge( *it, v, i );

#ifdef DEBUG
				cerr << "Soluzioni dopo ricerca locale: " << localSearchSolution.toString() << endl;
#endif

				// Controllo se ho trovato una soluzione migliore della massima trovata in precedenza
				if ( localSearchSolution > maxSolution )
				{
#ifdef DEBUG
					cerr << "Migliorato" << endl;
#endif
					maxSolution = Solution( localSearchSolution );
					// SFANCULATI break; // SFANCULATI BEST
				}

				// Resetto la shakedSolution per effettuare una nuova ricerca
				for( int j = 0; j < closure.size(); j++ )
					localSearchSolution.removeEdge( v, i );

				for( auto it = removedEdges.rbegin(); it != removedEdges.rend(); ++it )
					localSearchSolution.addEdge( *it, v, i );

//				if( wasServer )
//				{
//#ifdef DEBUG
//					cerr << "PIPPIRIPACCHIO2 questa variabile è utile" << endl;
//#endif
//					localSearchSolution.getEdge( v, i )->setServer( tempVehicle );
//				}

#ifdef DEBUG
				cerr << "Fine ciclo: ripristinata la shakedSolution iniziale?" << endl;
				cerr << "Shaked: " << localSearchSolution.toString() << endl;
#endif

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

		for ( int i = 0; i < M; i++ )
			if ( !isFeasible( &shakedSolution, i ) )
				throw 2;

		// Incremento il numero di iterazioni svolte
		k = 1 + k % K_MAX;
	}
	
	// Ottimizzazione finale
	optimizeSolution( optimalSolution );
	
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
			 src,
			 dst;
		int	 edge = -1;

		// Prima di generare un buco nella soluzione, la muto, così da partire da
		// una soluzione diversa ad ogni ciclo, e non sempre dalla stessa.
		mutateSolution( &shakedSolution, vehicle, ceil( XI * k ) );

		// Verifico che il buco sia stato creato correttamente
		try
		{
			edge = openSolutionRandom( &shakedSolution, vehicle, k, &src, &dst );
			if ( edge < 0 || edge >= shakedSolution.size() )
				throw 10;
		}
		catch( int e )
		{
#ifdef DEBUG
			cerr << "Open fallita " << e << endl;
#endif
			continue;
		}
		
#ifdef DEBUG
		cerr << shakedSolution.toString();
		cerr << "Buco " << src << " " << dst << "\tindice " << edge << endl;
#endif
		
		// Cerco di ottimizzare il veicolo appena shakerato
		mrBeanBeanBinPacking( &shakedSolution, vehicle );

		list<Edge*> closure = closeSolutionDijkstra( shakedSolution, vehicle, src, dst, edge );
		if ( !closure.size() )
		{
#ifdef DEBUG
			cerr << "Pippero." << endl;
#endif
			continue;
		}

		for ( auto it = closure.rbegin(); it != closure.rend(); ++it )
			shakedSolution.addEdge( *it, vehicle, edge );

/*
		// Ottimizzo la soluzione
		optimizeSolution( &shakedSolution );
*/
		
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

		for ( int i = 0; i < M; i++ )
			if ( !isFeasible( &shakedSolution, i ) )
				throw 2;

		k = 1 + k % K_MAX;
	}
	
	// Ottimizzazione finale
	optimizeSolution( optimalSolution );
	
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
		bool mutate = false;
		int exterminate = 2;
		do{
			// Effettuo una mutazione solo se non rischio di causare infeasibilità ad altri veicoli
			if( isRemovable( solution, vehicle, current_edge ) )
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
	// Estraggo un lato casuale dalla soluzione se non differentemente indicato
	if( edge == -1 )
		edge = (uint) rand() % ( solution->size( vehicle ) - 1 );
	else
		if( edge == solution->size( vehicle ) - 1 )
			return false;

	// Se uno dei due lati della chiusura non è rimovibile, non posso agire
	if ( !isRemovable( solution, vehicle, edge ) || !isRemovable( solution, vehicle, edge + 1 ) )
		return false;

	uint src = solution->getEdge( vehicle, edge )->getSrc();
	uint dst = solution->getEdge( vehicle, edge )->getDst();

	if( !solution->getDirection( vehicle, edge ) )
		dst ^= src ^= dst ^= src;

#ifdef DEBUG
	cerr << "Close " << edge << ": ( " << src << ", " << dst << " ) " << endl;
#endif

	// Ricavo la destinazione della destinazione dal lato scelto
	uint final_dst = solution->getEdge( vehicle, edge + 1 )->getDst( dst );
	Edge* first = graph.getEdge( src, dst );
	Edge* second = graph.getEdge( dst, final_dst );

	// Elimino i lati src->dst e dst->final_dst
	solution->removeEdge( vehicle, edge );
	solution->removeEdge( vehicle, edge );

	// Inserisco il lato che unisce src->final_dst
	bool autoCiclo = src == final_dst;
	if( !autoCiclo )
		solution->addEdge( graph.getEdge( src, final_dst ), vehicle, edge );

	// Devo controllare la feasibility per eventuali problemi di domanda
	if( isFeasible( solution, vehicle ) )
	{
#ifdef DEBUG
		cerr << "Close completato sul lato ( " << src << ", " << dst << " ) ";
		cerr << "chiuso con ( " << src << " " << final_dst << " ) escludendo il nodo " << dst << endl;
#endif
		return true;
	}
	else
	{
		if ( !autoCiclo )
			solution->removeEdge( vehicle, edge );

		solution->addEdge( second, vehicle, edge );
		solution->addEdge( first, vehicle, edge );

	}

	return false;
}

bool Solver::mutateSolutionOpen( Solution *solution, uint vehicle, int edge )
{
	// Estraggo un lato casuale dalla soluzione se non già indicato
	if( edge == -1 )
		edge = (uint) rand() % solution->size( vehicle );

	// Se il lato non è rimovibile, non posso agire
	if ( !isRemovable( solution, vehicle, edge ) )
		return false;

	uint src = solution->getEdge( vehicle, edge )->getSrc();
	uint dst = solution->getEdge( vehicle, edge )->getDst();

	if( !solution->getDirection( vehicle, edge ) )
		dst ^= src ^= dst ^= src;

#ifdef DEBUG
	cerr << "Open " << edge << ": ( " << src << ", " << dst << " ) " << endl;
#endif

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

/*
#ifdef DEBUG
		cerr << "Test: " << testables;
		cerr << ". Ho scelto te: " << closer;
		cerr << " per aprire ( " << src << ", " << dst << " ) " << endl;
#endif
*/
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

int Solver::openSolutionRandom( Solution *solution, uint vehicle, int k, uint* src, uint* dst )
{
	// Prendo un lato escludendo il primo e l'ultimo della soluzione, così da non escludere il deposito dalla soluzione
	//// Prima verifico che la soluzione abbia almeno 3 lati
	//if( solution->size( vehicle ) <= 2 )
	//	return -1;
	//	//return false:

	int edge = (int)( rand() % ( solution->size( vehicle ) - 1 ) );

	// Controllo di poter togliere il lato ricevuto come parametro
	if( !isRemovable( solution, vehicle, edge ) )
		throw -1;
		//return false;

	*src = solution->getEdge( vehicle, edge )->getSrc();
	*dst = solution->getEdge( vehicle, edge )->getDst();

	// Barbascambio di variabili a seconda del verso in cui tale lato viene percorso dal veicolo
	if ( !solution->getDirection( vehicle, edge ) )
		*dst ^= *src ^= *dst ^= *src;

#ifdef DEBUG
	cerr << "In open: v = " << vehicle << " k = " << k << " edge = " << edge << " src = " << *src << " dst = " << *dst << endl;
	cerr << solution->toString();
#endif
	// Rimuovo k+1 lati
	// Itero sul minimo valore tra k e la lunghezza attuale della soluzione (k+1<s => k<s-1!!!)
	//  così da non togliere più lati di quanti la soluzione non ne abbia
	int ktemp = ( k < solution->size( vehicle ) - 1 ?
				  k : (int)solution->size( vehicle ) - 1 );

#ifdef DEBUG
	cerr << "iterazioni da compiere: " << ktemp << endl;
#endif

	// Rimuovo il primo lato
	if ( ktemp > 0 )
		solution->removeEdge( vehicle, edge );
	else 
		throw -1;

	// Rimuovo al più k lati
	//for ( int i = 0; i < ktemp; i++ )
	bool forward	= true,
		 backward	= true;
	int i = 1;
	while( ( forward || backward ) && ( i < ktemp ) )
	{
#ifdef DEBUG
		cerr << "Ho " << endl << solution->toString();
#endif
		// Decido se rimuovere il lato all'inizio (edge-1) o alla fine (edge) del buco creato
		// ( edge > 0 ) serve a garantire che il deposito non venga estromesso dalla soluzione ( se edge è il deposito allora è già stato rimosso un lato a lui connesso e non ne possono essere rimossi altri ), il controllo esterno a verificare circa quasi la stessa cosa
		
		// holeDirection indica in che direzione evolve il buco inserito nella soluzione
		//  - false	:	buco evolve in avanti
		//  - true	:	buco evolve in dietro
		//bool holeDirection;
		
		// Controlliamo di non eliminare lati oltre la lunghezza della soluzione corrente
		//if( edge >= solution->size( vehicle ) )
		if( edge >= solution->size( vehicle ) )
		{
			forward = false;
			//edge = (uint)( solution->size( vehicle ) -1 );
			//holeDirection = true;
		}

		// Controlliamo di non eliminare lati precedenti al deposito iniziale
		//else if ( edge <= 0 )
		if( edge < 1 )
		{
			backward = false;
			//edge = 0;
			//holeDirection = false;
		}
		// Nessun problema sull'arco da rimuovere
	//	else
	//	if( forward )
	//	{
	//		holeDirection = rand() & 2;
	//		edge -= holeDirection;
	//	}

		// Provo a rimuovere un lato da entrambe le parti
		if( forward )
		{
			if( isRemovable( solution, vehicle, edge ) )
			{
				*dst = solution->getEdge( vehicle, edge )->getDst( *dst );
				solution->removeEdge( vehicle, edge );
				i++;
#ifdef DEBUG
				cerr << "Forward: fe = " << edge << " new dst = " << *dst << endl;
#endif
			}
			else
				forward = false;
		}

		// Controllo se posso fare un'altra mossa
		if( i >= ktemp )
			break;

		if( backward )
		{
			if( isRemovable( solution, vehicle, edge - 1 ) )
			{
				*src = solution->getEdge( vehicle, edge - 1 )->getDst( *src );
				solution->removeEdge( vehicle, edge - 1 );
				// Faccio tornare indietro il forward perchè il suo indice sarebbe sfasato di 1, avendo eliminato un lato a lui precedente.
				edge--;
				i++;
#ifdef DEBUG
				cerr << "Backward: be = " << edge << " new src = " << *src;
#endif
			}
			else
				backward = false;
		}
		
/*
		// Controllo se devo aggiornare il nodo di partenza o destinazione, a seconda del lato rimosso
		if ( holeDirection )
			*src = solution->getEdge( vehicle, edge )->getDst( *src );
		else
			*dst = solution->getEdge( vehicle, edge )->getDst( *dst );
		
		// Rimuovo il lato scelto dalla soluzione
		solution->removeEdge( vehicle, edge );
*/
	}
#ifdef DEBUG
		cerr << "Esco con " << endl << solution->toString();
#endif

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
	
#ifdef DEBUG
	cerr << "Bellman chiamato sul veicolo " << vehicle << " per collegare " << src << " con " << dst << " in " << edgeIndex << endl;
#endif
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

Solution Solver::solve( string method, int repetition )
{
	// Numero di iterazioni
	// VNASD
	if( output_file.is_open() )
	{
		if( repetition != -1 )
			output_file << method << repetition << " " << M << endl;
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
			else
			{
				if( !method.compare( "VNAASD" ) )
					currentSolution = vnaasd( N_ITER, currentSolution, repetition );
			}
		}
	}

	for ( int i = 0; i < M; i++ )
		if ( !isFeasible( &currentSolution, i ) )
			throw 3;
	

#ifdef DEBUG
	cerr << "Solve" << currentSolution.toString();
#endif
	return currentSolution;
}

bool Solver::isFeasible( const Solution* solution, int vehicle ) const
{
	return	solution->getDemand( vehicle ) <= Q && solution->getCost( vehicle ) <= tMax;
}

bool Solver::isRemovable( const Solution* solution, int vehicle, int index ) const
{
	Vehicle* tempVehicle = solution->getVehicle( vehicle );
	MetaEdge* tempMeta = solution->getEdge( vehicle, index );
	
	// Se non è la prima volta per cui passo da questo lato, è certamente rimovibile
	for ( int i = 0; i < index; i++ )
		if ( *solution->getEdge( vehicle, i ) == *tempMeta )
			return true;

	// Il lato NON è rimovibile se io lo servo e non sono l'unico a passarci.
	// Ed è la prima volta che ci passo! (vedi sopra)
	// Questo è vero assumendo un bin packing fatto precedentemente
	return !( tempMeta->getDemand() &&
			  tempMeta->isServer( tempVehicle ) &&
			  tempMeta->getTaken() > 1 );
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

int Solver::mrBeanBeanBinPacking( Solution* solution, int vehicle )
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

		if ( edge->getProfit() > 0 && edge->isServer( optimizationVehicle ) )
			served.insert( edge );
	}

	// Ordino i lati trovati in ordine decrescente. Questo rende l'algoritmo First Fit Decreasing.
	vector<MetaEdge*> toServe( served.begin(), served.end() );
	sort( toServe.begin(), toServe.end(), solution->comparePacking );
	
#ifdef DEBUG
	cerr << "Lati serviti ordinati:" << endl;
	for ( auto it = toServe.begin(); it != toServe.end(); ++it )
		cerr << "( " << (*it)->getSrc() << " " << (*it)->getDst() << " ) " << (*it)->getDemand() << endl;
#endif
	
	int swaps = 0;
	// Ciclo sull'intera soluzione del veicolo
	for ( auto it = toServe.begin(); it != toServe.end(); ++it )
	{
		// Prendo i veicoli che passano dal lato
		vector<const Vehicle*> takers = (*it)->getTakers();

		// Ciclo sui takers per cercare di attribuire la domanda ad un altro veicolo
		for( int j = 0; j < takers.size(); j++ )
		{
			if( takers[ j ] != solution->getVehicle( vehicle ) )
			{
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

void Solver::cleanVehicle( Solution* solution, int vehicle )
{
#ifdef DEBUG
	cerr << "Pulizia di " << vehicle << ": " << solution->toString( vehicle );
#endif
	Solution save( *solution );

	uint previous;
	uint next = depot;
	for ( int i = 0; i < solution->size( vehicle ); i++ )
	{
		// Il buco corrente inizia dove era finito il precedente
		previous = next;

		// Finché posso rimuovere lati avere problemi, lo faccio
		while ( i < solution->size( vehicle ) && isRemovable( solution, vehicle, i ) )
		{
			int diffProfit = solution->getProfit( vehicle );

			// Rimuovo il lato i
			MetaEdge* temp = solution->getEdge( vehicle, i );
			solution->removeEdge( vehicle, i );

			diffProfit -= solution->getProfit( vehicle );

			if ( diffProfit == 0 )
				next = temp->getDst( next );
			else
			{
				solution->addEdge( temp->getEdge(), vehicle, i );
				break;
			}
		}

		// Ho un buco da previous a next: se differiscono, lo chiudo direttamente
		//  e sposto l'indice in modo che punti oltre il buco chiuso
		if ( previous != next )
		{
			solution->addEdge( graph.getEdge( previous, next ), vehicle, i );
			
			// Possibilità che, aggiungendo un lato, questo dia profitto,
			//  io lo serva e la sua domanda mi renda unfeasible: controllo.
			if ( !isFeasible( solution, vehicle ) )
			{
#ifdef DEBUG
				cerr << "Pulizia troppo fruttuosa tra " << previous << " e " << next << endl;
#endif
				// Reimposto la soluzione
				*solution = Solution( save );
				next = previous;

				// Non aumentando i, al ciclo successivo ritento lo stesso buco
				//  ma con un lato in meno.
			}
			else
				i++;
		}

		// Aggiorno next se non ho finito il lavoro
		if ( i < solution->size( vehicle ) )
			next = solution->getEdge( vehicle, i )->getDst( next );
	}
	
#ifdef DEBUG
	cerr << "Ripulito: " << solution->toString( vehicle );
	if ( solution->getCost() > save.getCost() )
		cerr << "Sono un vero danno." << endl;
#endif
}

void Solver::optimizeSolution( Solution* solution )
{
	for ( int v = 0; v < M; v++ )
	{
		// Cerco di ottimizzare il veicolo appena shakerato
		mrBeanBeanBinPacking( solution, v );
		cleanVehicle( solution, v );

#ifdef DEBUG
		cerr << "Ottimizzo localmente su " << v << ": " << solution->toString( v );
#endif

		uint previous = depot;
		uint next;
		for ( int i = 0; i < solution->size( v ); i++ )
		{
#ifdef DEBUG
			cerr << "Lavoro sul veicolo " << v << " lato " << i;
			cerr << " ( " << solution->getEdge( v, i )->getSrc() << " " << solution->getEdge( v, i )->getDst() << " ) " << endl;
			cerr << "Parto da: " << solution->toString();
#endif
			// Elimino almeno un lato
			list <Edge*> removedEdges;
			bool wasServer;

			Vehicle* tempVehicle = solution->getVehicle( v );

			MetaEdge* tempMeta = solution->getEdge( v, i );
			wasServer = tempMeta->isServer( tempVehicle );
			// Pro thinking:
			// Se MrBean non è riuscito a riassegnare questo lato ad altri veicoli ed io non sono l'unico che lo attraversa,
			// è inutile cercare di toglierlo dalla soluzione in quanto renderebbe infeasible un altro veicolo, per cui salto.
			if( !isRemovable( solution, v, i ) )
			{
#ifdef DEBUG
				cerr << "Il lato " << i << " non è rimovibile. Passo al lato successivo." << endl;
#endif
				previous = tempMeta->getDst( previous );
				continue;
			}

			removedEdges.push_back( tempMeta->getEdge() );

			solution->removeEdge( v, i );
			next = tempMeta->getDst( previous );

			// Elimino lati dalla soluzione fintanto che questi non ne aumentano il profitto e fintanto che sono presenti nella soluzione
			while( i < solution->size( v ) && isRemovable( solution, v, i ) )
			{
				// Calcolo la differenza di profitto che abbiamo nel togliere un lato alla soluzione
				int diffProfit = solution->getProfit( v );

				// Rimuovo il lato i
				MetaEdge* temp = solution->getEdge( v, i );
				solution->removeEdge( v, i );

				diffProfit -= solution->getProfit( v );

				// Se non ho differenze di profitto, tolgo quel lato dalla soluzione
				if( diffProfit == 0 )
				{
					// Sposto il nodo di partenza
					next = temp->getDst( next );
					// Inserisco il lato tolto nella lista
					removedEdges.push_back( temp->getEdge() );
				}
				else
				{
					// Altrimenti lo riaggiungo
					solution->addEdge( temp->getEdge(), v, i );
					//temp->setServer( tempVehicle );
					break;
				}

			}

#ifdef DEBUG
			cerr << "Creato un buco di " << removedEdges.size() << " lati su ( " << previous << " " << next << " ) " << endl;
			cerr << "Cristo: " << solution->toString() << endl;
#endif

			// Chiedo a Dijkstra di calcolarmi la chiusura migliore
			list<Edge*> closure = closeSolutionDijkstra( *solution, v, previous, next, i );
			previous = next;

			if ( !closure.size() )
			{
				// Quell'incapace del Sig. Bellman-Ford-Zucchelli ha fallito: ripristino.
				for( auto it = removedEdges.rbegin(); it != removedEdges.rend(); ++it )
					solution->addEdge( *it, v, i );

				i += removedEdges.size() - 1;
				continue;
			}

			// Se questo porta un miglioramento, effettuo la chiusura, altrimenti riaggiungo il lato i
			for ( auto it = closure.rbegin(); it != closure.rend(); ++it )
				solution->addEdge( *it, v, i );

#ifdef DEBUG
			cerr << "Soluzioni dopo ricerca locale: " << solution->toString() << endl;
#endif

			// Do per scontato che Bellman mi dia, nel caso peggiore, una soluzione identica
			i += closure.size() - 1;
		}
	}

	for ( int i = 0; i < M; i++ )
		if ( !isFeasible( solution, i ) )
			throw 3;
}

