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
 */
Edge::Edge( uint src, uint dst ):
	src( src < dst ? src : dst ),
	dst( src > dst ? src : dst ) {}


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
 * Getter per il costo associato all'arco.
 *
 * @return	il costo associato all'arco.
 */
uint Edge::getCost() const
{
	return cost;
}

/**
 * Comparatore di uguaglianza tra archi.
 *
 * @return	vero, se gli archi hanno le stesse proprietà
 *			( sorgente, destinazione, costo, profitto, domanda )
 */
bool Edge::operator ==( const Edge& other ) const
{
	return this->equals( other );
}

bool Edge::equals( const Edge& other ) const
{
	return getSrc() == other.getDst( getDst() ) &&
		   getDst() == other.getDst( getSrc() );
		   /*&&
		   getCost() == other->getCost() &&
		   getProfit() == other->getProfit() &&
		   getDemand() == other->getDemand();
		   */
}

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

/*** DijkyEdge ***/

/**
 * Costruttore.
 *
 * @param src		nodo sorgente
 * @param dst		nodo destinazione
 */
DijkyEdge::DijkyEdge( uint src, uint dst ):
	Edge( src, dst ) {}

/**
 * Setter per il costo associato all'arco.
 *
 * @param cost	il costo da associare all'arco.
 */
void DijkyEdge::setCost( uint cost )
{
	this->cost = cost;
}

/**
 * Getter per la domanda associata all'arco.
 *
 * @return	0, essendo l'arco privo di costo.
 */
uint DijkyEdge::getDemand() const
{
	return 0;
}

/**
 * Getter per il profitto associato all'arco.
 *
 * @return	0, essendo l'arco privo di costo.
 */
float DijkyEdge::getProfit() const
{
	return 0;
}


/*** ProfitableEdge ***/

ProfitableEdge::ProfitableEdge( uint src, uint dst, uint cost, uint demand, float profit ):
	Edge( src, dst ),
	demand( demand ),
	profit( profit )
{
	this->cost = cost;
}

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
