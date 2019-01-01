/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_GEOMETRY_MAPPINGS_HPP
#define SERVER_GEOMETRY_MAPPINGS_HPP

#include "chunk/geometry_mapping.hpp"
#include "math/rectt.hpp"
#include "network/basictypes.hpp"

#include <set>

class ServerGeometryMapping;
class Space;


/**
 *	This class maintains a collection of ServerGeometryMapping instances for a
 *	single space.
 */
class ServerGeometryMappings : public GeometryMappingFactory
{
public:
	ServerGeometryMappings( const Space & space );
	~ServerGeometryMappings();

	// Override form GeometryMappingFactory
	virtual GeometryMapping * createMapping( ChunkSpacePtr pSpace,
			const Matrix & m, const std::string & path,
			DataSectionPtr pSettings );

	void removeMapping( ServerGeometryMapping * pMapping );

	bool tickLoading( const BW::Rect & cellRect, bool unloadOnly,
			SpaceID spaceID );
	void prepareNewlyLoadedChunksForDelete();
	void calcLoadedRect( BW::Rect & loadedRect ) const;

	bool isFullyUnloaded() const;

private:
	typedef std::set< ServerGeometryMapping * > Mappings;

	Mappings mappings_;
	const Space & space_;
};

#endif // SERVER_GEOMETRY_MAPPINGS_HPP
