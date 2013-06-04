//
//  meta.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-05-09.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "meta.h"

using namespace std;
using namespace solver;
using namespace model;

/*** MetaEdge ***/

/**
 * Costruttore
 */
MetaEdge::MetaEdge( Edge* reference ):
	actualEdge( reference )
{
	takers = vector<const Vehicle*>();
}

MetaEdge::MetaEdge( const MetaEdge& source ):
	actualEdge( source.actualEdge )
{
	// TODO: Copio le statistiche ma non i lati passanti
	takers = vector<const Vehicle*>();
}

uint MetaEdge::getSrc() const
{
	return actualEdge->getSrc();
}

uint MetaEdge::getDst() const
{
	return actualEdge->getDst();
}

uint MetaEdge::getDst( uint src ) const
{
	return actualEdge->getDst( src );
}

/**
 * Getter per il costo associato all'arco.
 *
 * @return	il costo associato all'arco.
 */
uint MetaEdge::getCost() const
{
	return actualEdge->getCost();
}

/**
 * Getter per la domanda associata all'arco.
 *
 * @return	la domanda associata all'arco.
 */
uint MetaEdge::getDemand() const
{
	return actualEdge->getDemand();
}

/**
 * Getter per il profitto associato all'arco.
 *
 * @return	il profitto associato all'arco.
 */
float MetaEdge::getProfit() const
{
	return actualEdge->getProfit();
}

/**
 * Getter del rapporto tra profitto e domanda associati all'arco, utile per l'ordinamento.
 *
 * @return profitto/domanda se domanda != 0, -1 altrimenti
 */
float MetaEdge::getProfitDemandRatio() const
{
	return actualEdge->getProfitDemandRatio();
}

/**
 * Segna il lato come preso dal veicolo chiamante la funzione.
 *
 * @param	il veicolo che imposta questo lato come preso.
 * @return	il numero di volte per cui il lato è stato preso.
 */
unsigned long MetaEdge::setTaken( const Vehicle* taker, int occurrence )
{
	// Cerco n le occorrenze del veicolo. Inserisco prima della (n+1)-esima.
	for ( auto it = takers.begin(); it < takers.end(); ++it )
		if ( **it == *taker )
			if ( occurrence-- < 0 )
			{
				// In caso si usino i reverse_iterator,
				// bisogna usare .base() facendo piu' o meno ++i--
				takers.insert( it, taker );
				return takers.size();
			}

	// Non ho trovato n+1 occorrenze. Inserisco in coda.
	takers.push_back( taker );
	
	return takers.size();
}

/**
 * Segna la n-esima occorrenza del lato come non preso dal veicolo chiamante
 * la funzione.
 * ! Può causare un cambio di veicolo servente in caso il veicolo rimosso sia il servente attuale.
 *	 Questo può dar luogo a soluzioni infeasible.
 *
 * @param	il veicolo che imposta questo lato come non preso.
 * @param	l'occorrenza del veicolo da rimuovere.
 * @return	il numero di volte per cui il lato è stato preso, 0 se ciò non è mai successo.
 */
unsigned long MetaEdge::unsetTaken( const Vehicle* taker, int occurrence )
{
	// Cerco il veicolo. Se lo trovo lo cancello, altrimenti niente.
	for ( auto it = takers.begin(); it < takers.end(); ++it )
		if ( **it == *taker )
			if ( --occurrence < 0 )
			{
				// In caso si usino i reverse_iterator,
				// bisogna usare .base() facendo piu' o meno ++i--
				takers.erase( it );
				break;
			}
	
	return takers.size();
}

/**
 * Getter del numero di volte per cui il lato è stato preso.
 *
 * @return	il numero di volte per cui il lato è stato preso, 0 se ciò non è mai successo.
 */
unsigned long MetaEdge::getTaken() const
{
	return takers.size();
}

/**
 * Comparatore del veicolo in input col veicolo servente il lato
 *
 * @return	vero, se il veicolo in input serve il lato
 */
bool MetaEdge::isServer( const Vehicle* aVehicle ) const
{
	return *( takers.front() ) == *aVehicle;
}

/**
 * Getter del veicolo servente il lato
 *
 * @return	il veicolo servente il lato
 */
const Vehicle* MetaEdge::getServer() const
{
	return takers.front();
}

bool MetaEdge::setServer( const Vehicle* vehicle )
{
	// Per impostare un veicolo come servitore, devo spostarlo in testa alla lista dei takers.
	// Per prima cosa cerco il vehicle richiesto
	for( int i = 0; i < takers.size(); i++ )
		if( takers[ i ] == vehicle )
		{
			// Inserisco il corrente elemento in testa al vettore
			takers.insert( takers.begin(), takers[ i ] );
			// Elimino l'elemento stesso dalla posizione in cui si trovava in precedenza,
			// tenendo conto che ora l'array ha un elemento in più
			takers.erase( takers.begin() + i + 1 );
			// Sono riuscito a scambiare il server
			return true;
		}

	return false;
}

vector<const Vehicle*> MetaEdge::getTakers() const
{
	return takers;
}

Edge* MetaEdge::getEdge() const
{
	return actualEdge;
}

/**
 * Operatore di confronto tra lati
 */
bool MetaEdge::operator ==( MetaEdge& other ) const
{
	return this->equals( other );
}
bool MetaEdge::operator !=( MetaEdge& other ) const
{
	return !( *this == other );
}
bool MetaEdge::equals( const MetaEdge& other ) const
{
	return actualEdge == other.actualEdge;
}

/*** MetaGraph ***/

/**
 * Costruttore
 */
MetaGraph::MetaGraph( Graph g )
{
	this->edges = unordered_map<Edge*, MetaEdge*>();
	
	for ( Edge* edge : g.getEdges() )
		edges.insert( make_pair( edge, new MetaEdge( edge ) ) );
}

// Costruttore strambo
MetaGraph::MetaGraph( const MetaGraph& source )
{
	// Creo la nuova mappa lati-metalati
	this->edges = unordered_map<Edge*, MetaEdge*>();

	// Ciclo su tutti i metalati del metagrafo e ne faccio una copia
	for( auto edge : source.edges )
	{
		edges.insert( make_pair( edge.first, new MetaEdge( *edge.second ) ) );
	}
}

/**
 * Distruttore
 */
MetaGraph::~MetaGraph()
{
//	// Cancello tutti i miei MetaLati
//	for ( auto edge : edges )
//		delete edge.second;
}

// Getter della corrispondenza lato reale - metalato
MetaEdge* MetaGraph::getEdge( const Edge* edge ) const
{
	// edge deve essere const per l'uso che viene fatto della funzione (greedyCompare etc),
	// ma non può esserlo per unordered_map => const_cast
	return edges.at( const_cast<Edge*>( edge ) );
}

