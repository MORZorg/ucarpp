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

int main( int argc, const char * argv[] )
{
	if ( argc == 0 )
	{
		cout << "Usage:" << endl << "\tucarpp path [M]" << endl;
		exit( 1 );
	}
	
	int V = 0,
		M = 0,
		L = 0,
		Q = 0,
		tMax = 0,
		depot = 0;
	if ( argc > 2 )
		M = stoi( argv[ 2 ] );
	else
		M = 1;
	
	/** Lettura dei dati in ingresso */
	ifstream in;
	in.open( argv[ 1 ] );
	string line;
	smatch sm;
	regex re;
	
	try
	{
		// Intestazione
		//regex re1( "NUMBER:\\s(.*)\\.dat" );
		re = *new regex( "NUMBER: (.*).dat" );
		getline( in, line );
		if ( regex_search( line, sm, re ) )
			cerr << "File: " << sm[ 1 ] << endl;
		else
			cerr << "Errore. " << line;
		
		// Dati
		re = *new regex( "NUMBER OF VERTICES:\\s(\\d+)" );
		getline( in, line );
		if ( regex_search( line, sm, re ) )
		{
			V = stoi( sm[ 1 ] );
			
			cerr << "Vertici: " << V << endl;
		}
		else
			cerr << "Errore. " << line;
	}
	catch (exception& e)
	{
		cerr << "_" << endl;
	}
	re = *new regex( "NUMBER OF EDGES:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		L = stoi( sm[ 1 ] );
		
		cerr << "Lati: " << L << endl;
	}
	else
		cerr << "Errore. " << line;
	
	re = *new regex( "CAPACITY:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		Q = stoi( sm[ 1 ] );
		
		cerr << "Capacita`: " << Q << endl;
	}
	else
		cerr << "Errore. " << line;
	
	re = *new regex( "TIME LIMIT:\\s(\\d+)" );
	getline( in, line );
	if ( regex_search( line, sm, re ) )
	{
		tMax = stoi( sm[ 1 ] );
		
		cerr << "Tempo disponibile: " << tMax << endl;
	}
	else
		cerr << "Errore. " << line;
	
	// Lati
	Graph grafo( V );
	
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
			cerr << "Errore. " << line << endl;
		else
		{
			src = stoi( sm[ 1 ] ) -1;
			dst = stoi( sm[ 2 ] ) -1;
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
		depot = stoi( sm[ 1 ] ) -1;
		cerr << "Deposito: " << depot << endl;
	}
	else
		cerr << "Errore. " << line << endl;
	
	in.close();
	
	// Completo la magliatura del grafo
	cerr << "Matrice dei costi: " << endl;
	grafo.completeCosts();
	for ( int i = 0; i < V; i++ )
	{
		for ( int j = 0; j < V; j++ )
			cerr << grafo.getCost( i, j ) << "\t";
		cerr << endl;
	}
	
    return 0;
}

