/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_USER_DATA_OBJECT_HPP
#define CHUNK_USER_DATA_OBJECT_HPP

#include "chunk_item.hpp"
#include "chunk.hpp"
#include "cstdmf/aligned.hpp"
#include "cstdmf/guard.hpp"
#include "math/matrix.hpp"
#include <string>
#include "user_data_object.hpp"


class UserDataObjectType; 
class Space;


/**
    The use of which shall be to define placable metadata objects that can be
	embedded into each chunk using world editor. 
*/
class ChunkUserDataObject : public ChunkItem, public Aligned
{
	DECLARE_CHUNK_ITEM( ChunkUserDataObject )

public:
	ChunkUserDataObject();
	virtual ~ChunkUserDataObject();
	bool load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString = NULL);
	virtual void toss( Chunk * pChunk );
	void bind();

protected:
	SmartPointer<UserDataObjectType>	pType_;
	SmartPointer< UserDataObject > pUserDataObject_;


private:
	UserDataObjectInitData initData_;
};

typedef SmartPointer<ChunkUserDataObject> ChunkUserDataObjectPtr;
/**
 *	This class is a cache of ChunkUserDataObjects, so we can create their python
 *	objects when they are bound and not before.
 */
class ChunkUserDataObjectCache : public ChunkCache
{
public:
	ChunkUserDataObjectCache( Chunk & chunk );
	~ChunkUserDataObjectCache();

	virtual void bind( bool isUnbind );

	void add( ChunkUserDataObject * pUserDataObject );
	void del( ChunkUserDataObject * pUserDataObject );

	static Instance<ChunkUserDataObjectCache> instance;

private:
	typedef std::vector< ChunkUserDataObject * > ChunkUserDataObjects;
	ChunkUserDataObjects	userDataObjects_;
};


#endif //CHUNK_USER_DATA_OBJECT_HPP
