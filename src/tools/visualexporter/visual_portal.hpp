/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_PORTAL_HPP
#define VISUAL_PORTAL_HPP

#include <vector>
#include "math/planeeq.hpp"
#include "math/boundbox.hpp"
#include "resmgr/datasection.hpp"

/**
 *	This class is a portal in a visual
 */
class VisualPortal : public ReferenceCount
{
public:
	VisualPortal();
	~VisualPortal();

	//this method adds a point to the portal,
	//swapping the y,z elements.
	void addPoint( const Vector3 & pt );
	//this method adds a point to the portal,
	//but does not swap the y,z elements.
	void addSwizzledPoint( const Vector3 & pt );
	void name( const std::string & name );
	void save( DataSectionPtr pInSect );
	void planeEquation( PlaneEq& result );
	bool cull( const BoundingBox& bb, bool& culled );
	bool cullHull( const std::vector<PlaneEq>& boundaries, bool& culled );
	void reverse();

	void nameFromProps( class INode* n );
	const char* propDataFromPropName( const char* propName );

	//This helper method takes all the points in the pts_
	//container, and sorts them into a convex hull, based
	//on the plane equation gleened from the first 3 points.
	void createConvexHull();

private:
	VisualPortal( const VisualPortal& );
	VisualPortal& operator=( const VisualPortal& );

	

	std::vector<Vector3>	pts_;
	std::string				name_;
	std::string				label_;
};


typedef SmartPointer<VisualPortal>	VisualPortalPtr;

#endif // VISUAL_PORTAL_HPP
