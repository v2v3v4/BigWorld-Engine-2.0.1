/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ITEM_VIEW_HPP
#define ITEM_VIEW_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/tool_view.hpp"
#include "gizmo/formatter.hpp"


/**
 *	This class reveals a vector of chunk items
 */
class ChunkItemRevealer : public PyObjectPlus
{
	Py_Header( ChunkItemRevealer, PyObjectPlus )

public:
	ChunkItemRevealer( PyTypePlus * pType ) : PyObjectPlus( pType ) { }
	virtual ~ChunkItemRevealer() { }

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_INQUIRY_METHOD(				pySeq_length )			// len(x)
	PY_BINARY_FUNC_METHOD(			pySeq_concat )			// x + y
	PY_INTARG_FUNC_METHOD(			pySeq_item )			// x[i]
	PY_INTINTARG_FUNC_METHOD(		pySeq_slice )			// x[i:j]
	PY_OBJOBJ_PROC_METHOD(			pySeq_contains )		// v in x

	typedef std::vector<ChunkItemPtr> ChunkItems;
	virtual void reveal( ChunkItems & items ) = 0;
};
PY_SCRIPT_CONVERTERS_DECLARE( ChunkItemRevealer )


/**
 *	This class holds a vector of chunk items
 */
class ChunkItemGroup : public ChunkItemRevealer
{
	Py_Header( ChunkItemGroup, ChunkItemRevealer )
public:
	ChunkItemGroup( ChunkItems items = ChunkItems(), PyTypePlus * pType = &s_type_ );
	~ChunkItemGroup();

	bool			add( ChunkItemPtr pNewbie );
	ChunkItems		get() const	{	return items_; }

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_INQUIRY_METHOD(				pySeq_length )			// len(x)
//	PY_BINARY_FUNC_METHOD(			pySeq_concat )			// x + y
//	PY_INTARG_FUNC_METHOD(			pySeq_repeat )			// x * n
//	PY_INTARG_FUNC_METHOD(			pySeq_item )			// x[i]
//	PY_INTINTARG_FUNC_METHOD(		pySeq_slice )			// x[i:j]
	PY_INTOBJARG_PROC_METHOD(		pySeq_ass_item )		// x[i] = v
	PY_INTINTOBJARG_PROC_METHOD(	pySeq_ass_slice )		// x[i:j] = v
//	PY_OBJOBJ_PROC_METHOD(			pySeq_contains )		// v in x
//	PY_BINARY_FUNC_METHOD(			pySeq_inplace_concat )	// x += y
//	PY_INTARG_FUNC_METHOD(			pySeq_inplace_repeat )	// x *= n

	PY_METHOD_DECLARE( py_add )
	PY_METHOD_DECLARE( py_rem )

	PY_RO_ATTRIBUTE_DECLARE( items_.size(), size )

	PY_FACTORY_DECLARE()

private:
	virtual void reveal( std::vector< ChunkItemPtr > & items );

	ChunkItems	items_;
};
PY_SCRIPT_CONVERTERS_DECLARE( ChunkItemGroup )

typedef SmartPointer<ChunkItemGroup> ChunkItemGroupPtr;

/**
 *	This class draws an item as selected. It calls a virtual function
 *	in a chunk item revealer to find the objects that it is to draw.
 */
class ChunkItemView : public ToolView
{
	Py_Header( ChunkItemView, ToolView )

public:
	ChunkItemView( PyTypePlus * pType = &s_type_ );
	~ChunkItemView();

	virtual void viewResource( const std::string& resourceID )
		{ resourceID_ = resourceID; }
	virtual void render( const class Tool& tool );

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( revealer_, revealer )

	PY_FACTORY_DECLARE()

protected:
	SmartPointer<ChunkItemRevealer>		revealer_;

private:
	ChunkItemView( const ChunkItemView& );
	ChunkItemView& operator=( const ChunkItemView& );
};

/**
 *	This class draws the selected item's bounding box
 */
class ChunkItemBounds : public ChunkItemView
{
	Py_Header( ChunkItemBounds, ChunkItemView )
public:
	ChunkItemBounds( PyTypePlus * pType = &s_type_ );

	PY_RW_ATTRIBUTE_DECLARE( colour_, colour )

	static ChunkItemBounds * New(
		SmartPointer<ChunkItemRevealer> spRevealer, uint32 col,
		float growFactor, std::string texture, float offset, float tile );
	PY_AUTO_FACTORY_DECLARE( ChunkItemBounds,
		OPTARG( SmartPointer<ChunkItemRevealer>, NULL,
		OPTARG( uint32, 0xffffffff,		// colour
		OPTARG( float, 0.0f,			// grow factor
		OPTARG( std::string, "",		// texture
		OPTARG( float, 0.0f,			// offset
		OPTARG( float, 1.0f,			// tiling
		END ) ) ) ) ) ) )

	virtual void render( const class Tool& tool );
private:
	int colour_;
	float growFactor_;
	Moo::BaseTexturePtr texture_;
	float offset_;
	float tile_;
};


/**
 * Utility method to get a vector of chunks from a vector of chunkItems
 */
std::vector<Chunk*> extractChunks( std::vector<ChunkItemPtr> chunkItems );


/**
 * Get a list of chunks from the items revealed by the revealer
 */
std::vector<Chunk*> extractChunks( ChunkItemRevealer* revealer );


#endif // ITEM_VIEW_HPP
