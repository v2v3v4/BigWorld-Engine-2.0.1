/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_DYNAMIC_OBSTACLE_HPP
#define CHUNK_DYNAMIC_OBSTACLE_HPP

#include "chunk_embodiment.hpp"

class PyModelObstacle;


/*~ class BigWorld.PyModelObstacle
 *
 *	The PyModelObstacle object is a Model that is integrated into the collision scene.  They have no motors, 
 *	attachments or fashions, like a PyModel and so are driven by a single MatrixProvider, which is often linked to 
 *	the Entity that will use the PyModelObstacle.  the PyModelObstacle can be dynamic and move around the world, 
 *	still engaged in the collision scene, or static, remaining in a single location.  In the latter case, it makes 
 *	sense for the owner Entity to be stationary in the world, otherwise the PyModelObstacle and Entity position 
 *	would be dispersed, which is undesirable.  The PyModelObstacle should be assigned to the self.model attribute 
 *	of an Entity to become part of the world and once this occurs, the dynamic or static state of the 
 *	PyModelObstacle is fixed and cannot be changed.  PyModelObstacles are more expensive that standard PyModels, 
 *	hence should only be used when completely necessary.
 *
 *	PyModelObstacles are constructed using BigWorld.PyModelObstacle function.
 */
/**
 *	This class is a dynamic obstacle in a chunk. It moves around every
 *	frame and you can collide with it.
 */
class ChunkDynamicObstacle : public ChunkDynamicEmbodiment
{
public:
	ChunkDynamicObstacle( PyModelObstacle * pObstacle );
	virtual ~ChunkDynamicObstacle();

	virtual void tick( float dTime );
	virtual void draw();
	virtual void toss( Chunk * pChunk );

	virtual void enterSpace( ChunkSpacePtr pSpace, bool transient );
	virtual void leaveSpace( bool transient );
	virtual void move( float dTime );

	virtual const Matrix & worldTransform() const;
	virtual void localBoundingBox( BoundingBox & bb, bool skinny = false ) const;
	virtual void worldBoundingBox( BoundingBox & bb, const Matrix& world, bool skinny = false ) const;

	PyModelObstacle * pObstacle() const;

	static int convert( PyObject * pObj, ChunkEmbodimentPtr & pNew,
		const char * varName );

private:
	ChunkDynamicObstacle( const ChunkDynamicObstacle& );
	ChunkDynamicObstacle& operator=( const ChunkDynamicObstacle& );
};

typedef SmartPointer<ChunkDynamicObstacle> ChunkDynamicObstaclePtr;


/**
 *	This class is a static obstacle in a chunk. It's still a little bit
 *	dynamic because scripts create and delete these, but it doesn't move
 *	around every frame like ChunkDynamicObstacle does.
 */
class ChunkStaticObstacle : public ChunkStaticEmbodiment, public Aligned
{
public:
	ChunkStaticObstacle( PyModelObstacle * pObst );
	virtual ~ChunkStaticObstacle();

	virtual void tick( float dTime );
	virtual void draw();
	virtual void toss( Chunk * pChunk );
	virtual void lend( Chunk * pLender );

	virtual void enterSpace( ChunkSpacePtr pSpace, bool transient );
	virtual void leaveSpace( bool transient );

	virtual const Matrix & worldTransform() const;
	virtual void localBoundingBox( BoundingBox & bb, bool skinny = false ) const;
	virtual void worldBoundingBox( BoundingBox & bb, const Matrix& world, bool skinny = false ) const;

	PyModelObstacle * pObstacle() const;

	static int convert( PyObject * pObj, ChunkEmbodimentPtr & pNew,
		const char * varName );

private:
	ChunkStaticObstacle( const ChunkStaticObstacle& );
	ChunkStaticObstacle& operator=( const ChunkStaticObstacle& );

	Matrix	worldTransform_;
};


class SuperModel;
class ChunkObstacle;
typedef SmartPointer<ChunkObstacle> ChunkObstaclePtr;
#include "pyscript/script_math.hpp"
#include "math/boundbox.hpp"
class PyDye;

/**
 *	This class is the python representation of an obstacle made out of a model.
 *	It can be either static or dynamic.
 */
class PyModelObstacle : public PyObjectPlus
{
	Py_Header( PyModelObstacle, PyObjectPlus )

public:
	PyModelObstacle( SuperModel * pSuperModel, MatrixProviderPtr pMatrix = NULL,
		bool dynamic = false, PyTypePlus * pType = &s_type_ );
	virtual ~PyModelObstacle();

	void attach();
	void detach();

	void enterWorld( ChunkItem * pItem );
	void leaveWorld();

	void tick( float dTime );
	void draw( const Matrix & worldTransform );

	const Matrix & worldTransform() const;

	void localBoundingBox( BoundingBox& bb, bool skinny = false ) const;
	void worldBoundingBox( BoundingBox& bb, const Matrix& world, bool skinny = false ) const;

	bool attached() const							{ return attached_; }
	bool dynamic() const							{ return dynamic_; }

	typedef std::vector<ChunkObstaclePtr> Obstacles;
	Obstacles & obstacles()							{ return obstacles_; }

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PyObject * pyGet_sources();
	PY_RO_ATTRIBUTE_SET( sources )

	PY_RW_ATTRIBUTE_DECLARE( pMatrix_, matrix );

	PY_READABLE_ATTRIBUTE_GET( dynamic_, dynamic );
	int pySet_dynamic( PyObject * val );

	PyObject * pyGet_bounds();
	PY_RO_ATTRIBUTE_SET( bounds )

	PY_RO_ATTRIBUTE_DECLARE( attached_, attached )

	PY_RW_ATTRIBUTE_DECLARE( vehicleID_, vehicleID )

	PyObject * node( const std::string & nodeName );
	PY_AUTO_METHOD_DECLARE( RETOWN, node, ARG( std::string, END ) );

	PY_FACTORY_DECLARE()

private:
	SuperModel			* pSuperModel_;
	MatrixProviderPtr	pMatrix_;
	bool				dynamic_;
	bool				attached_;
	uint32				vehicleID_;

	BoundingBox			localBoundingBox_;
	Obstacles			obstacles_;

	typedef std::map< std::string, SmartPointer<PyDye> > Dyes;
	Dyes				dyes_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyModelObstacle )


inline PyModelObstacle * ChunkDynamicObstacle::pObstacle() const
	{ return (PyModelObstacle*)&*pPyObject_; }

inline PyModelObstacle * ChunkStaticObstacle::pObstacle() const
	{ return (PyModelObstacle*)&*pPyObject_; }


#endif // CHUNK_DYNAMIC_OBSTACLE_HPP
