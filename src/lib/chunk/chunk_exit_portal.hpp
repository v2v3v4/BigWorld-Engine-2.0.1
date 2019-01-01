/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_EXIT_PORTAL_HPP
#define CHUNK_EXIT_PORTAL_HPP

#include "chunk_item.hpp"
#include "chunk_manager.hpp"

/**
 *	This class is a chunk item that represents an
 *	exit portal.  If we are drawn, you can be sure
 *	that there has been some 'inside/outside' action
 *	going on.
 */
class ChunkExitPortal : public ChunkItem
{
public:
	ChunkExitPortal( struct ChunkBoundary::Portal& p );
	bool load( DataSectionPtr pSection );

	///Use this method from external classes to check
	///which exit portals have been traversed this frame.
	typedef std::vector<ChunkExitPortal*> Vector;
	static ChunkExitPortal::Vector& seenExitPortals();
	struct ChunkBoundary::Portal& portal() const { return portal_; }

#ifdef EDITOR_ENABLED
	virtual const char * edClassName()		{ return "ChunkExitPortal"; }
	virtual void edBounds( BoundingBox & bbRet ) const;
	virtual void edPostClone( EditorChunkItem* srcItem ) {} 
	virtual void edPostCreate()  {} 
	virtual void edPostModify() {} 

	virtual DataSectionPtr pOwnSect();
	virtual const DataSectionPtr pOwnSect()	const;
#endif

#if UMBRA_ENABLE
	void createUmbraObject();
#endif

	void draw();

private:
	struct ChunkBoundary::Portal&	portal_;
	static ChunkExitPortal::Vector seenExitPortals_;

#ifdef EDITOR_ENABLED
	DataSectionPtr pOwnSection_;
#endif
};


#endif // CHUNK_EXIT_PORTAL_HPP
