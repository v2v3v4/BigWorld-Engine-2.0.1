/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __mesh_hpp__
#define __mesh_hpp__

#include "vertex.hpp"
#include "face.hpp"
#include "material.hpp"

class Mesh
{
public:
	Mesh();
	~Mesh();

	// returns the number of meshes
	uint32 count();

	// initialises Mesh for mesh "index" - access functions now use this
	// mesh. this must be called before accessing vertex data
	bool initialise( uint32 index, bool objectSpace = true );

	// frees up memory used by the mesh
	void finalise();

	MDagPathArray& meshes();

	std::string name();
	std::string nodeName();
	std::string fullName();

	std::vector<Point3>& positions();
	std::vector<Point3>& normals();
	std::vector<Point4>& colours();
	std::vector<Point2>& uvs( uint32 set );
	uint32 uvSetCount() const;
	std::vector<Face>& faces();

	std::vector<material>& materials();
	
	matrix4<float> transform();

protected:
	// dag paths to meshes
	MDagPathArray _meshes;

	// node transform
	matrix4<float> _transform;

	// geometry data
	std::string _name;
	std::string _nodeName;
	std::string _fullName;
	std::vector<Point3> _positions;
	std::vector<Point3> _normals;
	std::vector<Point4> _colours;
	std::vector<std::vector<Point2>> _uvs;
	std::vector<Face> _faces;
	std::vector<material> _materials;
};

#endif // __mesh_hpp__