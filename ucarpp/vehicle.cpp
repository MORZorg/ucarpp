//
//  vehicle.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "vehicle.h"

#ifndef DEBUG
//#define DEBUG
#endif

using namespace std;
using namespace solver;
using namespace model;


/*** Vehicle ***/

Vehicle::Vehicle( int _id ): id( _id )
{
	path = list<MetaEdge*>();
}

//Vehicle::Vehicle( const Vehicle& source ): id( source.id )
//{
//	path = source.path;
//}

MetaEdge* Vehicle::getEdge( int index ) const
{
	return *next( path.begin(), index );
}

void Vehicle::addEdge( MetaEdge* edge, long index )
{
	list <MetaEdge*>::iterator it;
	if( index == -1 )
		it = path.end();
	else
		it = next( path.begin(), index );

	// Controllo in che punto della lista devo inserire il passaggio del veicolo nel metalato 
	int occurence = 0;
	for( auto i = path.begin(); i != it; ++i )
		if( **i == *edge )
			occurence++;

	edge->setTaken( this, occurence );
	
	if ( index == - 1 || index == path.size() )
		path.push_back( edge );
	else
		path.insert( it, edge );
}

void Vehicle::removeEdge( long index )
{
	// Rimuovo l'ultimo lato
	if ( index == -1 )
		index = path.size() -1;
	
	int occurrence = 0;
	// Creo l'iteratore alla posizione richiesta e conto le occorrenze del lato da rimuovere
	list<MetaEdge*>::iterator it = next( path.begin(), index );
	for ( auto i = path.begin(); i != it; ++i )
		if ( **i == **it )
			occurrence++;
	
	(*it)->unsetTaken( this, occurrence );
	path.erase( it );
}

unsigned long Vehicle::size() const
{
	return path.size();
}

uint Vehicle::getCost() const
{
	uint result = 0;
	for ( auto it = path.begin(); it != path.end(); it++ )
		result += (*it)->getCost();
	
	return result;
}

uint Vehicle::getDemand() const
{
	uint result = 0;
	for ( auto it = path.begin(); it != path.end(); it++ )
	{
		auto jt = path.begin();
		while ( **(jt++) != **it );
		jt--;
		
		result += (*it)->getDemand() * ( (*it)->isServer( this ) && jt == it );
	}
	
	return result;
}

uint Vehicle::getProfit() const
{
	uint result = 0;
	for ( auto it = path.begin(); it != path.end(); it++ )
	{
		auto jt = path.begin();
		while ( **(jt++) != **it );
		jt--;
		
//#ifdef DEBUG
//		cerr << "Lato: " << (*it)->getSrc() << " " << (*it)->getDst() << endl;
//		cerr << "Profitto: " << (*it)->getProfit() << endl;
//		cerr << "Servente: " << (*it)->isServer( this ) << endl;
//		cerr << "Uguale: " << ( ( jt == it ) ? "vero" : "falso" ) << endl;
//#endif
		result += (*it)->getProfit() * ( (*it)->isServer( this ) && jt == it );
	}
	
	return result;
}

// true se la direzione di percorrenza è da src a dst, false altrimenti
bool Vehicle::getDirection( int edge ) const
{
	auto desired = next( path.begin(), edge );
	uint previous;
	auto it = path.begin();
	if ( it == path.end() )
		throw 404;
	previous = (*it)->getSrc();
		
	for ( ; it != desired; it++ )
		previous = (*it)->getDst( previous );
	
	return previous == (*it)->getSrc();
}

string Vehicle::toString() const
{
	stringstream ss;
	auto it = path.begin();
	if ( it == path.end() )
		return "";
#ifndef DEBUG
	uint previous = (*it)->getSrc();
#endif

	for ( ; it != path.end(); it++ )
	{
#ifndef DEBUG
		ss << "(" << previous + 1 << " ";
		previous = (*it)->getDst( previous );
		ss << previous + 1 << ") ";
#else
		ss << "(" << (*it)->getSrc() << " " << (*it)->getDst() << ") ";
#endif
	}
	ss << " Profitto: " << getProfit() << endl;
	
	return ss.str();
}

// Ritorna l'identificativo del veicolo
int Vehicle::getId() const
{
	return id;
}

/**
 * Comparatore di uguaglianza tra veicoli
 */
bool Vehicle::operator ==( const Vehicle& other ) const
{
	return this->equals( other );
}

bool Vehicle::operator !=( const Vehicle& other ) const
{
	return !(this->equals( other ) );
}

bool Vehicle::equals( const Vehicle& other ) const
{
	return getId() == other.getId();
}
