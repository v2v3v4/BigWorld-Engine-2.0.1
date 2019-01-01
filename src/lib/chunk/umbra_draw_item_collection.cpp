/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "umbra_config.hpp"

#if UMBRA_ENABLE

#include "umbra_draw_item_collection.hpp"

#include <umbramodel.hpp>
#include <umbraobject.hpp>


UmbraDrawItemCollection::UmbraDrawItemCollection()
{
}

UmbraDrawItemCollection::~UmbraDrawItemCollection()
{
}

/**
 *	This is the implementation of the UmbraDrawItem interface draw method
 *	It renders the draw items referenced by this collection as part of the colour pass
 *	@param pChunkContext the chunk that is currently set up 
 *	(i.e. the currently set lighting information)
 *	@return the Chunk that is set after the draw, this is in case the current
 */
/*virtual*/ Chunk* UmbraDrawItemCollection::draw(Chunk* pChunkContext)
{
	for (ItemVector::iterator it = items_.begin();
		it != items_.end();
		++it)
	{
		pChunkContext = (*it)->draw(pChunkContext);
	}
	return pChunkContext;
}

/**
 *	This is the implementation of the UmbraDrawItem interface drawDepth method
 *	It renders the draw items referenced by this collection as part of the depth pass
 *	@param pChunkContext the chunk that is currently set up 
 *	(i.e. the currently set lighting information)
 *	@return the Chunk that is set after the draw, this is in case the current
 */
/*virtual*/ Chunk* UmbraDrawItemCollection::drawDepth(Chunk* pChunkContext)
{
	for (ItemVector::iterator it = items_.begin();
		it != items_.end();
		++it)
	{
		pChunkContext = (*it)->drawDepth(pChunkContext);
	}
	return pChunkContext;
}


namespace
{
	/*
	 *	This helper function adds the points of the oriented bounding box 
	 *	of the umbra object to the point list
	 */
	void obbPoints( Umbra::Object* pObject, std::vector<Vector3>& outPoints )
	{
		Matrix obb;

		pObject->getOBB((Umbra::Matrix4x4&)obb);

		outPoints.reserve( outPoints.size() + 8 );

		// Create the 8 corners of the bounding box and add them to
		// the point list
		for (uint32 i = 0; i < 8; i++)
		{
			Vector3 corner( i & 1 ? 1.f : -1.f,
				i & 2 ? 1.f : -1.f,
				i & 4 ? 1.f : -1.f );
			corner = obb.applyPoint( corner );
			outPoints.push_back( corner );
		}
	}

	/*
	 *	This helper function calculates the centre of the points
	 *	passed in
	 */
	Vector3 pointsOrigin( const std::vector<Vector3>& points )
	{
		BoundingBox bb;
		std::vector<Vector3>::const_iterator it = points.begin();
		for (;it != points.end(); ++it)
		{
			bb.addBounds( *it );
		}

		return bb.centre();
	}

	/*
	 *	This helper class is used to transform
	 *	a list of points using std::transform
	 *	to shift their origin
	 */
	class Subtracter
	{
	public:
		Subtracter( const Vector3& v ) :
		  value_( v )
		{}
		Vector3 operator ()(const Vector3& in)
		{
			return in - value_;
		}
	private:
		Vector3 value_;
	};

} // anonymous namespace


/**
 *	This method creates the umbra object for the draw item collection
 *	it assumes that at least one object has been added to the collection
 *	@param pCell the cell to add this umbra object to
 */
void UmbraDrawItemCollection::createObject( Umbra::Cell* pCell )
{
	// First we create the list of the points used to 
	// create the model for the umbra object
	std::vector<Vector3> points;

	for (uint32 i = 0; i < items_.size(); i++)
	{
		obbPoints(items_[i]->pUmbraObject()->object(), points );
		items_[i]->pUmbraObject()->object()->setCell( NULL );
	}

	// Get the centre of the points
	Vector3 centre = pointsOrigin( points );

	// Shift the points so that their centre is at the origin
	std::transform( points.begin(), points.end(), points.begin(), Subtracter( centre ) );

	// Create the umbra objects
	pModel_ = UmbraModelProxy::getObbModel( &points.front(), points.size() );
	pObject_ = UmbraObjectProxy::get( pModel_ );

	// Set up the transform of the object
	Matrix objectToCell;
	objectToCell.setTranslate(centre);
	pObject_->object()->setObjectToCellMatrix( (Umbra::Matrix4x4&)objectToCell );

	// place the object in a cell
	pObject_->object()->setCell( pCell );

	// Make sure the user pointer of the object is pointing at us
	pObject_->object()->setUserPointer( (void*)this );
}

/**
 *	This method adds a Umbra draw item to this collection
 *	@param pItem the item to add to the collection
 */
void UmbraDrawItemCollection::addItem( UmbraDrawItem* pItem )
{
	items_.push_back( pItem );
}

#endif // UMBRA_ENABLE
