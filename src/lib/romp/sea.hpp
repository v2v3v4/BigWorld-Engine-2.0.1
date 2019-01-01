/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SEA_HPP
#define SEA_HPP

#include <vector>
#include "cstdmf/smartpointer.hpp"

typedef SmartPointer<class DataSection> DataSectionPtr;

/**
 *	This class is a big flat sea.
 */
class Sea : public ReferenceCount
{
public:
	Sea();

	void load( DataSectionPtr pSect );
	void save( DataSectionPtr pSect );

	void draw( float dTime, float timeOfDay );

	float seaLevel() const		{ return seaLevel_; }
	void seaLevel( float f )	{ seaLevel_ = f; }

	float wavePeriod() const	{ return wavePeriod_; }
	void wavePeriod( float f )	{ wavePeriod_ = max(f,0.05f); }

	float waveExtent() const	{ return waveExtent_; }
	void waveExtent( float f )	{ waveExtent_ = f; }

	float tidePeriod() const	{ return tidePeriod_; }
	void tidePeriod( float f )	{ tidePeriod_ = max(f,0.05f); }

	float tideExtent() const	{ return tideExtent_; }
	void tideExtent( float f )	{ tideExtent_ = f; }

	uint32 surfaceTopColour() const		{ return surfaceTopColour_; }
	void surfaceTopColour( uint32 i )	{ surfaceTopColour_ = i; }

	uint32 surfaceBotColour() const		{ return surfaceBotColour_; }
	void surfaceBotColour( uint32 i )	{ surfaceBotColour_ = i; }

	uint32 underwaterColour() const		{ return underwaterColour_; }
	void underwaterColour( uint32 i )	{ underwaterColour_ = i; }

private:
	float	seaLevel_;
	float	wavePeriod_;
	float	waveExtent_;
	float	tidePeriod_;
	float	tideExtent_;

	uint32	surfaceTopColour_;
	uint32	surfaceBotColour_;
	uint32	underwaterColour_;

	float	waveTime_;
};


/**
 *	This class manages a collection of seas.
 */
class Seas : public std::vector< SmartPointer< Sea > >
{
public:
	void draw( float dTime, float timeOfDay )
	{
		for (iterator i = begin(); i != end(); i++)
			(*i)->draw( dTime, timeOfDay );
	}
};

#endif // SEA_HPP