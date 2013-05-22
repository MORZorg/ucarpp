//
//  main.cpp
//  ucarpp
//	Considera il primo parametro passato il file da leggere ed il secondo il numero di veicoli da
//	utilizzare. In caso il secondo non venga specificato, il problema viene risolto con un veicolo.
//
//  Created by Maurizio Zucchelli on 03/25/13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "main.h"

using namespace std;
using namespace boost;

int main( int argc, const char * argv[] )
{
	if ( argc == 0 )
	{
		cout << "Usage:" << endl << "\tucarpp path [M]" << endl;
		exit( 1 );
	}
	
	int V = 0,
		M = 1,
		L = 0,
		Q = 0,
		tMax = 0,
		depot = 0;
	string filename;
	if ( argc > 2 )
	{
		M = stoi( argv[ 2 ] );
	}
	
	/** Lettura dei dati in ingresso */
	ifstream in;
	in.open( argv[ 1 ] );
	string line;
	smatch sm;
	regex re;
	
	re = *new regex( "NUMBER:\\s(.*\\.dat)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		filename = sm[ 1 ];
		cerr << "File: " << sm[ 1 ] << endl;
	}
	else
	{
		cerr << "Errore nella lettura del nome del file. " << line << endl;
		exit( 1 );
	}
	
	// Dati
	re = *new regex( "NUMBER OF VERTICES:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		V = stoi( sm[ 1 ] );
		
		cerr << "Vertici: " << V << endl;
	}
	else
	{
		cerr << "Errore nella lettura del numero di vertici. " << line << endl;
		exit( 1 );
	}
	
	re = *new regex( "NUMBER OF EDGES:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		L = stoi( sm[ 1 ] );
		
		cerr << "Lati: " << L << endl;
	}
	else
	{
		cerr << "Errore nella lettura del numero di lati. " << line << endl;
		exit( 1 );
	}
	
	re = *new regex( "CAPACITY:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		Q = stoi( sm[ 1 ] );
		
		cerr << "Capacita`: " << Q << endl;
	}
	else
	{
		cerr << "Errore nella lettura della capacita`. " << line << endl;
		exit( 1 );
	}
	
	re = *new regex( "TIME LIMIT:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		tMax = stoi( sm[ 1 ] );
		
		cerr << "Tempo disponibile: " << tMax << endl;
	}
	else
	{
		cerr << "Errore nella lettura del tempo disponibile. " << line << endl;
		exit( 1 );
	}
	
	// Lati
	model::Graph grafo( V );
	
	re = *new regex( "LIST OF EDGES:" );
	getline( in, line );
	if ( !regex_search( line, re ) )
		cerr << "Errore. " << line << endl;
	
	int src, dst, t, d;
	float p;
	re = *new regex( "\\((\\d+),(\\d+)\\) cost (\\d+) demand (\\d+) profit ([0-9.]+)" );
	for ( int i = 0; i < L; i++ )
	{
		getline( in, line );
		regex_search( line, sm, re );
		if ( sm.size() < 6 )
		{
			cerr << "Errore nella lettura dei dati del lato. " << line << endl;
			exit( 1 );
		}
		else
		{
			src = stoi( sm [ 1 ] ) - 1;

			dst = stoi( sm[ 2 ] ) - 1;

			t = stoi( sm[ 3 ] );

			d = stoi( sm[ 4 ] );

			p = stoi( sm[ 5 ] );
			
			grafo.addEdge( src, dst, t, d, p );
			
			fprintf( stderr, "(%d, %d): %d, %d, %.2f.\n", src, dst, t, d, p );
		}
	}
	
	re = *new regex( "DEPOT:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		depot = stoi( sm[ 1 ] ) - 1;

		cerr << "Deposito: " << depot << endl;
	}
	else
	{
		cerr << "Errore nella lettura del deposito. " << line << endl;
		exit( 1 );
	}
	
	in.close();
	
	// Completo la magliatura del grafo
	grafo.completeCosts();
#ifdef DEBUG
	cerr << "Matrice dei costi: C (P, D) " << endl;
	for ( int i = 0; i < V; i++ )
	{
		for ( int j = 0; j < V; j++ )
		{
			if ( i == j )
			{
				cerr << "auto\t\t";
				continue;
			}
			model::Edge* edge = grafo.getEdge( i, j );
			cerr << edge->getCost() << " (" << edge->getProfit() << ", " << edge->getDemand() << ") \t";
		}
		cerr << endl;
	}
#endif
	
	// Creo il risolutore
	solver::Solver solver( grafo, depot, M, Q, tMax );
	solver::Solution* solution = solver.solve();
	
#ifdef FORMAL_OUT
	// Stampo l'output
	cout << "Solution of Problem " << filename << " - Number of Vehicles: " << M << endl << endl;
	cout << "Total Profit: " << solution->getProfit() << endl << endl;
	cout << "Total Cost: " << solution->getCost() << endl << endl;
	for ( int i = 0; i < M; i++ )
	{
		cout << endl << "Route " << i << " Details:" << endl;

		cout << endl << "Services Sequence:" << endl;
		cout << ":)" << endl;

		cout << endl << "Vertex Sequence:" << endl;
		cout << ":]" << endl;
		
		cout << endl;
		cout << "Profit: " << solution->getProfit( i ) << endl;
		cout << "Cost: " << solution->getCost( i ) << endl;
		cout << "Load: " << solution->getDemand( i ) << endl;
		cout << endl;
	}
#endif
	
    return 0;
}
