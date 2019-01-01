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

#include "wpmarker.hpp"

#include "chunk/chunk_model.hpp"
#include "chunk/chunk_model_obstacle.hpp"
#include "model/super_model.hpp"
#include "moo/render_context.hpp"
#include "pyscript/script.hpp"
#include "pyscript/py_data_section.hpp"
#include "cstdmf/debug.hpp"
#include "resmgr/bwresource.hpp"
#include "entitydef/constants.hpp"

#include <string>

DECLARE_DEBUG_COMPONENT2( "WPMarker", 0 )

IMPLEMENT_CHUNK_ITEM( WPMarker, marker, 0 )

int WPMarker_token = 0; // reference this elsewhere (wpgen.cpp). stops file being linked out.

extern std::map< std::string, MarkerGirthInfo > g_markerGirthInfo;

/**
 *	Constructor
 */
WPMarker::WPMarker() :
	transform_( Matrix::identity ),
	pSuperModel_( NULL )
{
}

/**
 *	Destructor
 */
WPMarker::~WPMarker()
{
}

std::vector<MarkerItem> WPMarker::markers_;


namespace 
{

bool isWideMarkerName( const std::string & n )
{
	BW_GUARD;

	return g_markerGirthInfo.find(n) != g_markerGirthInfo.end();
}

}

/**
 *	Toss method - 
 *  tosses the marker into the relevant chunks markercache.
 *  also 
 */
void WPMarker::toss( Chunk * pChunk )
{
	BW_GUARD;

	if (pChunk == pChunk_) return;

	if (pChunk_ != NULL)
	{
		WPMarkerCache::instance( *pChunk_ ).del( this );

		if (pSuperModel_ != NULL)
		{
			ChunkModelObstacle::instance( *pChunk_ ).delObstacles( this );
		}
	}
	this->ChunkItem::toss( pChunk );
	if (pChunk_ != NULL)
	{
		WPMarkerCache::instance( *pChunk_ ).add( this );

		if (pSuperModel_ != NULL)
		{
			Matrix world( pChunk_->transform() );
			world.preMultiply( this->transform_ );

			for (int i = 0; i < this->pSuperModel_->nModels(); i++)
			{
				ChunkModelObstacle::instance( *pChunk_ ).addModel(
					this->pSuperModel_->topModel( i ), world, this );
			}
		}
	} 

	// now in chunck, update coords
	if (pChunk != 0)
	{
		Vector3 position = pChunk_->transform().applyPoint( position_ );

		// if this marker needs wide girths generated for it, then push it's position on
		// the global list.
		if (isWideMarkerName(category_) )
		{
			MarkerItem newItem;
			newItem.position = position;
			newItem.category = category_;
			markers_.push_back( newItem );
		}

	}
	else 
	{
		WARNING_MSG( "WPMarker::toss: pChunk_ = 0\n" );
	}

}

/**
 *	Load the marker from the given data section
 *
 * @param pSection  chunk file DataSectionPtr.
 */
bool WPMarker::load( DataSectionPtr pSection )
{
	BW_GUARD;

	if ( !pSection->findChild( "transform" ) )
	{
		WARNING_MSG( "ChunkItemMarker::load: marker section doesn't have transform\n" );
		Script::releaseLock();
		return true;
	}

	position_ = pSection->readVector3( "transform/row3" );
	transform_ = pSection->readMatrix34( "transform" );
	category_ = pSection->readString( "category", "" );

	// see if model specified in marker_categories.xml
	std::string modelName =  BWResource::instance().rootSection()->readString(
		std::string( EntityDef::Constants::markerCategoriesFile() ) +
		"/" + category_ + "/wpgen/model" );

	if ( modelName != "" )
	{
		std::vector<std::string> modelNames( 1, modelName );
		pSuperModel_ = new SuperModel( modelNames );
		if (pSuperModel_->nModels() != 1)
		{
			ERROR_MSG( "WPEntity::load: "
				"Could not load obstacle model %s\n",
				modelName.c_str() );
			delete pSuperModel_;
			pSuperModel_ = NULL;
		}
	}
	else
	{
		typeName_ = pSection->readString( "type" );
		pProps_ = pSection->openSection( "properties" );
	}

	return true;
}

/**
 *	finishLoad method
 */
void WPMarker::finishLoad()
{
	BW_GUARD;

	if ( typeName_ != "" )
	{
		PyObject * pModule = PyImport_ImportModule(
			const_cast<char*>(typeName_.c_str()) );
		if (pModule != NULL)
		{
			PyObject * pClass = PyObject_GetAttrString(
				pModule, const_cast<char*>(typeName_.c_str()) );

			if (pClass != NULL)
			{
				// make an instance of the class
				PyObject * pEntityInst = PyObject_CallObject( pClass, PyTuple_New(0) );
				if (pEntityInst != NULL)
				{
					PyObject * pTuple = PyTuple_New(1);

					if (pProps_)
					{
						PyTuple_SetItem( pTuple, 0,
								new PyDataSection( pProps_ ) );
					}
					else
					{
						Py_INCREF( Py_None );
						PyTuple_SetItem( pTuple, 0, Py_None );
					}

					PyObject * pResult = Script::ask(
						PyObject_GetAttrString( pEntityInst, "getObstacleModel" ),
						pTuple,
						"WPEntity::load: getObstacle",
						/*okIfFunctionNull*/true );
					std::string obstModelName;
					if (Script::setAnswer( pResult, obstModelName ))
					{
						if ( obstModelName != "" )
						{
							std::vector<std::string> modelNames( 1, obstModelName );
							pSuperModel_ = new SuperModel( modelNames );
							if (pSuperModel_->nModels() != 1)
							{
								ERROR_MSG( "WPEntity::load: "
									"Could not load obstacle model %s\n",
									obstModelName.c_str() );
								delete pSuperModel_;
								pSuperModel_ = NULL;
							}
						}
					}
				}
				Py_DECREF( pClass );
			}
			Py_DECREF( pModule );
		}
		else if (BWResource::fileExists( BWResolver::resolveFilename(
			std::string( EntityDef::Constants::entitiesEditorPath() ) +
			"/" + typeName_ + ".py" ) ))
		{
			ERROR_MSG( "Could not load editor entity module %s\n", typeName_.c_str() );
			PyErr_Print();
		}
		PyErr_Clear();
	}
}

/**
 *	overridden draw method
 */
void WPMarker::draw()
{
	BW_GUARD;

	if( pSuperModel_ == NULL )
		finishLoad();
	if (pSuperModel_ != NULL)
	{
		Moo::rc().push();
		Moo::rc().preMultiply( transform_ );

		pSuperModel_->draw( NULL );

		Moo::rc().pop();
	}
}


// -----------------------------------------------------------------------------
// Section: WPMarkerCache
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
WPMarkerCache::WPMarkerCache( Chunk & )
{
}

/**
 *	Destructor
 */
WPMarkerCache::~WPMarkerCache()
{
}

/**
 *	Add this marker
 */
void WPMarkerCache::add( WPMarkerPtr e )
{
	BW_GUARD;

	markers_.push_back( e );
}

/**
 *	Remove this marker
 */
void WPMarkerCache::del( WPMarkerPtr e )
{
	BW_GUARD;

	WPMarkers::iterator found = std::find(
		markers_.begin(), markers_.end(), e );
	if (found != markers_.end())
		markers_.erase( found );
}


/// Static instance accessor initialiser
ChunkCache::Instance<WPMarkerCache> WPMarkerCache::instance;
