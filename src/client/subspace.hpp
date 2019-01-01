/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SUBSPACE_HPP
#define SUBSPACE_HPP

#include "moo/moo_math.hpp"

class SceneRoot;
class CollisionScene;
class PyModel;
class ChunkSpace;

/**
 *	This class maintains a scene graph and a collision scene.
 *
 *	Its scene root is the endpoint of world transform calculations,
 *	even though the subspace itself may be overlaid onto the world
 *	(in which case the camera and global lights are transformed
 *	down into its co-ordinate system before drawing).
 *
 *	This class is not intended to survive the transition to full chunking.
 */
class SubSpace : public Aligned
{
public:
	SubSpace( SceneRoot * pScene, CollisionScene * pObstacles,
		ChunkSpace * pSpace );
	~SubSpace();

	ChunkSpace & space()						{ return *pSpace_; }

	const Matrix & transform()					{ return transform_; }
	const Matrix & transformInverse()			{ return transformInverse_; }
	void transform( const Matrix & t )
	{
		transform_ = t;
		transformInverse_.invert( t );
		transformNonIdentity_ = true;
	}

	void tick( float dTime );
	void draw();

	PyModel * pEngineModel();

private:
	SceneRoot		* pScene_;
	CollisionScene	* pObstacles_;
	ChunkSpace		* pSpace_;

	Matrix		transform_;
	Matrix		transformInverse_;
	bool			transformNonIdentity_;

	PyModel			* pEngineModel_;
};

#endif // SUBSPACE_HPP
