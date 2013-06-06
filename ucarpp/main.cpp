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

#ifndef DEBUG
//#define DEBUG
#endif

#ifndef OUTPUT_FILE
#define OUTPUT_FILE
#endif

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
	
	re = regex( "NUMBER:\\s(.*\\.dat)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		filename = sm[ 1 ];
#ifdef DEBUG
		cerr << "File: " << sm[ 1 ] << endl;
#endif
	}
	else
	{
		cerr << "Errore nella lettura del nome del file. " << line << endl;
		exit( 1 );
	}

	// Dati
	re = regex( "NUMBER OF VERTICES:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		V = stoi( sm[ 1 ] );
		
#ifdef DEBUG
		cerr << "Vertici: " << V << endl;
#endif
	}
	else
	{
		cerr << "Errore nella lettura del numero di vertici. " << line << endl;
		exit( 1 );
	}
	
	re = regex( "NUMBER OF EDGES:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		L = stoi( sm[ 1 ] );
		
#ifdef DEBUG
		cerr << "Lati: " << L << endl;
#endif
	}
	else
	{
		cerr << "Errore nella lettura del numero di lati. " << line << endl;
		exit( 1 );
	}
	
	re = regex( "CAPACITY:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		Q = stoi( sm[ 1 ] );
		
#ifdef DEBUG
		cerr << "Capacita`: " << Q << endl;
#endif
	}
	else
	{
		cerr << "Errore nella lettura della capacita`. " << line << endl;
		exit( 1 );
	}
	
	re = regex( "TIME LIMIT:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		tMax = stoi( sm[ 1 ] );
		
#ifdef DEBUG
		cerr << "Tempo disponibile: " << tMax << endl;
#endif
	}
	else
	{
		cerr << "Errore nella lettura del tempo disponibile. " << line << endl;
		exit( 1 );
	}
	
	// Lati
	model::Graph grafo( V );
	
	re = regex( "LIST OF EDGES:" );
	getline( in, line );
	if ( !regex_search( line, re ) )
		cerr << "Errore. " << line << endl;
	
	int src, dst, t, d;
	float p;
	re = regex( "\\((\\d+),(\\d+)\\) cost (\\d+) demand (\\d+) profit ([0-9.]+)" );
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
			
#ifdef DEBUG
			fprintf( stderr, "(%d, %d): %d, %d, %.2f.\n", src, dst, t, d, p );
#endif
		}
	}
	
	re = regex( "DEPOT:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		depot = stoi( sm[ 1 ] ) - 1;

#ifdef DEBUG
		cerr << "Deposito: " << depot << endl;
#endif
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

	// Leggo il tipo di metodo da utilizzare per risolvere il problema.
	// Se non indicato uso la VNS.
	string method;
	int repetition = -1;
	if( argc > 3 )
	{
		method = string( argv[ 3 ] );
#ifdef DEBUG
		cerr << "'" << method << "'" << endl;
#endif
		// Uso una regex per controllare se è stata richiesta la funzione alternata vnasd che alterna vns e vnd.
		re = regex( "VNASD(\\d+)" );
		if( regex_search( method, sm, re ) )
		{
#ifdef DEBUG
			cerr << endl;
		    for ( auto x : sm )
				cerr << "'" <<  x << "' ";
			cerr << endl;
			cerr << "Valore letto: " << sm[ 1 ] << " da " << method;
			cerr << " lunga " << method.length() << endl;
#endif

			repetition = stoi( sm[ 1 ] );
			method = "VNASD";
		}

		re = regex( "VNAASD(\\d+)" );
		if( regex_search( method, sm, re ) )
		{
			repetition = stoi( sm[ 1 ] );
			method = "VNAASD";
		}

	}
	else
		method = "VNS";

	// Controllo se devo risolvere il problema per le istanze modificate, ovvero con tempo massimo = 40 e capacità = 30.
	string type;
	if( argc > 4 && !strcmp( argv[ 4 ], "MDF" ) )
	{
		Q = 30;
		tMax = 40;
		type = "MDF";
	}
	else
		type = "ORG";

	// Creo il risolutore
	solver::Solver solver( grafo, depot, M, Q, tMax );
	// Se richiesto, imposto il nome del file sul quale scrivere i risultati intermedi
#ifdef OUTPUT_FILE
	filename = filename.replace( filename.find( "dat" ), 3, to_string( M ) );
	filename += "." + method;
	if ( repetition != -1 )
		filename += to_string( repetition );
	filename += "." + type;
	
	solver.setOutputFile( filename );
#endif

	solver::Solution solution = solver.solve( method, repetition );
	
//	cerr << "main" << solution.toString();
#ifdef FORMAL_OUT
	// Stampo l'output
	cout << "Solution of Problem " << filename << " - Number of Vehicles: " << M << endl << endl;
	cout << "Total Profit: " << solution.getProfit() << endl << endl;
	cout << "Total Cost: " << solution.getCost() << endl << endl;
	for ( int i = 0; i < M; i++ )
	{
		cout << endl << "Route " << i << " Details:" << endl;

		cout << endl << "Services Sequence:" << endl;
		cout << solution.toServicesSequence( i ) << endl;

		cout << endl << "Vertex Sequence:" << endl;
		cout << solution.toVertexSequence( i ) << endl;
		
		cout << endl;
		cout << "Profit: " << solution.getProfit( i ) << endl;
		cout << "Cost: " << solution.getCost( i ) << endl;
		cout << "Load: " << solution.getDemand( i ) << endl;
		cout << endl;
	}

	cout << solution.toString() << endl;
#endif

    return 0;
}
