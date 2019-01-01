/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_PORTAL_HPP
#define EDITOR_CHUNK_PORTAL_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "chunk/chunk.hpp"
#include "chunk/chunk_item.hpp"
#include "chunk/chunk_boundary.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include <map>
#include <string>


namespace Script
{
	PyObject * getData( const Chunk * pChunk );
};


/**
 *	This class is the chunk item created to represent a portal when
 *	it has a special name and can thus be referenced (and fiddled with)
 *	by scripts.
 */
class EditorChunkPortal : public PyObjectPlus, public ChunkItem
{
	Py_Header( EditorChunkPortal, PyObjectPlus )

public:
	EditorChunkPortal( ChunkBoundary::Portal * pPortal,
		PyTypePlus * pType = & s_type_ );
	~EditorChunkPortal();

	virtual void edPostClone( EditorChunkItem* srcItem ) {}
	virtual void edPostCreate()  {}
	virtual void edPostModify() {}
	// Python stuff
	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_RO_ATTRIBUTE_DECLARE( pChunk_, home )
	PY_RW_ATTRIBUTE_DECLARE( triFlags_, triFlags )

	PY_RO_ATTRIBUTE_DECLARE( pPortal_->internal, internal )
	PY_RW_ATTRIBUTE_DECLARE( pPortal_->permissive, permissive )
	PY_RO_ATTRIBUTE_DECLARE( pPortal_->pChunk, chunk )

	PyObject * pyGet_points();
	PY_RO_ATTRIBUTE_SET( points )

	PY_RO_ATTRIBUTE_DECLARE( pPortal_->uAxis, uAxis )
	PY_RO_ATTRIBUTE_DECLARE( pPortal_->vAxis, vAxis )
	PY_RO_ATTRIBUTE_DECLARE( pPortal_->origin, origin )
	PY_RO_ATTRIBUTE_DECLARE( pPortal_->lcentre, lcentre )
	PY_RO_ATTRIBUTE_DECLARE( pPortal_->centre, centre )
	PY_RO_ATTRIBUTE_DECLARE( pPortal_->plane.normal(), plane_n )
	PY_RO_ATTRIBUTE_DECLARE( pPortal_->plane.d(), plane_d )
	PY_RO_ATTRIBUTE_DECLARE( pPortal_->label, label )


	// ChunkItem overrides
	virtual void toss( Chunk * pChunk );
	virtual bool edShouldDraw();
	virtual bool isPortal() const				{ return true; }
	virtual void draw();	// for debugging

	virtual void incRef() const		{ this->PyObjectPlus::incRef(); }
	virtual void decRef() const		{ this->PyObjectPlus::decRef(); }
	virtual int refCount() const	{ return this->PyObjectPlus::refCount(); }

	virtual const char * edClassName()		{ return "Portal"; }
	virtual bool edEdit( class GeneralEditor & editor );
	virtual void edBounds( BoundingBox& bbRet ) const;
	virtual const Matrix & edTransform();
	virtual DataSectionPtr pOwnSect();
	virtual const DataSectionPtr pOwnSect()	const;

	virtual bool edSave( DataSectionPtr pSection );

	virtual bool edIsSnappable() { return false; }

	virtual void syncInit();

private:
	EditorChunkPortal( const EditorChunkPortal& );
	EditorChunkPortal& operator=( const EditorChunkPortal& );

	std::string getExtern() const;
	bool getInvasive() const;

	std::string getLabel() const;
	bool setLabel( const std::string & v );

	std::string getX() const;
	std::string getY() const;
	std::string getZ() const;

	std::string getNormal() const;
	std::string getD() const;
	std::string getUAxis() const;
	std::string getLocalCentre() const;

	Chunk * otherChunk() const;

	ChunkBoundary::Portal * pPortal_;
	uint32					triFlags_;
	Matrix					xform_;

	static uint32	s_settingsMark_;
	static bool		s_drawAlways_;

	Moo::EffectMaterialPtr portalMat_;
	Moo::EffectMaterialPtr portalSelectMat_;

	friend class EditorPortalObstacle;
};

typedef SmartPointer<EditorChunkPortal> EditorChunkPortalPtr;


/**
 *	This class keeps track of all python-accessible objects in a chunk.
 *	It also takes care of creating the ChunkPortal items when a chunk
 *	loads (actually when it binds, for threading reasons)
 */
class ChunkPyCache : public ChunkCache
{
	typedef ChunkPyCache This;

public:
	ChunkPyCache( Chunk & chunk );
	~ChunkPyCache();

	void createPortalsAgain() { bound_ = false; }

	void add( const std::string & name, PyObject * pObject );
	void del( const std::string & name );

	SmartPointer<PyObject> get( const std::string & name );

	static PyObject * chunkInhabitant( const std::string & label,
		const std::string & chunk, const std::string & space );
	PY_AUTO_MODULE_STATIC_METHOD_DECLARE( RETOWN, chunkInhabitant,
		ARG( std::string, ARG( std::string, OPTARG( std::string, "", END ) ) ) )

	static PyObject * findChunkFromPoint( const Vector3 & point,
		const std::string & space );
	PY_AUTO_MODULE_STATIC_METHOD_DECLARE( RETOWN, findChunkFromPoint,
		ARG( Vector3, OPTARG( std::string, "", END ) ) )

	virtual void bind( bool isUnbind );
	static void touch( Chunk & chunk );

	static Instance<ChunkPyCache>	instance;

	typedef std::map< std::string, SmartPointer<PyObject> > NamedPyObjects;

private:
	Chunk & chunk_;
	NamedPyObjects	exposed_;
	bool bound_;
};


#endif // EDITOR_CHUNK_PORTAL_HPP
