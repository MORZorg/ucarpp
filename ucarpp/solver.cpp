//
//  solver.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "solver.h"

#ifndef DEBUG
#define DEBUG
#endif

#ifndef OUTPUT_FILE
#define OUTPUT_FILE
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

Solution Solver::vns( int nIter, Solution baseSolution )
{
	// Inizializzo il generatore di numeri casuali
	srand( (uint)time( NULL ) );
	int k = 1;
	// Creo una copia della soluzione iniziale sulla quale applicare la vns
	Solution shakedSolution = baseSolution;
	Solution* optimalSolution = new Solution( baseSolution );

	/*
#ifdef OUTPUT_FILE
	// Stampo informazioni sulla soluzione iniziale
	cout << "VNS" << endl;
	output_file << "VNS" << endl;
	output_file << baseSolution.getProfit() << " " << baseSolution.getCost() << " " << baseSolution.getDemand() << endl;
#endif
*/
	
	// Ciclo fino a quando la stopping rule me lo consente o prima se trovo una soluzione migliore di quella iniziale
	while ( nIter-- > 0 )
	{
		// Copio la soluzione di base su una soluzione che elaborerò nella vns
		shakedSolution = Solution( baseSolution );
		
		/*** Shaking ***/
		// Estraggo un veicolo ed un lato iniziale casuali
		// Tengo traccia anche dei nodi sorgente e destinazione di tale lato
		uint vehicle = rand() % M,
			 edge,
			 src,
			 dst;

		// Non mi interesso del valore di ritorno perchè pressoche inutile. :D
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

		uint previous = depot;
		uint next;
		for ( int i = 0; i < shakedSolution.size( vehicle ); i++ )
		{
			// Elimino almeno un lato
			list <Edge*> removedEdges;
			removedEdges.push_back( shakedSolution.getEdge( vehicle, i )->getEdge() );
			shakedSolution.removeEdge( vehicle, i );
			next = removedEdges.front()->getDst( previous );

			// Elimino lati dalla soluzione fintanto che questi non ne aumentano il profitto e fintanto che sono presenti nella soluzione
			while( i <  shakedSolution.size( vehicle ) )
			{
				// Calcolo la differenza di profitto che abbiamo nel togliere un lato alla soluzione
				int diffProfit = shakedSolution.getProfit( vehicle );

				// Rimuovo il lato i
				Edge* temp = shakedSolution.getEdge( vehicle, i )->getEdge();
				shakedSolution.removeEdge( vehicle, i );

				diffProfit -= shakedSolution.getProfit( vehicle );

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
					shakedSolution.addEdge( temp, vehicle, i );
					break;
				}

			}

#ifdef DEBUG
			cerr << "Creato un buco di " << removedEdges.size() << " lati" << endl;
#endif

			// Chiedo a Dijkstra di calcolarmi la chiusura migliore
			/*
			list<Edge*> closure = closeSolutionDijkstra( shakedSolution, vehicle, previous,
														temp->getDst( previous ), i );
			previous = temp->getDst( previous );
			*/
			list<Edge*> closure = closeSolutionDijkstra( shakedSolution, vehicle, previous, next, i );
			previous = next;
			
			// Se questo porta un miglioramento, effettuo la chiusura, altrimenti riaggiungo il lato i
			for ( auto it = closure.rbegin(); it != closure.rend(); ++it )
				shakedSolution.addEdge( *it, vehicle, i );
			
			// Controllo se ho trovato una soluzione migliore della massima trovata in precedenza
			if ( shakedSolution > maxSolution )
			{
				maxSolution = Solution( shakedSolution );
				//break; // SFANCULATI BEST
			}

			// Resetto la shakedSolution per effettuare una nuova ricerca
			for( int j = 0; j < closure.size(); j++ )
				shakedSolution.removeEdge( vehicle, i );

			for( auto it = removedEdges.rbegin(); it != removedEdges.rend(); ++it )
				shakedSolution.addEdge( *it, vehicle, i );

			i += removedEdges.size() - 1;
			//shakedSolution.addEdge( temp, vehicle, i );
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
		
		k = 1 + k % K_MAX;
	}
	
#ifdef DEBUG
	cerr << "VND" << optimalSolution->toString();
#endif
	return *optimalSolution;
}

uint Solver::mutateSolution( Solution *solution, uint vehicle, int k )
{
	srand( (uint)time( NULL ) );

	// Devo modificare la soluzione passatami per k volte
	// Ciclo sul numero di volte calcolato
	for( int i = 0; i < k; i++ )
	{
		// Scelgo un lato sul quale operare
		uint edge = rand() % solution->size( vehicle );

		// Casualmente scelgo se aprire o chiudere un lato
		// Non mi interesso del valore di ritorno delle funzioni usate perchè so già dove il buco è stato creato, essendo io a passarlo come parametro.
		if( (float) rand() / RAND_MAX > P_CLOSE )
		{
			mutateSolutionOpen( solution, vehicle, edge );
		}
		else
		{
			mutateSolutionClose( solution, vehicle, edge );
		}
	}

	return solution->size();
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
	int testables = adj.size();
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

Solution Solver::solve()
{
	// Numero di iterazioni
	currentSolution = vns( N_ITER, currentSolution );
	
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
