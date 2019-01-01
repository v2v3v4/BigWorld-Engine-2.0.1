/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ORIENTED_BBOX_HPP
#define ORIENTED_BBOX_HPP


#include "polyhedron.hpp"


class BoundingBox;
class Matrix;


namespace Math
{

/**
 *  This class implements an oriented bounding box. At the moment, this class
 *  is only useful for doing intersection tests with oriented bounding boxes.
 *  @see Polyhedron
 */
class OrientedBBox : public Polyhedron
{
public:
	OrientedBBox();

	OrientedBBox( const BoundingBox& bb, const Matrix& matrix );

	void create( const BoundingBox& bb, const Matrix& matrix );

	unsigned int numPoints() const;

	unsigned int numFaces() const;

	unsigned int numEdges() const;

	Vector3 point( unsigned int i ) const;

	Face face( unsigned int i ) const;

	Edge edge( unsigned int i ) const;

private:
	std::vector<Vector3> points_;
	std::vector<Face> faces_;
	std::vector<Edge> edges_;
};

} // namespace Math


#endif //ORIENTED_BBOX_HPP
