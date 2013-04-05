//
//  node.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 04/04/13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "edge.h"

/*** Edge ***/

/**
 * Costruttore.
 * Salva in src il nodo con indice minore tra src e dst ed in dst il rimanente.
 *
 * @param src		nodo sorgente
 * @param dst		nodo destinazione
 * @param demand	domanda del lato
 * @param profit	profitto del lato
 */
Edge::Edge( uint src, uint dst ):
	src( src < dst ? src : dst ),
	dst( src > dst ? src : dst ),
	taken( 0 ) {}

/**
 * Ritorna il nodo destinazione a partire da un dato nodo di partenza.
 *
 * @param src	il nodo di partenza di cui si vuole sapere la destinazione
 * @return	il nodo di destinazione. Se il nodo di partenza non è appartenente all'arco, viene
 *			ritornato il nodo salvato come "dst".
 */
uint Edge::getSrc()
{
	return src;
}

/**
 * Ritorna il nodo destinazione a partire da un dato nodo di partenza.
 *
 * @param src	il nodo di partenza di cui si vuole sapere la destinazione
 * @return	il nodo di destinazione. Se il nodo di partenza non è appartenente all'arco, viene
 *			ritornato il nodo salvato come "dst".
 */
uint Edge::getDst()
{
	return dst;
}

/**
 * Ritorna il nodo destinazione a partire da un dato nodo di partenza.
 *
 * @param src	il nodo di partenza di cui si vuole sapere la destinazione
 * @return	il nodo di destinazione. Se il nodo di partenza non è appartenente all'arco, viene
 *			ritornato il nodo salvato come "dst".
 */
uint Edge::getDst( uint src )
{
	return src == this->dst ? this->src : this->dst;
}

/**
 * Getter per la domanda associata all'arco.
 *
 * @return	0, essendo l'arco prido di domanda.
 */
uint Edge::getDemand() { return 0; }

/**
 * Getter per il profitto associato all'arco.
 *
 * @return	0, essendo l'arco prido di profitto.
 */
float Edge::getProfit() { return 0; }

/*** ProfitEdge ***/

ProfitEdge::ProfitEdge( uint src, uint dst, uint demand, float profit ):
	Edge( src, dst ),
	demand( demand ),
	profit( profit ) {}

/**
 * Getter per la domanda associata all'arco.
 *
 * @return	la domanda associata all'arco.
 */
uint ProfitEdge::getDemand()
{
	return demand;
}

/**
 * Getter per il profitto associato all'arco.
 *
 * @return	il profitto associato all'arco.
 */
float ProfitEdge::getProfit()
{
	return profit;
}