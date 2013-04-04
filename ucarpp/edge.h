//
//  node.h
//  ucarpp
//
//  Created by Maurizio Zucchelli on 04/04/13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#ifndef __ucarpp__edge__
#define __ucarpp__edge__

typedef unsigned int uint;

class Edge
{
	uint src,
		 dst,
		 demand;
	float profit;
	uint taken;
	
public:
	Edge( uint, uint, uint, float );
	uint getDst( uint );
};

#endif /* defined(__ucarpp__node__) */
