//
//  solution.cpp
//  ucarpp
//
//  Created by Maurizio Zucchelli on 2013-04-13.
//  Copyright (c) 2013 Marco Maddiona, Riccardo Orizio, Mattia Rizzini, Maurizio Zucchelli. All rights reserved.
//

#include "solution.h"

#ifndef DEBUG
//#define DEBUG
#endif

using namespace std;
using namespace solver;
using namespace model;


/*** Solution ***/


Solution::Solution( int M, Graph graph ):
	M( M ), graph( graph ), compareRatioGreedy( &this->graph )
{
////	vehicles = (Vehicle*)calloc( M, sizeof( Vehicle ) );
//	vehicles = static_cast<Vehicle*> (::operator new ( sizeof( Vehicle ) * M ) );
//	for ( int i = 0; i < M; i++ )
//		new ( &vehicles[ i ] ) Vehicle( i );
	vehicles = vector<Vehicle*>();
	for ( int i = 0; i < M; i++ )
		vehicles.push_back( new Vehicle( i ) );
}

Solution::Solution( const Solution& source ):
M( source.M ), graph( source.graph ), compareRatioGreedy( &this->graph )
{
	vehicles = vector<Vehicle*>();
	for ( int i = 0; i < M; i++ )
//		vehicles.push_back( source.vehicles[ i ] );
	{
		vehicles.push_back( new Vehicle( i ) );
		for ( int j = 0; j < source.vehicles[ i ]->size(); j++ )
		{
			Edge* e = source.getEdge( i, j )->getEdge();
			vehicles[ i ]->addEdge( graph.getEdge( e ) );
		}
	}
}

Solution::~Solution()
{
//	// Cancello tutti i miei veicoli
//	for ( Vehicle* aVehicle : vehicles )
//		delete aVehicle;
}

bool Solution::operator>( const Solution& other ) const
{
	return getProfit() > other.getProfit() ||
			( getProfit() == other.getProfit() &&
			  ( getDemand() < other.getDemand() ||
				( getDemand() == other.getDemand() &&
				  getCost() < other.getCost() ) ) );
}

MetaEdge* Solution::getEdge( int vehicle, int index ) const
{
	return vehicles[ vehicle ]->getEdge( index );
}

void Solution::addEdge( Edge* edge, int vehicle, int index )
{
	vehicles[ vehicle ]->addEdge( graph.getEdge( edge ), index );
}

void Solution::removeEdge( int vehicle, int index )
{
	vehicles[ vehicle ]->removeEdge( index );
}

unsigned long Solution::size() const
{
	unsigned long result = 0;
	for ( int i = 0; i < M; i++ )
		result += size( i );
	
	return result;
}

unsigned long Solution::size( int vehicle ) const
{
	return vehicles[ vehicle ]->size();
}

uint Solution::getProfit() const
{
	uint result = 0;
	for ( int i = 0; i < M; i++ )
		result += getProfit( i );
	
	return result;
}

uint Solution::getProfit( int vehicle ) const
{
	return vehicles[ vehicle ]->getProfit();
}

uint Solution::getCost() const
{
	uint result = 0;
	for ( int i = 0; i < M; i++ )
		result += getCost( i );
	
	return result;
}

uint Solution::getCost( int vehicle ) const
{
	return vehicles[ vehicle ]->getCost();
}

uint Solution::getDemand() const
{
	uint result = 0;
	for ( int i = 0; i < M; i++ )
		result += getDemand( i );
	
	return result;
}

uint Solution::getDemand( int vehicle ) const
{
	return vehicles[ vehicle ]->getDemand();
}

bool Solution::getDirection( int vehicle, int index ) const
{
	return vehicles[ vehicle ]->getDirection( index );
}

string Solution::toString() const
{
	stringstream ss;
	for ( int i = 0; i < M; i++ )
		ss << i + 1 << ":\t" << toString( i ) << endl;
	
	return ss.str();
}

string Solution::toString( int vehicle ) const
{
	return vehicles[ vehicle ]->toString();
}
