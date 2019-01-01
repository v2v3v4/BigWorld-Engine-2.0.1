/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_OVERLAPPER_HPP
#define EDITOR_CHUNK_OVERLAPPER_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "chunk/chunk_item.hpp"
#include "chunk/chunk_overlapper.hpp"


/**
 *	This class is a chunk item that records another chunk overlapping
 *	the one it is in.
 */
class EditorChunkOverlapper : public ChunkItem
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkOverlapper )

public:
	EditorChunkOverlapper();
	~EditorChunkOverlapper();

	bool load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString = NULL );

	virtual void toss( Chunk * pChunk );
	virtual void draw();
	virtual void lend( Chunk * pLender );

	Chunk *			pOverlapper()			{ return pOverlapper_; }
	DataSectionPtr	pOwnSect()				{ return pOwnSect_; }

	/**
	 * Chunks which should be drawn; must be cleared every frame
	 */
	static std::vector<Chunk*> drawList;

private:
	EditorChunkOverlapper( const EditorChunkOverlapper& );
	EditorChunkOverlapper& operator=( const EditorChunkOverlapper& );

	void bindStuff();

	Chunk *			pOverlapper_;
	DataSectionPtr	pOwnSect_;
	bool			bound_;

	static bool		s_drawAlways_;		// at options: render/scenery/shells/gameVisibility
	static uint32	s_settingsMark_;
};

typedef SmartPointer<EditorChunkOverlapper> EditorChunkOverlapperPtr;

/**
 *	This class keeps track of all the overlappers in a chunk,
 *	and can form and cut them when a chunk is moved.
 */
class EditorChunkOverlappers : public ChunkCache
{
public:
	EditorChunkOverlappers( Chunk & chunk );
	~EditorChunkOverlappers();

	static Instance<EditorChunkOverlappers> instance;

	void add( EditorChunkOverlapperPtr pOverlapper );
	void del( EditorChunkOverlapperPtr pOverlapper );

	void form( Chunk * pOverlapper );
	void cut( Chunk * pOverlapper );

	void bind( bool isUnbind );

	typedef std::vector< EditorChunkOverlapperPtr > Items;
	const Items& overlappers() const
	{
		return items_;
	}
private:
	Chunk *				pChunk_;
	Items				items_;
	bool				binding_;
};


#endif // EDITOR_CHUNK_OVERLAPPER_HPP
