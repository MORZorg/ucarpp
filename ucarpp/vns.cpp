//
//  vns.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "solution.h"
#include "solver.h"

using namespace std;
using namespace solver;
using namespace model;


Solution Solver::vns( int nIter, Solution baseSolution )
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
		// Copio la soluzione di base su una soluzione che elaborerò nella vns
		shakedSolution = Solution( baseSolution );
		
		/*** Shaking ***/
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
		src = shakedSolution.getEdge( vehicle, edge )->getSrc();
		dst = shakedSolution.getEdge( vehicle, edge )->getDst();
		
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
				src = shakedSolution.getEdge( vehicle, edge )->getDst( src );
			else
				dst = shakedSolution.getEdge( vehicle, edge )->getDst( dst );
			
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
		int kvns = ceil( XI * ( k + 1 ) );
		
//		while( !closeSolutionRandom( &shakedSolution, vehicle, src, dst, kvns, edge ) );
		if ( !closeSolutionRandom( &shakedSolution, vehicle, src, dst, kvns, edge ) )
			throw 200;
		
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
				break; // SFANCULATI BEST
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

