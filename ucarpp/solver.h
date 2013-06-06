//
//  solver.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__solver__
#define __ucarpp__solver__

//#include <unordered_set>
#include <list>
#include <vector>
#include <sstream>

#include <fstream>
#include <string>

#include "headings.h"
#include "edge.h"
#include "graph.h"
#include "meta.h"
#include "solution.h"

namespace solver
{
	class Solver
	{
		private:
			const int	N_ITER		= 200;
			const int	K_MAX		= 10;
			const float	XI			= 4;
			const float	P_CLOSE		= .25;
			const float	P_ACCEPT	= .95;
			const std::string OUTPUT_FILE_DIR = "../progressive_output/";
			const std::string OUTPUT_FILE_EXTENSION = ".sbra";

			model::Graph graph;
			uint depot,
			M,
			Q,
			tMax;
			Solution currentSolution;
			std::ofstream output_file;
			
			Solution createBaseSolution();
			void createBaseSolution( Solution*, int );
			Solution vns( int, Solution );
			Solution vnd( int, Solution );
			Solution vnasd( int, Solution, int );
			Solution vnaasd( int, Solution, int );

			// Metodi usati per modificare progressivamente la soluzione
			// Metodo che sceglie come mutare la soluzione, decidendo se ampliandola o restringendola. Ritorna la lunghezza della nuova soluzione modificata.
			uint mutateSolution( Solution*, uint, int );
			// La soluzione viene chiusa, ovvero due lati adiacenti vengono sostituiti con uno che ne collega gli estremi.
			bool mutateSolutionClose( Solution*, uint, int = -1 );
			// La soluzione viene ampliata, inserendo un nuovo lato in quello indicato.
			bool mutateSolutionOpen( Solution*, uint, int = -1 );

			// Creo un buco nella soluzione di più lati adiacenti. Usato solo nella vnd.
			uint openSolutionRandom( Solution*, uint, int, uint*, uint* );
			// Metodo inefficiente perchè potenzialmente esplosivo a causa del numero di chiusure possibili.
			bool closeSolutionRandom( Solution*, int, uint, uint, int, int );
			std::list<model::Edge*> closeSolutionDijkstra( Solution, int, uint, uint, int );

			// Metodo basato sul concetto della Bin Packing, usato per cercare una prima ottimizzazione della soluzione.
			// Il metodo può essere richiamato anche più volte in ogni ciclo di risoluzione.
			int mrBeanBeanBinPacking( Solution*, uint );

			bool isFeasible( const Solution*, int ) const;
			bool isRemovable( const Solution*, int, int ) const;

			void printToFile( Solution* );

		public:
			Solver( model::Graph, uint, uint, uint, uint );
			
			Solution solve( std::string, int );

			bool setOutputFile( std::string );
	};
}

#endif /* defined(__ucarpp__solver__) */
