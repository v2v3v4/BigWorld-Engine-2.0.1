/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef AOI_UPDATE_SCHEMES_HPP
#define AOI_UPDATE_SCHEMES_HPP

#include "cstdmf/stdmf.hpp"

#include <map>
#include <string>

typedef uint8 AoIUpdateSchemeID;


/**
 *	This class stores an AoI update scheme that can be used by EntityCache.
 */
class AoIUpdateScheme
{
public:
	AoIUpdateScheme();
	bool init( float minDelta, float maxDelta );

	double apply( float distance ) const
	{
		return (distance * distanceWeighting_ + 1.f) * weighting_;
	}

private:
	float weighting_;
	float distanceWeighting_;
};


/**
 *	This class stores the AoI schemes that can be used by EntityCache.
 */
class AoIUpdateSchemes
{
public:
	typedef AoIUpdateSchemeID SchemeID;

	static double apply( SchemeID scheme, float distance )
	{
		return schemes_[ scheme ].apply( distance );
	}

	static bool init();

	static bool getNameFromID( SchemeID id, std::string & name );
	static bool getIDFromName( const std::string & name, SchemeID & rID );

private:
	// Not using std::vector just to remove one dereference.
	static AoIUpdateScheme schemes_[ 256 ];

	typedef std::map< std::string, SchemeID > NameToSchemeMap;
	static NameToSchemeMap nameToScheme_;

	typedef std::map< SchemeID, std::string > SchemeToNameMap;
	static SchemeToNameMap schemeToName_;
};

#endif // AOI_UPDATE_SCHEMES_HPP
