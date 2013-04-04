//
//  node.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 04/04/13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "edge.h"

/**
 * Costruttore.
 *
 * @param src		nodo sorgente
 * @param dst		nodo destinazione
 * @param demand	domanda del lato
 * @param profit	profitto del lato
 */
Edge::Edge( uint src, uint dst, uint demand, float profit )
{
	this->src = src;
	this->dst = dst;
	this->demand = demand;
	this->profit = profit;
	
	// Imposto il lato come mai preso in soluzione.
	this->taken = 0;
}

/**
 * Ritorna il nodo destinazione a partire da un dato nodo di partenza.
 * Se il nodo di partenza non appartiene all'arco, viene restituito il nodo salvato come "src".
 */
uint Edge::getDst( uint src )
{
	return this->src == src ? this->dst : this->src;
}