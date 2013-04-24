//
//  node.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 04/04/13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "edge.h"

using namespace std;
using namespace model;

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
 * Segna il nodo come preso, incrementando il contatore del numero di volte per cui il nodo è stato preso.
 *
 * @return	il numero di volte per cui il lato è stato preso.
 */
uint Edge::setTaken()
{
	return ++taken;
}

/**
 * Decrementa il contatore del numero di volte per cui il nodo è stato preso fino a renderlo non preso (valore 0);
 *
 * @return	il numero di volte per cui il lato è stato preso, 0 se ciò non è mai successo.
 */
uint Edge::unsetTaken()
{
	if ( taken > 0 )
		return --taken;
	
	return 0;
}

/**
 * Getter del numero di volte per cui il lato è stato preso.
 *
 * @return	il numero di volte per cui il lato è stato preso, 0 se ciò non è mai successo.
 */
uint Edge::getTaken() const
{
	return taken;
}


/**
 * Ritorna il nodo destinazione a partire da un dato nodo di partenza.
 *
 * @param src	il nodo di partenza di cui si vuole sapere la destinazione
 * @return	il nodo di destinazione. Se il nodo di partenza non è appartenente all'arco, viene
 *			ritornato il nodo salvato come "dst".
 */
uint Edge::getSrc() const
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
uint Edge::getDst() const
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
uint Edge::getDst( uint src ) const
{
	return src == this->dst ? this->src : this->dst;
}

/**
 * Getter per la domanda associata all'arco.
 *
 * @return	0, essendo l'arco privo di domanda.
 */
 uint Edge::getDemand() const { return 0; }

/**
 * Getter per il profitto associato all'arco.
 *
 * @return	0, essendo l'arco privo di profitto.
 */
float Edge::getProfit() const { return 0; }

/**
 * Calcola il rapporto tra profitto e domanda, utile per l'ordinamento.
 *
 * @return profitto/domanda se domanda != 0, -1 altrimenti
 */
float Edge::getProfitDemandRatio() const
{
	if ( getDemand() != 0 )
		return getProfit() / getDemand();
	else
		return -1;
}

/*** ProfitEdge ***/

ProfitableEdge::ProfitableEdge( uint src, uint dst, uint demand, float profit ):
	Edge( src, dst ),
	demand( demand ),
	profit( profit ) {}

/**
 * Getter per la domanda associata all'arco.
 *
 * @return	la domanda associata all'arco.
 */
uint ProfitableEdge::getDemand() const
{
	return demand;
}

/**
 * Getter per il profitto associato all'arco.
 *
 * @return	il profitto associato all'arco.
 */
float ProfitableEdge::getProfit() const
{
	return profit;
}
