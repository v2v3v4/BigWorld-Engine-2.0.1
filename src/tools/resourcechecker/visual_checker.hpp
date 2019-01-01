/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VISUAL_CHECKER_HPP
#define VISUAL_CHECKER_HPP

#include "resmgr/datasection.hpp"

#include <string>
#include <set>

#define UNKNOWN_TYPE_NAME ( "(unknown)" )
/**
 *	Ensures that a given visual passes a series of validity checks
 */
class VisualChecker
{
private:
	Vector3 minSize_;
	Vector3 maxSize_;
	uint32 maxTriangles_;
	uint32 minTriangles_;
	uint32 maxHierarchyDepth_;
	bool snapVertices_;
	bool portals_;
	Vector3 portalSnap_;
	float portalDistance_;
	float portalOffset_;
	bool checkHardPoints_;
	std::set<std::string> hardPoints_;

	std::string typeName_;
	std::string exportAs_;

	std::vector<std::string> errors_;

	bool checkTriangleCount( DataSectionPtr spPrims );
	bool checkMorphTargets( DataSectionPtr spPrims, const std::set<std::string>& nodeNames );
	void addError( const char * format, ... );
public:

	/** Construct a VisualChecker with setting appropriate for the given visual */
	VisualChecker( const std::string& visualName, bool cacheRules = true, bool snapVertices = true );

	/** Check the visual for errors according to our current rules */
	bool check( DataSectionPtr visualSection, const std::string& primResName );

	/** If check() failed, get the errors that occured, one per line */
	std::string errorText();

	/** The type of visual we have determined from the file name */
	std::string typeName() const { return typeName_; }

	/** The recommended type (static, static with nodes, normal) to export the visual as */
	std::string exportAs() const { return exportAs_; }

	/** Sets the snap vertices flag. */
	void snapVertices( bool snapVertices ) { snapVertices_ = snapVertices; }
};


#endif // VISUAL_CHECKER_HPP
