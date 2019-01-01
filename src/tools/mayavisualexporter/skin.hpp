/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __skin_hpp__
#define __skin_hpp__

#include "bonevertex.hpp"
#include "matrix4.hpp"

class Skin
{
public:
	Skin();
	~Skin();

	// returns the number of skins
	uint32 count();

	// initialises Skin for skin "index" - access functions now use this
	// skin cluster. this must be called before accessing the vertex weights
	bool initialise( uint32 index );

	// frees up memory used by the initialised skin
	void finalise();

	// returns the name of the currently initialised skin cluster
	std::string name();

	// returns dag paths of influenced meshes
	MDagPathArray& meshes();

	// access functions
	uint32 numberOfBones();
	
	// returns number of vertices for geometry object path (0 if not influenced by skin)
	uint32 numberOfVertices( const std::string& path );

	// returns number of vertex data for geometry object path (empty std::vector if not influenced by skin)
	std::vector<BoneVertex>& vertices( const std::string& path );
	
	// returns a std::vector of object paths influenced by the skin
	std::vector<std::string> paths();

	// returns the bone names (full dag name or partial dag name)
	std::string boneNameFull( uint32 bone );
	std::string boneNamePartial( uint32 bone );
	
	// returns the MDagPath for bone
	MDagPath boneDAG( uint32 bone );
	
	// returns the BoneSet for bone
	BoneSet& boneSet( std::string mesh );
	
	// returns the initial transform for bone index bone
	matrix4<float> transform( uint32 bone, bool relative = false );
	
	// returns the initial transform for the bone boneName (where boneName is the full bone name)
	matrix4<float> transform( const std::string& boneName, bool relative = false );

	// this method removes skin bits that do not influence any of the meshes in the mDagPathArray
	void trim( MDagPathArray& meshes );
	
protected:
	// name of currently initialised skin cluster
	std::string _name;

	// std::vector of skin cluster MObjects in the maya scene
	std::vector<MObject> _skins;

	// maps joint paths to bone indexes, refers to *current* initialised skin only
	std::map<std::string, uint32> _boneIndexes;
	std::map<uint32, MDagPath> _bonePaths;

	// dag paths to influenced meshes
	MDagPathArray _meshes;

	// bone sets for each mesh - same index should be used as the _meshes array
	std::vector<BoneSet> _boneSets;

	// number of bones influencing current skin
	uint32 _bones;

	// blended vertex data for each influenced geometry object
	std::map< std::string, std::vector<BoneVertex> > _vertices;

	// initial bone transforms, to get the path of the object at an index, use the
	// same index into _bonePaths
	std::vector< matrix4<float> > _transforms;

	// in case we want relative transforms
	std::vector< matrix4<float> > _relativeTransforms;
	
	// empty bone vertex std::vector
	static std::vector<BoneVertex> _empty;
};

#endif // __skin_hpp__