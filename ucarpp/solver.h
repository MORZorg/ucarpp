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

			model::Graph graph;
			uint depot,
			M,
			Q,
			tMax;
			Solution currentSolution;
			
			Solution createBaseSolution();
			void createBaseSolution( Solution*, int );
			Solution vns( int, Solution );
			Solution vnd( int, Solution );
			bool closeSolutionRandom( Solution*, int, uint, uint, int, int );
			std::list<model::Edge*> closeSolutionDijkstra( Solution, int, uint, uint, int );

			bool isFeasible( const Solution*, int ) const;
		public:
			Solver( model::Graph, uint, uint, uint, uint );
			
			Solution solve();
	};
}

#endif /* defined(__ucarpp__solver__) */
