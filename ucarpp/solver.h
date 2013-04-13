//
//  solver.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__solver__
#define __ucarpp__solver__

#include "graph.h"

#ifdef DEBUG
#include <iostream>
#endif


class Solution
{
public:
	vector<Edge> path;
	
	Solution();
};

class Solver
{
private:
	Graph graph;
	int M;
	Solution* currentSolution;
	
	void createBaseSolution();
	
	
	struct compareRatioDescending
	{
		bool operator()( const Edge& lhs, const Edge& rhs ) const
		{
			if ( lhs.getProfitDemandRatio() == rhs.getProfitDemandRatio() )
				return graph.getCost( lhs ) > graph.getCost( rhs );
			
			return lhs.getProfit() > rhs.getProfit();
		}
	} compareEdges;
public:
	Solver( Graph, int );
};

#endif /* defined(__ucarpp__solver__) */
