/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __hierarchy_hpp__
#define __hierarchy_hpp__

#include "matrix.hpp"
#include "mesh.hpp"

class Hierarchy
{
public:
	Hierarchy( Hierarchy* parent );
	~Hierarchy();

	void clear();

	void getSkeleton( Skin& skin );
	void getMeshes( Mesh& mesh );

	void removeNode( const std::string& name );
	std::string removeNode( Hierarchy* node );
	void addNode( const std::string& name, Hierarchy& h );
	Hierarchy* recursiveFind( const std::string& name );
	Hierarchy* getParent() const { return parent_; }


	void addNode( const std::string& path, MDagPath dag, std::string accumulatedPath = "" );

	// this is for adding a child only 1 level below (for fixup nodes)
	void addNode( const std::string& path, matrix4<float> worldTransform );

	void dump( const std::string& parent = "" );

	void customName(std::string customName);
	void name(std::string name);
	std::string name();
	std::string path();
	std::string dag();
	std::vector<std::string> children( bool orderChildren = false );
	Hierarchy& child( const std::string& name );

	Hierarchy& find( const std::string& path );

	uint32 numberOfFrames();

	// total number of objects in hierarchy, exluding the root node
	uint32 count();

	matrix4<float> transform( double frame, bool relative = false );

protected:
	std::string _name;

	std::string _customPath; // for manually added nodes, eg fixup nodes
	matrix4<float> _relativeTransform; // for manually added nodes
	matrix4<float> _worldTransform; // for manually added nodes

	MDagPath _path;

	std::map<std::string, Hierarchy*> _children;
	Hierarchy* parent_;
};

bool numChildDescending(
	std::pair< std::string, Hierarchy* > lhs,
	std::pair< std::string, Hierarchy* > rhs );

#endif // __hierarchy_hpp__