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

#include "pymodel.hpp"

#ifndef CODE_INLINE
#include "pymodel.ipp"
#endif

#include "cstdmf/config.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/dogwatch.hpp"
#include "cstdmf/vectornodest.hpp"
#include "cstdmf/memory_trace.hpp"

DECLARE_DEBUG_COMPONENT2( "PyModel", 0 );

#include "pyfashion.hpp"
#include "pydye.hpp"

#include "action_queue.hpp"
#include "motor.hpp"
#include "pymodelnode.hpp"
#include "shimmer_draw_override.hpp"
#include "stipple_draw_override.hpp"

#include "ashes/bounding_box_gui_component.hpp"
#include "ashes/text_gui_component.hpp"

#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk.hpp"
#include "chunk/chunk_obstacle.hpp"

#include "romp/rain.hpp"
#include "romp/water.hpp"
#include "romp/geometrics.hpp"
#include "model/super_model.hpp"
#include "romp/font.hpp"
#include "romp/labels.hpp"

#include "math/colour.hpp"

#include "model/super_model_dye.hpp"

#include "moo/render_context.hpp"
#include "moo/visual_channels.hpp"
#include "moo/node_catalogue.hpp"

#include "fmodsound/py_sound.hpp"
#include "fmodsound/py_sound_list.hpp"

#include "pyscript/py_data_section.hpp"

#pragma warning (disable:4355)	// this used in initialiser list


extern bool g_crossFadeLODs;

static SmartPointer< ShimmerDrawOverride > s_pShimmer = NULL;
static SmartPointer< StippleDrawOverride > s_pStipple = NULL;

// -----------------------------------------------------------------------------
// Section: PyModel
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyModel )

PY_BEGIN_METHODS( PyModel )
	PY_METHOD( action )
	PY_METHOD( playSound )
	PY_METHOD( getSound )

	// Aliases provided for backwards compatibility
	PY_METHOD_ALIAS( playSound, playFxSound )
	PY_METHOD_ALIAS( getSound, getFxSound )

	PY_METHOD( addMotor )
	PY_METHOD( delMotor )
	PY_METHOD( straighten )
	PY_METHOD( rotate )
	PY_METHOD( alignTriangle )
	PY_METHOD( reflectOffTriangle )
	PY_METHOD( zoomExtents )
	PY_METHOD( node )
	PY_METHOD( origin )
	PY_METHOD( cue )

	PY_METHOD( saveActionQueue )
	PY_METHOD( restoreActionQueue )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyModel )
	/*~ attribute PyModel.inWorld
	 *
	 *	This attribute is used to determine whether the model has been added to
	 *	the world.  It is read only, and is set to 1 if the Model is in the
	 *	world, 0 otherwise.  In order to add a model to the world, you can
	 *	attach it to another model, or set the model attribute of an entity to
	 *	this PyModel.
	 *
	 *	@type	Read-Only Integer
	 */
	PY_ATTRIBUTE( inWorld )
	/*~ attribute PyModel.visible
	 *
	 *	This attribute controls whether or not the model is rendered.  If it is
	 *	set to 0, the model is not rendered, anything else and it is rendered.
	 *
	 *	@type	Integer
	 */
	PY_ATTRIBUTE( visible )
	/*~ attribute PyModel.visibleAttachments
	 *
	 *	This attribute can be set to allow hiding of model but update of animation and drawing
	 *	of attachments (or allowing selective control of their individual visibility). Only meaningful
	 *	when visible flag is set to false.
	 *
	 *	@type	Integer
	 */
	PY_ATTRIBUTE( visibleAttachments )
	/*~ attribute PyModel.moveAttachments
	 *
	 *	This attribute can be set to allow motors to function on attached objects.
	 *	By default this flag is turned off, because calling move on all known nodes
	 *	on all PyModels can take a measurable amount of time, even if there are no
	 *	motors attached.
	 *	Note the action matcher is a motor, turn this flag on to allow the action
	 *	matcher to apply to attached objects.
	 *
	 *	@type	Boolean
	 */
	PY_ATTRIBUTE( moveAttachments )
	/*~ attribute PyModel.outsideOnly
	 *
	 *	Indicates that this model is only ever positioned in outside chunks.
	 *	This allows the engine to optimise processing on this model.
	 *	There are extra processing steps that need to be done if  this model is
	 *	capable of being inside.  If it is only ever going to be outside, then
	 *	optimise by setting this attribute to 1. By default it is 0.
	 *
	 *	@type	Integer
	 */
	PY_ATTRIBUTE( outsideOnly )
	/*~ attribute PyModel.stipple
	 *
	 *	This attribute sets the model and its attachments into stipple mode,
	 *	meaning the model is drawn with a screen-based mask that only allows
	 *	drawing on every other pixel.  It can be used as a cheap invisibility
	 *	or transparency effect.
	 *
	 *	@type	Integer
	 */
	PY_ATTRIBUTE( stipple )
	/*~ attribute PyModel.shimmer
	 *
	 *	This attribute ovverides the model's shaders and draws all opaque
	 *	pixels into the shimmer channel.
	 *
	 *	@type	Integer
	 */
	PY_ATTRIBUTE( shimmer )
	/*~ attribute PyModel.moveScale
	 *
	 *	This attribute sets the model's actions's movement speed scale.
	 *	The higher the moveScale the faster the model moves.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( moveScale )
	/*~ attribute PyModel.actionScale
	 *
	 *	This attribute sets the model's action's animation speed scale.
	 *	The higher the actionScale the faster the model's actions are played.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( actionScale )

	/*~ attribute PyModel.position
	 *
	 *	The position of the model within the world, as a 3d vector.  This is in
	 *	theory write as well as read, however if there are motors attached to
	 *	the model, then those motors will often update the models position
	 *	every tick, at which point any data written will be overwritten before
	 *	it gets rendered at the new position.  In a model with no motors
	 *	though, you can set the models position by writing to this attribute.
	 *
	 *	@type	Vector3
	 */
	PY_ATTRIBUTE( position )
	/*~ attribute PyModel.scale
	 *
	 *	Used to scale the model in each of the 3 dimensions.  Takes a vector3,
	 *	each component of which specifies the scale for the model in the
	 *	corresponding dimension.  This only effects the rendering of the model,
	 *	not its bounding box for collision tests
	 *
	 *	@type	Vector3
	 */
	PY_ATTRIBUTE( scale )
	/*~ attribute PyModel.height
	 *
	 *	This is the height of the bounding box used for collision detection.
	 *	Changing this variable sets the top of the bounding box to height above
	 *	the existing bottom of the bounding box.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( height )
	/*~ attribute PyModel.yaw
	 *
	 *	The rotation of the model about its own (rather than the world's)
	 *	y-axis.  This is, in theory, write as well as read, however if there
	 *	are motors attached to the model, then those motors will often update
	 *	the models yaw every tick, at which point any data written will be
	 *	overwritten before it gets rendered at the new yaw.  In a model with no
	 *  motors though, you can set the models yaw by writing to this attribute.
	 *
	 *	@type	Float;
	 */
	PY_ATTRIBUTE( yaw )
	/*~ attribute PyModel.unitRotation
	 *
	 *	This attribute allows scripts to access the difference in yaw between
	 *	the model and the direction the ActionMatcher (if one is present) is
	 *	matching to.
	 *
	 *	@type	Read-Only Float
	 */
	PY_ATTRIBUTE( unitRotation )
	/*~ attribute PyModel.queue
	 *
	 *	This read only attribute is a list of the names of the actions which
	 *	have been queued up to play.
	 *
	 *	@type	Read-Only List of Strings
	 */
	PY_ATTRIBUTE( queue )
	/*~	attribute PyModel.motors
	 *
	 *	This attribute is the list of motors which affect the position and
	 *	rotation of the model, based on the position and rotation of the owning
	 *	entity.  It is generally manipulated using the addMotor and delMotor
	 *	functions on the model.  On creation, an entity's model has an
	 *	ActionMatcher as its only motor.
	 *
	 *	@type	List of Motors
	 */
	PY_ATTRIBUTE( motors )
	/*~	attribute PyModel.sources
	 *
	 *	This read-only attribute is a list of the resource ids of all the
	 *	models that make up the super model.  In the case of a simple model
	 *	this is a list of just one resource id.
	 *
	 *	@type	Read-Only List of Strings
	 */
	PY_ATTRIBUTE( sources )
	/*~	attribute PyModel.bounds
	 *
	 *	This read-only attribute gives the bounding box for the model.  It
	 *	returns a MatrixProvider.  The matrix specifies what transformation
	 *	would need to be applied to a 1x1x1 cube placed at the
	 *	world-space origin to scale, rotate and translate it to bound the
	 *	model.  The bounds themselves are read in from the &lt;boundingBox>
	 *	property of the .visual file that was used to create the model,
	 *	and don't update to reflect the current pose of the model.
	 *
	 *	This MatrixProvider can be used with the BoundingBoxGUIComponent to
	 *	find the smallest bounding rectangle of the model, in relation to the
	 *	current viewport on the world.
	 *
	 *	@type	Read-Only MatrixProvider
	 */
	PY_ATTRIBUTE( bounds )
	/*~ attribute PyModel.tossCallback
	 *
	 *	This attribute is the callback function that gets called every time the
	 *	model moves from one chunk to another.  There are no specific arguments
	 *	supplied to the function.
	 *
	 *	@type	Function object
	 */
	PY_ATTRIBUTE( tossCallback )
	/*~ attribute	PyModel.root
	 *
	 *	This read-only attribute is the root node of the model.
	 *
	 *	@type Read-Only PyModelNode
	 */
	PY_ATTRIBUTE( root )

	/*~ attribute	PyModel.stopSoundsOnDestroy
	 *
	 *  Controls whether or not sounds attached to this model are stopped when
	 *  this model is destroyed.  This attribute is set to True by default,
	 *  however you may want to set this to False if you have a special effect
	 *  whose model may disappear before its sound events have finished playing.
	 *
	 *	@type bool
	 */
	PY_ATTRIBUTE( stopSoundsOnDestroy )

PY_END_ATTRIBUTES()

/*~ function BigWorld.Model
 *
 *	Model is a factory function that creates and returns a new PyModel object
 *	(None if there are problems loading any of the requested resources).
 *	PyModel@@s are renderable objects which can be moved around the world,
 *	animated, dyed (have materials programatically assigned) and contain points
 *	to which objects can be attached.
 *
 *	The parameters for the function are one or more model resources. Each of
 *	these individual models is loaded, and combined into a "supermodel".
 *
 *	This function will block the main thread to load models in the event that
 *	they were not specified as prerequisites. To load models asynchronously
 *	from script use the method BigWorld.fetchModel().
 *
 *	For example, to load a body and a head model into one supermodel:
 *
 *	@{
 *	model = BigWorld.Model( "models/body.model", "models/head.model" )
 *	@}
 *
 *	@see	BigWorld.fetchModel
 *
 *	@param *modelPath	String argument(s) for the model.
 *
 *	@return A reference to the PyModel specified.
 */
PY_FACTORY_NAMED( PyModel, "Model", BigWorld )


PY_SCRIPT_CONVERTERS( PyModel )

/*~ function BigWorld.fetchModel
 *
 *	fetchModel is a function that creates and returns a new PyModel object
 *	through the callback function provided (None if there are problems loading
 *	any of the requested resources). PyModel@@s are renderable objects which
 *	can be moved around the world, animated, dyed (have materials
 *	programatically assigned) and contain points to which objects can be
 *	attached.
 *
 *	The parameters for the function are one or more model resources. Each of
 *	these individual models is loaded, and combined into a "supermodel".
 *
 *	This function is similar to the BigWorld.Model function. However, it
 *	schedules the model loading as a background task so that main game script
 *	execution would not be affected. After model loading is completed, callback
 *	function will be triggered with the valid PyModel.
 *
 *	For example, to load a body and a head model into one supermodel:
 *
 *	@{
 *	model = BigWorld.fetchModel( "models/body.model", "models/head.model", onLoadCompleted )
 *	@}
 *
 *	@param *modelPath	String argument(s) for the model.
 *	@param onLoadCompleted The function to be called when the model resource loading is
 *						 completed. An valid PyModel will be returned. When the model
 *						 loading fails. A None is passed back.
 *
 *	@return No return values.
 */

ScheduledModelLoader::ScheduledModelLoader( PyObject * bgLoadCallback,
							const std::vector< std::string > & modelNames ) :
	pBgLoadCallback_( bgLoadCallback ),
	modelNames_( modelNames )
{
	BW_GUARD;
	BgTaskManager::instance().addBackgroundTask( this );
}

void ScheduledModelLoader::doBackgroundTask( BgTaskManager & mgr )

{
	BW_GUARD;
	MEM_TRACE_BEGIN( modelNames_[0] )
	pSuperModel_ = new SuperModel( modelNames_ );

	mgr.addMainThreadTask( this );
}


void ScheduledModelLoader::doMainThreadTask( BgTaskManager & mgr )
{
	BW_GUARD;
	TRACE_MSG( "ScheduledModelLoader::doMainThreadTask:\n" );

	bool successfullLoad =
		modelNames_.size() == pSuperModel_->nModels();

	if (successfullLoad)
	{
		MEM_TRACE_END();
		pSuperModel_->checkBB( false );
		PyModel * model = new PyModel( pSuperModel_ );
		Py_INCREF( pBgLoadCallback_.getObject() );
		Script::call( &*(pBgLoadCallback_),
			Py_BuildValue( "(O)", (PyObject *) model ),
			"PyModel onModelLoaded callback: " );
		Py_DECREF( model );
	}
	else
	{
		ERROR_MSG( "Model(): Did not found all models "
			"Only found %d out of %d models requested\n",
			pSuperModel_->nModels(), modelNames_.size() );

		Py_INCREF( pBgLoadCallback_.getObject() );

		//following is equivalent to return a Py_None
		Script::call( &*(pBgLoadCallback_),
				Py_BuildValue( "(s)", NULL),
				"PyModel onModelLoaded callback: " );

		delete pSuperModel_;
		pSuperModel_ = NULL;
		MEM_TRACE_END()
	}
}

static PyObject * py_fetchModel( PyObject * args )
{
	BW_GUARD;
	int size = PyTuple_Size( args );
	if (size < 2)
	{
		PyErr_Format( PyExc_TypeError,
			"BigWorld.fetchModel requires at least 2 arguments" );
		return NULL;
	}

	int sz = PyTuple_Size( args ) - 1;
	std::vector< std::string > modelNames( sz );
	int i;
	PyObject * pItem = NULL;
	for (i = 0; i < sz; i++)
	{
		pItem = PyTuple_GetItem( args, i );	// borrowed
		if (!PyString_Check( pItem ))
		{
			PyErr_Format( PyExc_ValueError, "fetchModel(): "
				"Argument %d is not a string", i );
			return NULL;
		}
		modelNames[i] = PyString_AsString( pItem );
	}

	PyObject * bgLoadCallback = PyTuple_GetItem( args, sz );

	if (!PyCallable_Check( bgLoadCallback ))
	{
		PyErr_Format( PyExc_ValueError, "fetchModel(): "
			"Callback function is not a callable object" );
		return NULL;
	}

	// This object deletes itself.
	new ScheduledModelLoader( bgLoadCallback, modelNames );

	Py_Return;
}

PY_MODULE_FUNCTION( fetchModel, BigWorld )

// This is the model that is currently being ticked() or moved() or whatever.
// This replaces the SoundSourceMinder::s_pTopMinder_ thing that was used for
// letting Motors trigger sounds in rev().
//
// TODO: This is a pretty dodgy way of doing this and we should think of a
// better way of passing the PyModel* down into PyAttachment::tick() and
// PyAttachment::draw().
PyModel* PyModel::s_pCurrent_ = NULL;

/**
 *	Constructor
 */
PyModel::PyModel( SuperModel * pSuperModel, PyTypePlus * pType ) :
 PyAttachment( pType ),
 pSuperModel_( pSuperModel ),
 localBoundingBox_( Vector3::zero(), Vector3::zero() ),
 localVisibilityBox_( Vector3::zero(), Vector3::zero() ),
 height_( 0.f ),
 visible_( true ),
 visibleAttachments_( false ),
 moveAttachments_( false ),
 outsideOnly_( false ),
 unitOffset_( Vector3::zero() ),
 unitRotation_( 0 ),
 unitTransform_( Matrix::identity ),
 shouldDrawSkeleton_( false ),
 pDebugInfo_( NULL ),
 transformModsDirty_( false ),
 stipple_( false ),
 shimmer_( false ),
 moveScale_( 1.f ),
 actionScale_( 1.f )
#if FMOD_SUPPORT
 ,pSounds_( NULL ),
  cleanupSounds_( true )
#endif
{
	BW_GUARD;
	// calculate our local bounding boxes for future reference
	this->calculateBoundingBoxes();

	if (pSuperModel_ != NULL)
	{
		// get the initial transform matrices
		//Model::Animation * pMainDefault =
		//	pSuperModel->topModel(0)->lookupLocalDefaultAnimation();
		//pMainDefault->flagFactor( -1, initialItinerantTransform_ );
		//initialItinerantTransformInverse_.invert( initialItinerantTransform_);
		initialItinerantContext_ = Matrix::identity;
		initialItinerantContextInverse_.invert( initialItinerantContext_ );

		// and dress in default in case someone wants our nodes
		pSuperModel_->dressInDefault();
	}
}


/**
 *	Destructor
 */
PyModel::~PyModel()
{
	BW_GUARD;
	// take us out of the world
	if (this->isInWorld()) this->leaveWorld();

	// detach all our known nodes
	PyModelNodes::iterator nit;
	for (nit = knownNodes_.begin(); nit != knownNodes_.end(); nit++)
	{
		(*nit)->detach();
		Py_DECREF( *nit );
	}
	knownNodes_.clear();

	// inform all fashions that they have been disowned
	for (Fashions::iterator it = fashions_.begin(); it != fashions_.end(); it++)
		it->second->disowned();

	// Get rid of all motors
	for (Motors::iterator it = motors_.begin(); it != motors_.end(); it++)
	{
		(*it)->detach();
		Py_DECREF( (*it) );
	}

	// and finally delete the supermodel (whoops!)
	if (pSuperModel_ != NULL)
	{
		delete pSuperModel_;
		pSuperModel_ = NULL;
	}

	// This may be NULL.
	delete pDebugInfo_;

	// Clean up any sound events that might still be playing
#if FMOD_SUPPORT
	this->cleanupSounds();
#endif
}


/**
 *	We've changed chunks. Someone might be interested.
 */
void PyModel::tossed( bool outside )
{
	BW_GUARD;
	if (tossCallback_)
	{
		Py_INCREF( tossCallback_.getObject() );
		Script::call( &*tossCallback_, PyTuple_New(0),
			"PyModel toss callback: " );
	}

	PyModelNodes::iterator nit;
	for (nit = knownNodes_.begin(); nit != knownNodes_.end(); nit++)
		(*nit)->tossed( outside );
}


/**
 *	This method calculates the local space BoundingBoxes for the model
 */
void PyModel::calculateBoundingBoxes()
{
	BW_GUARD;
	if (pSuperModel_ != NULL)
	{
		BoundingBox bb;

		pSuperModel_->localBoundingBox( bb );

		float maxXZ = max( fabsf( bb.maxBounds().x ) , max( fabsf( bb.minBounds().x ),
			max( fabsf( bb.maxBounds().z ), fabsf( bb.minBounds().z ) ) ) ) * 2;
		float ysz = bb.maxBounds().y - bb.minBounds().y;

		localBoundingBox_ = BoundingBox(
			Vector3( -maxXZ, bb.minBounds().y - ysz*0.5f, -maxXZ ),
			Vector3( maxXZ, bb.maxBounds().y + ysz*0.5f, maxXZ ) );

		pSuperModel_->localVisibilityBox( bb );

		maxXZ = max( fabsf( bb.maxBounds().x ) , max( fabsf( bb.minBounds().x ),
			max( fabsf( bb.maxBounds().z ), fabsf( bb.minBounds().z ) ) ) ) * 2;
		ysz = bb.maxBounds().y - bb.minBounds().y;

		localVisibilityBox_ = BoundingBox(
			Vector3( -maxXZ, bb.minBounds().y - ysz*0.5f, -maxXZ ),
			Vector3( maxXZ, bb.maxBounds().y + ysz*0.5f, maxXZ ) );
	}
	else
	{
		localBoundingBox_.setBounds( Vector3(-0.5f,  0.0f, -0.5f),
								Vector3( 0.5f,  1.0f,  0.5f) );
		localVisibilityBox_ = localBoundingBox_;
	}
}


/**
 *	This method makes up a name for this model
 */
std::string PyModel::name() const
{
	BW_GUARD;
	if (!pSuperModel_) return "";

	std::string names;
	for (int i = 0; i < pSuperModel_->nModels(); i++)
	{
		if (i) names += ";";
		names += pSuperModel_->topModel(i)->resourceID();
	}
	return names;
}


/**
 *	Override from PyAttachment
 */
void PyModel::detach()
{
	BW_GUARD;
	this->PyAttachment::detach();

	// reset our coupling fashion since we are no longer a couple :(
	pCouplingFashion_ = NULL;
}


/**
 *	This allows scripts to get various properties of a model
 */
PyObject * PyModel::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	if (pSuperModel_)
	{
		// try to find the hard point named 'attr'
		const Moo::NodePtr pHardPoint = pSuperModel_->findNode(
			std::string( "HP_" ) + attr );
		if (pHardPoint)
		{
			return this->accessNode( pHardPoint )->pyGetHardPoint();
		}

		// try to find the action named 'attr'
		SuperModelActionPtr smap = pSuperModel_->getAction( attr );
		if (smap)
		{
			return new ActionQueuer( this, smap, NULL );
		}

		// try to find the dye matter or other fashion named 'attr'
		Fashions::iterator found = fashions_.find( attr );
		if (found != fashions_.end())
		{
			PyObject * pPyDye = &*found->second;
			Py_INCREF( pPyDye );
			return pPyDye;
		}
		SuperModelDyePtr pDye = pSuperModel_->getDye( attr, "" );
		if (pDye)	// some question as to whether we should do this...
		{	// but I want to present illusion that matters are always there
			SmartPointer<PyFashion> pPyDye = new PyDye( pDye, attr, "Default" );
			fashions_.insert( std::make_pair( attr, pPyDye ) );
			return pPyDye.getObject(); // ref count comes from constructor
		}
	}

	return PyAttachment::pyGetAttribute( attr );
}


/**
 *	This allows scripts to set various properties of a model
 */
int PyModel::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	if (pSuperModel_)
	{
		// try to find the hard point named 'attr'
		const Moo::NodePtr pHardPoint = pSuperModel_->findNode(
			std::string( "HP_" ) + attr );
		if (pHardPoint)
		{
			return this->accessNode( pHardPoint )->pySetHardPoint( value );
		}

		// check if it's a PyFashion
		if (PyFashion::Check( value ))
		{
			// some fashions must be copied when set into models
			PyFashion * pPyFashion = static_cast<PyFashion*>( value );

			// we always have to make a copy of the dye object here, because
			// it may have been created from a different model/supermodel.
			pPyFashion = pPyFashion->makeCopy( this, attr );
			if (pPyFashion == NULL) return -1;	// makeCopy sets exception

			Fashions::iterator found = fashions_.find( attr );
			if (found != fashions_.end()) found->second->disowned();
			fashions_[ attr ] = pPyFashion;
			Py_DECREF( pPyFashion );
			return 0;
		}

		// try to find the dye matter named 'attr'
		if (PyString_Check( value ) || value == Py_None)
		{
			const char * valStr = (value == Py_None ? "" :
				PyString_AsString( value ) );

			SuperModelDyePtr pDye = pSuperModel_->getDye( attr, valStr );
			if (pDye)	// only NULL if no such matter
			{
				SmartPointer<PyFashion> newPyDye(
					new PyDye( pDye, attr, valStr ), true );

				Fashions::iterator found = fashions_.find( attr );
				if (found != fashions_.end()) found->second->disowned();
				fashions_[ attr ] = newPyDye;
				return 0;
			}
		}

		// check if it's none and in our list of fashions (and not a dye)
		if (value == Py_None)
		{
			Fashions::iterator found = fashions_.find( attr );
			if (found != fashions_.end())
			{
				found->second->disowned();
				fashions_.erase( found );
				return 0;
			}
		}

	}

	return PyAttachment::pySetAttribute( attr, value );
}


/**
 *	This allows scripts to delete some properties of a model
 */
int PyModel::pyDelAttribute( const char * attr )
{
	BW_GUARD;
	// currently only fashions can be deleted
	Fashions::iterator found = fashions_.find( attr );
	if (found != fashions_.end())
	{
		found->second->disowned();
		fashions_.erase( found );
		return 0;
	}

	return this->PyAttachment::pyDelAttribute( attr );
}



/**
 *	Specialised get method for the 'sources' attribute
 */
PyObject * PyModel::pyGet_sources()
{
	BW_GUARD;
	int nModels = 0;
	if (pSuperModel_ != NULL)
		nModels = pSuperModel_->nModels();
	PyObject * pTuple = PyTuple_New( nModels );
	for (int i = 0; i < nModels; i++)
	{
		PyTuple_SetItem( pTuple, i, Script::getData(
			pSuperModel_->topModel(i)->resourceID() ) );
	}
	return pTuple;
}


/**
 *	Specialised get method for the 'position' attribute
 */
PyObject * PyModel::pyGet_position()
{
	BW_GUARD;
	// TODO: It may be nice to make a type so that this property can be modified
	// instead of being read-only.
	return Script::getReadOnlyData( this->worldTransform().applyToOrigin() );
}

/**
 *	Specialised set method for the 'position' attribute
 */
int PyModel::pySet_position( PyObject * value )
{
	BW_GUARD;
	Matrix	wM( this->worldTransform() );
	Vector3	newPos = wM.applyToOrigin();
	int ret = Script::setData( value, newPos, "position" );
	if (ret == 0)
	{
		wM.translation( newPos );
		this->worldTransform( wM );
	}

#if FMOD_SUPPORT
	// NB: event velocity WONT automatically update from setPosition because dTime is unknown
	if (pSounds_ &&
		!pSounds_->update( this->worldTransform().applyToOrigin(), this->worldTransform()[2], 0))
	{
		ERROR_MSG( "PyModel::pySet_position: "
			"Failed to update sound events\n" );
	}
#endif

	return ret;
}


/**
 *	Specialised get method for the 'scale' attribute
 */
PyObject * PyModel::pyGet_scale()
{
	BW_GUARD;
	Vector3	vScale(
		this->worldTransform().applyToUnitAxisVector(0).length(),
		this->worldTransform().applyToUnitAxisVector(1).length(),
		this->worldTransform().applyToUnitAxisVector(2).length() );
	// TODO: It may be nice to make a type so that this property can be modified
	// instead of being read-only.
	return Script::getReadOnlyData( vScale );
}

/**
 *	Specialised set method for the 'scale' attribute
 */
int PyModel::pySet_scale( PyObject * value )
{
	BW_GUARD;
	Matrix	wM( this->worldTransform() );
	Vector3	newScale;
	int ret = Script::setData( value, newScale, "scale" );
	if (ret == 0)
	{
		Vector3	vScale(
			wM.applyToUnitAxisVector(0).length(),
			wM.applyToUnitAxisVector(1).length(),
			wM.applyToUnitAxisVector(2).length() );

		Matrix	scaler;
		scaler.setScale(
			newScale[0]/vScale[0],
			newScale[1]/vScale[1],
			newScale[2]/vScale[2] );

		wM.preMultiply( scaler );
		this->worldTransform( wM );
	}
	return ret;
}


/**
 *	Specialised get method for the 'yaw' attribute
 */
float PyModel::yaw() const
{
	BW_GUARD;
	const Matrix & wM = this->worldTransform();
	return atan2f( wM.m[2][0], wM.m[2][2] );
}

/**
 *	Specialised set method for the 'yaw' attribute
 */
void PyModel::yaw( float yaw )
{
	BW_GUARD;
	Matrix	wM = this->worldTransform();
	wM.preRotateY( yaw - atan2f( wM.m[2][0], wM.m[2][2] ) );
	this->worldTransform( wM );
}


/**
 *	Specialised get method for the 'height' attribute
 */
float PyModel::height() const
{
	BW_GUARD;
	float height = height_;

	if (height_ <= 0.f)
	{
		if (pSuperModel_ != NULL)
		{
			BoundingBox bb( Vector3::zero(), Vector3::zero() );
			pSuperModel_->localBoundingBox( bb );
			height = bb.maxBounds().y - bb.minBounds().y;
		}
		else
		{
			height = 1.f;
		}
	}

	return height;
}

/**
 *	Specialised set method for the 'height' attribute. This does nothing at
 *	the moment.
 */
void PyModel::height( float height )
{
	BW_GUARD;
	height_ = height;
}



/**
 *	Simple function to turn a python sequence of strings into a vector
 *	of nodes, after checking that they're in the model.
 *	Yes, a get/setData could be made out of this but I can't be bothered.
 */
int findNodes( PyObject * pSeq, SuperModel * pSM,
	std::vector<Moo::NodePtr> & nodes, const char * varName )
{
	BW_GUARD;
	nodes.clear();

	int sz = PySequence_Size( pSeq );
	for (int i = 0; i < sz; i++)
	{
		std::string varNamePlus = varName;
		char nidbuf[32];
		bw_snprintf( nidbuf, sizeof( nidbuf ), " element %d", i );
		varNamePlus += nidbuf;

		std::string nodeName;
		if (Script::setData( &*SmartPointer<PyObject>( PySequence_GetItem(
			pSeq, i ), true ), nodeName, varNamePlus.c_str() ) != 0)
				return -1;

		Moo::NodePtr anode = pSM->findNode( nodeName );
		if (!anode)
		{
			PyErr_Format( PyExc_ValueError,
				"%s setter can't find node %s", varName, nodeName.c_str() );
			return -1;
		}
		nodes.push_back( anode );
	}

	return 0;
}



/**
 *	Model bounding box matrix provider class
 */
class ModelBBMProv : public MatrixProvider
{
public:
	ModelBBMProv( PyModel * pModel ) :
	  MatrixProvider( false, &s_type_ ),
	  pModel_( pModel ) { }

	~ModelBBMProv() { }

	virtual void matrix( Matrix & m ) const
	{
		BW_GUARD;
		if (!pModel_->isInWorld())
		{
			m = Matrix::identity;
			m[3][3] = 0.f;
			return;
		}

		BoundingBox bb = BoundingBox::s_insideOut_;
		pModel_->localBoundingBox( bb, true );
		m = pModel_->worldTransform();

		Vector3 bbMin = m.applyPoint( bb.minBounds() );
		m.translation( bbMin );

		Vector3 bbMax = bb.maxBounds() - bb.minBounds();
		m[0] *= bbMax.x;
		m[1] *= bbMax.y;
		m[2] *= bbMax.z;
	}

private:
	SmartPointer<PyModel>	pModel_;
};


/**
 *	Get the bounding box in a matrix provider
 */
PyObject * PyModel::pyGet_bounds()
{
	BW_GUARD;
	return new ModelBBMProv( this );
}


static Moo::NodePtr s_globalRootNode = NULL;
/**
 *	Helper function to get the unchanging scene root node
 */
static Moo::NodePtr s_getRootNode()
{
	BW_GUARD;
	if (!s_globalRootNode)
	{
		s_globalRootNode = Moo::NodeCatalogue::find( "Scene Root" );
		// too dodgy to call SuperModel fn since not static 'tho could be
	}
	return s_globalRootNode;
}

/*~ function PyModel.node
 *
 *	Returns the named node.  If no name is specified, the root node is
 *  returned.  If the named node doesn't exist then an exception is generated.
 *
 *	@param	nodeName	(optional) A string specifying the name of the node
 *						to lookup.  If not supplied the root node is assumed.
 *	@param	local		(optional) A local transform for the node.  This is a
 *						matrix provider that premultiplies itself with the
 *						node's world transform, effectively creating a new
 *						node to attach things to.
 *
 *	@return				The node with the specified name, or None if the node
 *						doesn't exist.
 */
/**
 *	Get the given node.
 *
 *	Note: Returns a new reference!
 */
PyObject * PyModel::node( const std::string & nodeName,
						 MatrixProviderPtr local )
{
	BW_GUARD;
	Moo::NodePtr pNode;

	if (nodeName.empty())
	{
		pNode = s_getRootNode();
		if (!pNode)
		{
			PyErr_SetString( PyExc_EnvironmentError,
				"Model.root on a blank model can only be accessed after loading"
				"a non-blank model (anywhere, at any time previously)." );
			return NULL;
		}
	}
	else
	{
		if (pSuperModel_ == NULL)
		{
			PyErr_SetString( PyExc_TypeError,
				"Model.node() is not supported for blank models" );
			return NULL;
		}

		pNode = pSuperModel_->findNode( nodeName );
		if (!pNode)
		{
			PyErr_Format( PyExc_ValueError, "PyModel.node: "
				"No node named %s in this Model.", nodeName.c_str() );
			return NULL;
		}
	}

	PyModelNode * pPyNode = this->accessNode( pNode, local );
	Py_INCREF( pPyNode );
	return pPyNode;
}


/**
 *	Access the given node, creating it if necessary.
 */
PyModelNode * PyModel::accessNode( Moo::NodePtr pNode, MatrixProviderPtr local )
{
	BW_GUARD;
	PyModelNodes::iterator nit;
	for (nit = knownNodes_.begin(); nit != knownNodes_.end(); nit++)
	{
		bool sameNode = (*nit)->pNode() == pNode;
		bool sameLocal = (*nit)->localTransform() == local;
		if ( sameNode && sameLocal ) break;
	}
	if (nit == knownNodes_.end())
	{
		knownNodes_.push_back( new PyModelNode( this, pNode, local ) );
		nit = knownNodes_.end() - 1;
	}
	return *nit;
}


/*~ function PyModel.cue
 *	This method is not yet supported.
 *
 *	This method sets the default response for the given cue identifier.
 *
 *	A cue is a named frame inside an animation, that is exported with
 *	the model.  If this function is used to set up a callback function
 *	then anytime that named frame gets played, the function will be
 *	called.
 *
 *	For example, assuming that there was a cue named Impact in the swordSwing
 *	animation, which is played using the SwordSwing ActionQueuer, then the
 *	following example will print "Sword hit target" at that point in the
 *	animation.
 *
 *	@{
 *	def onImpact( t ):  print "Sword hit target at time", t
 *	biped.cue( "Impact", onImpact )
 *	biped.SwordSwing()
 *	@}
 *
 *	It raises a TypeError if the given function parameter is not callable. It
 *	raises a ValueError if the identifier is unknown.
 *
 *	@param	identifier	A String.  The cue identifier we are setting a
 *						response for.
 *	@param	function	A callable object.  The callback for that cue
 *						identifier.
 *
 */
/**
 *	This method sets the default response for the given cue identifier.
 *	Sets a python exception on error.
 */
bool PyModel::cue( const std::string & identifier,
	SmartPointer<PyObject> function )
{
	return actionQueue_.cue( identifier, function );
}

/*~ function PyModel.saveActionQueue
 *
 *	This function is used to save the state of the pyModel's action queue.
 *	This useful when doing a change to the pyModel's supermodel which would involve
 *	reloading the supermodel and its animation and action states.
 *
 *	@param state DataSectionPtr to save the state to.
 */
PyObject * PyModel::py_saveActionQueue( PyObject * args )
{
	BW_GUARD;
	PyObject * pPyDS;
	if (!PyArg_ParseTuple( args, "O", &pPyDS) ||
		!PyDataSection::Check(pPyDS) )
	{
		PyErr_SetString( PyExc_TypeError, "PyModel.saveActionQueue() "
			"expects a PyDataSection" );
		return NULL;
	}

	DataSectionPtr state = static_cast<PyDataSection*>( pPyDS )->pSection();
	actionQueue_.saveActionState(state);

	Py_Return;
}

/*~ function PyModel.restoreActionQueue
 *
 *	This function is used to restore the state of the pyModel's action queue.
 *	This useful when doing a change to the pyModel's supermodel which would involve
 *	reloading the supermodel and its animation and action states.
 *
 *	@param state DataSectionPtr to restore the state from.
 *
 *	@return Returns True (1) whether the restore operation was able to completely restore the previous
 *			state of the action queue, False (0) otherwise. The restore operation may fail if an action that was in
 *			the actionQueue is no longer part of the refreshed supermodel.
 */
PyObject * PyModel::py_restoreActionQueue( PyObject * args )
{
	BW_GUARD;
	PyObject * pPyDS;
	if (!PyArg_ParseTuple( args, "O", &pPyDS) ||
		!PyDataSection::Check(pPyDS) )
	{
		PyErr_SetString( PyExc_TypeError, "PyModel.restoreActionQueue() "
			"expects a PyDataSection" );
		return NULL;
	}

	DataSectionPtr state = static_cast<PyDataSection*>( pPyDS )->pSection();
	return PyInt_FromLong( actionQueue_.restoreActionState( state, pSuperModel_, pOwnWorld_ ) );
}

/**
 *	Static python factory method
 */
PyObject * PyModel::pyNew( PyObject * args )
{
	BW_GUARD;
	// get the arguments
	std::vector< std::string > modelNames;
	int sz = PyTuple_Size( args );
	int i;
	for (i = 0; i < sz; i++)
	{
		PyObject * pItem = PyTuple_GetItem( args, i );	// borrowed
		if (!PyString_Check( pItem )) break;
		modelNames.push_back( PyString_AsString( pItem ) );
	}

	if (!i || i < sz)
	{
		PyErr_SetString( PyExc_TypeError,
			"Model(): expected a number (>0) of strings" );
		return NULL;
	}

	// ok, make the supermodel
	SuperModel * pSM = NULL;

	// unless a blank model is requested
	if (i != 1 || !modelNames[0].empty())
	{
		MEM_TRACE_BEGIN( modelNames[0] )

		pSM = new SuperModel( modelNames );
		if (!pSM->nModels() || pSM->nModels() != sz)
		{
			PyErr_Format( PyExc_ValueError, "Model(): "
				"Only found %d out of %d models requested",
				pSM->nModels(), sz );

			delete pSM;

			MEM_TRACE_END()
			return NULL;
		}

		MEM_TRACE_END()

		pSM->checkBB( false );
	}
	return new PyModel( pSM );
}

/*~ function PyModel.straighten
 *
 *	This method straightens a model, i.e. removes all rotations (including yaw)
 *  from it.  It also removes any scaling effects, leaving nothing but the
 *  translation part of the world matrix.
 *
 *	@return			None
 */
/**
 *	This method allows scripts to straighten a model,
 *	i.e. remove all rotations (including yaw) from it
 */
void PyModel::straighten()
{
	BW_GUARD;
	Vector3 offset = this->worldTransform().applyToOrigin();
	Matrix wM;
	wM.setTranslate( offset );
	this->worldTransform( wM );
}


/*~ function PyModel.rotate
 *
 *	This method rotates a model about an arbitrary axis (and optionally an
 *  arbitrary centre).  A centre may be specified, otherwise it defaults to
 *  (0,0,0).  The centre is relative to the origin of the model.
 *
 *	@param	angle		The angle to rotate the model through.
 *	@param	vAxis		A Vector3 axis about which to rotate the model.  This
 *						axis will pass through vCentre.
 *	@param	vCentre		A Vector3 through which vAxis passes, when the rotation
 *						about it is applied.  It defaults to (0,0,0).
 *
 *	@return				None
 */
/**
 *	This method allows scripts to rotate a model about an
 *	arbitrary axis (and optionally an arbitrary centre)
 */
void PyModel::rotate( float angle, const Vector3 & vAxis,
	const Vector3 & vCentre )
{
	BW_GUARD;
	Matrix wM = this->worldTransform();

	// push the centre through the transform
	Vector3 rotCentre;
	wM.applyVector( rotCentre, vCentre );

	// set the matrix position and remember the difference
	Vector3 offsetDiff = wM.applyToOrigin() - rotCentre;
	wM.translation( rotCentre );

	// make a quaternion for that rotation
	Quaternion rotor;
	rotor.fromAngleAxis( angle, vAxis );

	// turn it into a matrix and apply it to our world matrix
	Matrix poster;
	poster.setRotate( rotor );
	wM.postMultiply( poster );

	// and add back the model position
	wM.translation( wM.applyToOrigin() + offsetDiff );

	this->worldTransform( wM );
}


/*~ function PyModel.alignTriangle
 *
 *	This method aligns a model to the given triangle (specified as three points
 *  in world space, taken in a clockwise direction).  The resultant world
 *	matrix will be in the middle of the triangle, with its z-axis along the
 *	triangles normal.  If randomYaw is true, The model's y-axis will be random;
 *	otherwise it will be along the world's positive y-axis if the y component
 *	of the triangle's normal (in world space) is less than 0.95; otherwise
 *  along the world's positive x-axis.
 *
 *	For example:
 *	@{
 *	m.alignTriangle( (1, 0, 0), (-1, 0, 0), (0,0,1), 0)
 *	@}
 *	results in the model lying on its back, with its head pointing along the
 *	world's positive x-axis.
 *	Wheras
 *	@{
 *	m.alignTriangle( (1, 0, 0), (-1, 0, 0), (0,1,0), 0)
 *	@}
 *	results in the model standing upright, facing in the world's negative
 *	z-axis.
 *
 *	@param	vertex0		A Vector3 specifying the first point of the triangle to
 *						align to.
 *	@param	vertex1		A Vector3 specifying the second point of the triangle to
 *						align to.
 *	@param	vertex2		A Vector3 specifying the third point of the triangle to
 *						align to
 *	@param	randomYaw	A boolean specifying whether or not to apply a random
 *						yaw to the model.
 *
 *	@return				None
 */
/**
 *	This method allows scripts to align a model to the given triangle.
 *	Note the resultant matrix is in the middle of the triangle - you
 *	may want to post-set the position.
 *
 */
void PyModel::alignTriangle( const Vector3 & vertex0,
	const Vector3 & vertex1, const Vector3 & vertex2, bool randomYaw )
{
	BW_GUARD;
	Vector3 normal = (vertex2 - vertex0).crossProduct(vertex2 - vertex1);
	Vector3 center = (vertex0+vertex1+vertex2) / 3;

	Matrix wM = this->worldTransform();
	wM.lookAt( center, normal, fabsf(normal.y) < 0.95f ? Vector3(0.f,1.f,0.f) : Vector3(1.f,0.f,0.f) );
	wM.invert();

	if ( randomYaw )
	{
		Matrix rot;
		rot.setRotateZ( (float)(rand() % 360) * 0.01745f );
		wM.preMultiply( rot );
	}

	this->worldTransform( wM );
}


/*~ function PyModel.reflectOffTriangle
 *
 *	This method reflects the Vector3 specified by the fwd argument off the
 *  specified triangle, and places the model in the centre of the triangle
 *	facing in the direction of the reflected vector.  The model's vertical will
 *  be will be along the world's positive y-axis if the new unit forward vector
 *  has a world space y component less than 0.95, otherwise along the world's
 *  positive z-axis.
 *
 *
 *	@param	vertex0		A Vector3 specifying the first point, in world space,
 *						of the triangle to align to.
 *	@param	vertex1		A Vector3 specifying the second point, in world space,
 *						of the triangle to align to.
 *	@param	vertex2		A Vector3 specifying the third point, in world space,
 *						of the triangle to align to.
 *	@param	fwd			The vector, in world space, to reflect off the
 *						triangle, giving the new forward direction for the
 *						model.
 *
 *	@return				A Vector3, which is the new forward direction for the
 *						model in world space.
 */
/**
 *	This method allows scripts to align a model to the given triangle.
 *	Note the resultant matrix is in the middle of the triangle - you
 *	may want to post-set the position.
 */
Vector3 PyModel::reflectOffTriangle( const Vector3 & vertex0,
	const Vector3 & vertex1, const Vector3 & vertex2, const Vector3 & fwd )
{
	BW_GUARD;
	Vector3 edge[2];
	edge[0] = vertex2 - vertex0;
	edge[1] = vertex2 - vertex1;
	edge[0].normalise();
	edge[1].normalise();
	Vector3 normal = edge[0].crossProduct(edge[1]);
	Vector3 center = (vertex0+vertex1+vertex2) / 3;

	//now we have the normal, calc the reflection vector
	Vector3 reflection = (2.f*normal.dotProduct(fwd))*normal - fwd;

	Matrix wM = this->worldTransform();

	wM.lookAt( center, reflection, fabsf(reflection.y) < 0.95f ? Vector3(0.f,1.f,0.f) : Vector3(1.f,0.f,0.f) );
	wM.invert();

	this->worldTransform( wM );

	return reflection;
}



/*~ function PyModel.zoomExtents
 *
 *	This method places the model at the origin, scaled to fit within a 1 metre
 *  cube; that is, its longest dimension will be exactly 1 metre.  Its
 *	placement within the cube depends on upperFrontFlushness. If
 *	upperFrontFlushness is non-zero it moves the model to be flush with the top
 *  front of the cube, rather than the bottom.
 *
 *	You have the option of placing a model in a box is wider than it is tall.
 *	To do this, set the xzMultiplier to the (width / height) of the box.
 *
 *	@param	upperFrontFlushness	Whether to align with the top-front or the
 *			bottom-rear.
 *	@param	xzMultiplier ( default 1.0 ) for placing a model in a non-cubic box.
 *
 *	@return						None
 */
/**
 *	This method allows scripts to place a model at the origin,
 *	with a size equalling 1 metre cubed.
 */
void PyModel::zoomExtents( bool upperFrontFlushness, float xzMutiplier )
{
	BW_GUARD;
	BoundingBox bb = BoundingBox::s_insideOut_;
	this->localBoundingBox( bb, true );
	Vector3 extents( bb.maxBounds() - bb.minBounds() );
	//the xzMultiplier is a fudge that allows zoomExtents to deal with
	//non-cubic bounds
	extents.x /= xzMutiplier;
	extents.z /= xzMutiplier;
	float size = max( max( extents.z, extents.y ), extents.x );
	Matrix sc;
	sc.setScale( 1.f / size, 1.f / size, 1.f / size );
	Matrix tr;
	tr.setTranslate( Vector3(-bb.centre()) );
	tr.postMultiply( sc );

	if ( upperFrontFlushness )
	{
		//find half of the scaled size of the object.
		extents *= (1.f / (2.f*size));
		//find the smallest move out of y,z
		float move = min( 0.5f-extents.y,0.5f-extents.z );
		//and move it to the front/top of the unit cube.
		tr.postTranslateBy( Vector3(0,move,-move) );
		//note we find the min, because if we don't move it
		//exactly towards the camera, the resultant picture will
		//not be centered.
	}

	this->worldTransform( tr );
}

/*~ function PyModel.action
 *
 *	This method looks up an action by name from an internal list of actions and
 *  returns an ActionQueuer object if it finds it.  Otherwise a Python type error is thrown.
 *
 *	@param	actionName		A string which is the name of the action to search
 *							for.
 *
 *	@return					The ActionQueuer object named actionName.
 */
/**
 *	This method looks up an action by name and returns an ActionQueuer object
 *	if it finds it
 */
PyObject * PyModel::py_action( PyObject * args )
{
	BW_GUARD;
	// get the name
	char * actionName;
	if (!PyArg_ParseTuple( args, "s", &actionName ))
	{
		PyErr_SetString( PyExc_TypeError,
			"PyModel.action: Could not parse the arguments\n" );
		return NULL;
	}

	if (pSuperModel_)
	{
		// find the action
		SuperModelActionPtr smap = pSuperModel_->getAction( actionName );
		if (smap)
		{
			return new ActionQueuer( this, smap, NULL );
		}
	}

	// set the error if we couldn't find it
	PyErr_Format( PyExc_ValueError, "Model.action() "
		"could not find action %s", actionName );
	return NULL;
}


#if FMOD_SUPPORT
PySound* PyModel::createSoundInternal( const std::string& s )
{
	BW_GUARD;
	if (s.empty())
	{
		PyErr_Format( PyExc_ValueError,
			"Can't play unnamed sound" );
		return NULL;
	}

	if (!inWorld_)
	{
		PyErr_Format( PyExc_ValueError,
			"Can't attach sounds when not in the world" );
		return NULL;
	}

    PySound *pySound = static_cast< PySound* >( SoundManager::instance().createPySound( s ) );
    
    if ( pySound != SoundManager::pyError() )
    {
        this->attachSound( pySound );
        pySound->setModel( this );
    }
	else
	{
		Py_DECREF( pySound );
		PyErr_Format( PyExc_ValueError,
			"Can't create sound named %s", s.c_str() );

		return NULL;
	}

	return pySound;
}
#endif // FMOD_SUPPORT

/*~ function PyModel.getSound
 *
 *	Returns a PySound for a 3D sound event at the location of the model.  The
 *	sound can subsequently be played by calling its play() method.
 *
 *  For sound naming semantics, please see FMOD.playSound().
 *
 *	@param	s				The name of the sound
 *
 *	@return					The PySound for this sound event
 */
PyObject* PyModel::getSound( const std::string& s )
{
	BW_GUARD;
#if FMOD_SUPPORT	
    PySound * pySound = createSoundInternal( s );    
	return pySound;
#else
	ERROR_MSG( "PyModel::getSound: FMOD sound support is disabled.\n" );
	Py_RETURN_NONE;
#endif
}


/*~ function PyModel.playSound
 *
 *	Plays a 3D sound at the location of the model.
 *
 *  For sound naming semantics, please see FMOD.playSound().
 *
 *	@param	s				The name of the sound
 *
 *	@return					The PySound for this sound event
 */
PyObject* PyModel::playSound( const std::string& s )
{
	BW_GUARD;

#if FMOD_SUPPORT
	PySound *pySound = this->createSoundInternal( s );

	if ( pySound && !pySound->play() )
	{
        PyErr_Format( PyExc_StandardError, "Model.playSound() "
				"Error starting sound %s", s.c_str() );
		Py_DECREF( pySound );
		return NULL;
	}

	return pySound;
#else
	ERROR_MSG( "PyModel::playSound: FMOD sound support is disabled.\n" );
	Py_RETURN_NONE;
#endif
}

#if FMOD_SUPPORT
/**
 *  Returns a reference to this model's sound event list, instantiating first if
 *  necessary.
 */
PySoundList& PyModel::sounds()
{
	BW_GUARD;
	if (pSounds_ == NULL)
		pSounds_ = new PySoundList();

	return *pSounds_;
}


/**
 *  Add the given sound event to this model's sound event list and set its 3D
 *  properties to correspond to this model.
 */
bool PyModel::attachSound( PySound *pySound )
{
	BW_GUARD;
	if ( pySound->update( this->worldTransform().applyToOrigin() ) )
	{
		this->sounds().push_back( pySound );
		return true;
	}
	else
		return false;
}


void PyModel::cleanupSounds()
{
	BW_GUARD;
	if (pSounds_)
	{
		pSounds_->stopOnDestroy( cleanupSounds_ );
		delete pSounds_;
		pSounds_ = NULL;
	}
}
#endif // FMOD_SUPPORT

/*~ function PyModel.addMotor
 *
 *	This method adds the specified object to the model's list of Motors.
 *	All motors in this list are able to influence the movement and animation of
 *  the model based on the state of the entity which owns it.
 *
 *	@param	motor	A Motor which is to be added to the model
 *
 *	@return			None
 */
/**
 *	This method allows scripts to add a motor to a model.
 */
PyObject * PyModel::py_addMotor( PyObject * args )
{
	BW_GUARD;
	// parse args
	PyObject * pObject;
	if ( !PyArg_ParseTuple( args, "O", &pObject ) || !Motor::Check( pObject ) )
	{
		PyErr_SetString( PyExc_TypeError, "Model.addMotor() expects a Motor" );
		return NULL;
	}

	Motor * pMotor = (Motor*)pObject;

	// check that no-one owns it
	if (pMotor->pOwner() != NULL)
	{
		PyErr_SetString( PyExc_ValueError, "Model.addMotor() "
			"cannot add a Motor is already attached to a Model" );
		return NULL;
	}

	// take it over
	Py_INCREF( pMotor );
	pMotor->attach( this );
	motors_.push_back( pMotor );

	Py_Return;
}


/*~ function PyModel.delMotor
 *
 *	This method removes the specified object to the models list of Motors.
 *	All motors in this list are able to influence the movement and animation of
 *  the model based on the state of the entity which owns it.
 *
 *	@param	motor	A Motor which is to be removed from the model
 *
 *	@return			None
 */
/**
 *	This method allows scripts to remove a motor from a model.
 */
PyObject * PyModel::py_delMotor( PyObject * args )
{
	BW_GUARD;
	// parse args
	PyObject * pObject;
	if ( !PyArg_ParseTuple( args, "O", &pObject ) || !Motor::Check( pObject ) )
	{
		PyErr_SetString( PyExc_TypeError, "Model.delMotor() expects a Motor" );
		return NULL;
	}

	Motor * pMotor = (Motor*)pObject;

	// check that we own it
	if (pMotor->pOwner() != this)
	{
		PyErr_SetString( PyExc_ValueError, "Model.delMotor() "
			"was given a Motor that is not attached to this Model" );
		return NULL;
	}

	// remove it from our list
	for (uint i = 0; i < motors_.size(); i++)
	{
		if (motors_[i] == pMotor)
		{
			motors_.erase( motors_.begin() + i );
			i--;	// continue for safety (would use std::find o/wise)
		}
	}

	// and get rid of it
	pMotor->detach();
	Py_DECREF( pMotor );

	Py_Return;
}

/**
 *	Get the tuple of motors
 */
PyObject * PyModel::pyGet_motors()
{
	BW_GUARD;
	PyObject * pTuple = PyTuple_New( motors_.size() );

	for (uint i = 0; i < motors_.size(); i++)
	{
		Py_INCREF( motors_[i] );
		PyTuple_SetItem( pTuple, i, motors_[i] );
	}

	return pTuple;
}


/**
 *	Set the sequence of motors
 */
int PyModel::pySet_motors( PyObject * value )
{
	BW_GUARD;
	// first check arguments...
	if (!PySequence_Check( value ))
	{
		PyErr_SetString( PyExc_TypeError,
			"Model.motors must be set to a sequence of Motors" );
		return -1;
	}

	// ... thoroughly
	bool bad = false;
	for (int i = 0; i < PySequence_Size( value ) && !bad; i++)
	{
		PyObject * pTry = PySequence_GetItem( value, i );

		if (!Motor::Check( pTry ))
		{
			PyErr_Format( PyExc_TypeError, "Element %d of sequence replacing "
				" Model.motors is not a Motor", i );
			bad = true;
		}
		else if (((Motor*)pTry)->pOwner() != NULL)
		{
			PyErr_Format( PyExc_ValueError, "Element %d of sequence replacing "
				"Model.motors is already attached to a Model", i );
			bad = true;
		}

		Py_DECREF( pTry );
	}
	if (bad) return -1;


	// let old motors go
	for (uint i = 0; i < motors_.size(); i++)
	{
		motors_[i]->detach();
		Py_DECREF( motors_[i] );
	}
	motors_.clear();

	// fit new ones
	for (int i = 0; i < PySequence_Size( value ) && !bad; i++)
	{
		Motor * pMotor = (Motor*)PySequence_GetItem( value, i );

		pMotor->attach( this );
		motors_.push_back( pMotor );
		// We keep the reference returned by PySequence_GetItem.
	}

	return 0;
}

/**
 *	This method gives models a chance to move during entity tick.
 *
 *	We can't move during scene tick because our movement might put
 *	us in a different node which would stuff up all the
 *	iterators. And it's nice to have all of these movements done at once.
 */
void PyModel::move( float dTime )
{
	BW_GUARD;
	// tell it where the sound source is (in case the motor triggers a sound)
	PyModel::pCurrent( this );

	// optionally call move on known nodes.  This is optional because doing
	// it on all pymodels and all nodes can take a measurable chunk of time.	
	if (moveAttachments_)
	{	
		PyModelNodes::iterator nit;
		for (nit = knownNodes_.begin(); nit != knownNodes_.end(); nit++)
		{
			(*nit)->move( dTime );
		}
	}	

	// purposely not using iterators for stability
	for (uint i = 0; i < motors_.size(); i++)
	{
		motors_[i]->rev( dTime );
	}

#if FMOD_SUPPORT
// 	// __glenc__: Disabling this, as tick() has this too
// 	if (pSounds_ &&
// 		!pSounds_->update( this->worldTransform().applyToOrigin() ))
// 	{
// 		ERROR_MSG( "PyModel::pySet_position: "
// 			"Failed to update sound events\n" );
// 	}
#endif

	PyModel::pCurrent( NULL );
}


static DogWatch	watchAQ("Action Queue");
static DogWatch watchTracker("Tracker");


/**
 * This method updates the model for this frame.  Uses the
 * underlying Action Queue to update
 *
 * @param dTime is the change in time for this frame
 */
void PyModel::tick( float dTime )
{
	BW_GUARD;
	if (!transformModsDirty_)
	{
		unitRotation_ = 0.f;
		unitOffset_.setZero();
	}

	watchAQ.start();
	actionQueue_.tick( actionScale()*dTime, this );
	watchAQ.stop();

	if (pSuperModel_ != NULL)
	{
		watchTracker.start();
		float lastLod = pSuperModel_->lastLod();
		for (Fashions::iterator it = fashions_.begin(); it != fashions_.end(); it++)
			it->second->tick( dTime, lastLod );
		watchTracker.stop();
	}

	PyModelNodes::reverse_iterator nit;
	for (nit = knownNodes_.rbegin(); nit != knownNodes_.rend(); nit++)
	{
		PyModelNode* node = *nit;

		node->tick( dTime );
		
		if (node->refCount() == 1 && !node->hasAttachments())
		{
			*nit = knownNodes_.back();
			knownNodes_.pop_back();
			node->detach();
			Py_DECREF( node );
		}
	}

#if FMOD_SUPPORT
	const Matrix& matrix = this->worldTransform();
	if (pSounds_ &&
		!pSounds_->update( this->worldTransform().applyToOrigin(), matrix[2], dTime))
	{
		ERROR_MSG( "PyModel::tick: "
			"Failed to update sound events\n" );
	}
#endif
}


/**
 *	This method draws the model.
 */
void PyModel::draw( const Matrix & worldTransform, float lod )
{
	BW_GUARD;
	if ((!this->visible()) && (!this->visibleAttachments())) return;

	if (Moo::rc().reflectionScene())
	{
		MF_ASSERT_DEV( WaterSceneRenderer::currentScene() );
		if ( WaterSceneRenderer::currentScene() && WaterSceneRenderer::currentScene()->shouldReflect( worldTransform.applyToOrigin(), this ) )
			WaterSceneRenderer::incReflectionCount();
		else
			return;
	}

	static DogWatch pyModelWatch( "PyModelDraw" );

	static int running = 0;

	if (running++ == 0)
		pyModelWatch.start();

	if (PyModel::pCurrent() == NULL)
		PyModel::pCurrent( this );

	Moo::rc().push();
	Moo::rc().world( worldTransform );

	bool didAnimate = this->animateAsAttachment();
	if (pCouplingFashion_ && !didAnimate)
		Moo::rc().preMultiply( pCouplingFashion_->staticInverse() );

	//turn on global rendering effects
	if (stipple_)
	{
		if (!s_pStipple)
		{
			s_pStipple = new StippleDrawOverride();
		}
		s_pStipple->begin();
	}
	else if (shimmer_)
	{
		if (!s_pShimmer.hasObject())
		{
			s_pShimmer = new ShimmerDrawOverride();
		}
		s_pShimmer->begin();
	}

	this->drawAsAttachment( didAnimate ? &*pCouplingFashion_ : NULL );

	// draw all our attachments
	this->drawAttachments();

	//turn off global rendering effects
	if (stipple_)
	{
		s_pStipple->end();
	}
	else if (shimmer_)
	{
		s_pShimmer->end();
	}

	if (pDebugInfo_ != NULL)
	{
		this->drawDebugInfo();
	}

	/*
	BoundingBox bb = boundingBox_;

	Matrix m = Moo::rc().world();
	m.postMultiply( Moo::rc().viewProjection() );
	bb.calculateOutcode( m );

	bool wantToDraw = (!bb.combinedOutcode());

	if (!wantToDraw && particleSystems_.size())
	{
		ParticlesVector::iterator pit = particleSystems_.begin();
		ParticlesVector::iterator pend = particleSystems_.end();
		while (pit != pend && !wantToDraw)
		{
			ParticleSystem* ps = (*pit++).getObject();

			if ( ps->size() > 0 )
			{
				const BoundingBox& bb = ps->boundingBox();
				bb.calculateOutcode( Moo::rc().viewProjection() );
				if ( !bb.combinedOutcode() )
					wantToDraw = true;
			}
		}
	}
	if (wantToDraw)
	{
		//float alpha = -1.f;
		//Matrix	objToCam;
		//objToCam.multiply( Moo::rc().world(), Moo::rc().view() );
	}
	else
	{
		// update our focalMatrix to our world transform if we didn't draw
		//focalMatrixInWS_ = worldTransform;
		// no point doing this as we won't even get 'draw' called
		// if our chunk doesn't draw - so it would be strange
		// and inconsistent.
	}
	*/

	Moo::rc().pop();

	if (PyModel::pCurrent() == this)
		PyModel::pCurrent( NULL );

	if (--running == 0)
		pyModelWatch.stop();
}


/**
 *	Accumulate our visibilty box into the given one
 */
void PyModel::worldVisibilityBox( BoundingBox & vbb, const Matrix& worldTransform, bool skinny )
{
	BW_GUARD;
	if ((!this->visible()) && (!this->visibleAttachments())) return;

	BoundingBox bb(localVisibilityBox_);
	bb.transformBy(worldTransform);
	vbb.addBounds(bb);

	if (skinny || knownNodes_.empty()) return;

	PyModelNodes::iterator nit;
	for (nit = knownNodes_.begin(); nit != knownNodes_.end(); nit++)
	{
		(*nit)->worldBoundingBox(vbb, worldTransform);
	}
}


/**
 *	Accumulate our bounding box into the given one
 */
void PyModel::worldBoundingBox( BoundingBox & wbb, const Matrix& worldTransform, bool skinny )
{
	BW_GUARD;	

	BoundingBox bb;
	this->localBoundingBoxInternal(bb);
	bb.transformBy(worldTransform);
	wbb.addBounds(bb);

	if (skinny || knownNodes_.empty()) return;

	PyModelNodes::iterator nit;
	for (nit = knownNodes_.begin(); nit != knownNodes_.end(); nit++)
	{
		(*nit)->worldBoundingBox(wbb, worldTransform);
	}
}


/**
 *	Accumulate our bounding box into the given one
 */
void PyModel::localBoundingBox( BoundingBox & lbb, bool skinny )
{
	BW_GUARD;

	BoundingBox bb;
	this->localBoundingBoxInternal(bb);
	lbb.addBounds(bb);

	if (skinny || knownNodes_.empty()) return;

	//Accumulate our children in world space
	Matrix world( this->worldTransform() );
	BoundingBox wbb;
	PyModelNodes::iterator nit;
	for (nit = knownNodes_.begin(); nit != knownNodes_.end(); nit++)
	{
		(*nit)->worldBoundingBox(wbb, this->worldTransform());
	}

	//then convert back to local space.
	if (!wbb.insideOut())
	{
		world.invert();
		wbb.transformBy(world);
		lbb.addBounds(wbb);
	}
}


void PyModel::localVisibilityBox( BoundingBox & lbb, bool skinny )
{	
	BW_GUARD;
	if ((!this->visible()) && (!this->visibleAttachments())) return;

	lbb.addBounds(localVisibilityBox_);

	if (skinny || knownNodes_.empty()) return;

	//Accumulate our children in world space, then convert back to local space.
	Matrix world( this->worldTransform() );
	BoundingBox wbb;
	PyModelNodes::iterator nit;
	for (nit = knownNodes_.begin(); nit != knownNodes_.end(); nit++)
	{
		(*nit)->worldBoundingBox(wbb, this->worldTransform());
	}
	if (!wbb.insideOut())
	{
		world.invert();
		wbb.transformBy(world);
		lbb.addBounds(wbb);
	}
}





bool PyModel::drawSkeletons_  = false;
bool PyModel::drawNodeLabels_ = false;

/*~ function BigWorld.debugModel
 *
 * This function is used to turn on/off the drawing of the models
 * skeleton. This can also be done through the watcher located in
 * Render/Draw Skeletons
 */
/**
 *	Python debug setting method
 */
void debugModel(bool b)
{
	PyModel::drawSkeletons_ = b;
}

PY_AUTO_MODULE_FUNCTION( RETVOID, debugModel, ARG( bool, END ), BigWorld )


/**
 *	draws the skeleton of this model
 */
void PyModel::drawSkeleton()
{
	BW_GUARD;	
#if ENABLE_DRAW_SKELETON

	if (!drawSkeletons_ && !shouldDrawSkeleton_)
	{
		return;
	}

	BoundingBox bb;
	this->localBoundingBoxInternal(bb);
	Matrix m = Moo::rc().world();
	m.postMultiply( Moo::rc().viewProjection() );
	bb.calculateOutcode( m );
	if (bb.combinedOutcode())
	{
		return;
	}

	Moo::rc().push();
	Moo::rc().world( Matrix::identity );

	// draw skeleton for each model
	for (int i=0; i<pSuperModel_->nModels(); ++i)
	{
		Model * model = pSuperModel_->curModel( i );
		if (model != NULL)
		{
			NodeTreeIterator it  = model->nodeTreeBegin();
			NodeTreeIterator end = model->nodeTreeEnd();
			while (it != end)
			{
				// skip leaf nodes
				std::string nodeName = it->pData->pNode->identifier();
				if ( ( nodeName.substr(0, 3) != std::string("HP_") ) && ( nodeName.find("BlendBone", 0) == nodeName.npos) )
				{
					Vector3	fromPos = it->pParentTransform->applyToOrigin();
					const Matrix & nodeTrans = it->pData->pNode->worldTransform();
					Vector3 toPos = nodeTrans.applyToOrigin();

					// draw arrow body
					Geometrics::drawLine( fromPos, toPos, 0x0000ff00 );

					// draw arrow head
					Vector3 vDir  = (toPos - fromPos);
					vDir.normalise();
					Vector3 ortog = vDir.crossProduct( nodeTrans[1] );
					ortog.normalise();
					static const float length = 0.02f;
					Geometrics::drawLine( toPos, toPos - vDir * length + ortog * length, 0x00ff0000 );
					Geometrics::drawLine( toPos, toPos - vDir * length - ortog * length, 0x00ff0000 );
				}
				it++;
			}
		}
	}
	Moo::rc().pop();

#endif
}

/**
 *	draws the labels of each node in this model's skeleton
 */
void PyModel::drawNodeLabels()
{
	BW_GUARD;
	if (!drawNodeLabels_)
	{
		return;
	}

	BoundingBox bb;
	this->localBoundingBoxInternal(bb);
	Matrix m = Moo::rc().world();
	m.postMultiply( Moo::rc().viewProjection() );
	bb.calculateOutcode( m );
	if (bb.combinedOutcode())
	{
		return;
	}

	Moo::rc().push();
	Moo::rc().world( Matrix::identity );

	Labels * nodesIDs = new Labels;
	for (int i=0; i<pSuperModel_->nModels(); ++i)
	{
		Model * model = pSuperModel_->curModel( i );
		if (model != NULL)
		{
			NodeTreeIterator it  = model->nodeTreeBegin();
			NodeTreeIterator end = model->nodeTreeEnd();
			while (it != end)
			{
				const Matrix & nodeTrans = it->pData->pNode->worldTransform();
				Vector3 nodePos = nodeTrans.applyToOrigin();
				nodesIDs->add(it->pData->pNode->identifier(), nodePos);
				it++;
			}
		}
	}
	Moo::SortedChannel::addDrawItem(nodesIDs);
	Moo::rc().pop();
}

/**
 *	draws the debug info of this model
 */
void PyModel::drawDebugInfo()
{
	BW_GUARD;
	if (pDebugInfo_ == NULL || pDebugInfo_->empty() || !Moo::VisualChannel::enabled())
	{
		return;
	}

	BoundingBox bb;
	this->localBoundingBoxInternal(bb);
	Matrix m = Moo::rc().world();
	m.postMultiply( Moo::rc().viewProjection() );
	bb.calculateOutcode( m );
	if (bb.combinedOutcode())
	{
		return;
	}

	Vector3 position = this->worldTransform().applyToOrigin() + 
						Vector3(0, this->height() + 0.1f, 0);

	StackedLabels * labels = new StackedLabels(position);
	
	for (DebugStrings::iterator i = pDebugInfo_->begin();
		i != pDebugInfo_->end();
		i++)
	{
		labels->add( std::string(i->first) + ": " + i->second );
	}

	pDebugInfo_->clear();

	Moo::rc().push();
	Moo::rc().world( Matrix::identity );

	Moo::SortedChannel::addDrawItem( labels );
	Moo::rc().pop();
}



/**
 * animates this model as a single attachment
 *
 * @return the result from actionQueue.effect(), which
 *  is true if the action queue did any animating
 */
bool PyModel::animateAsAttachment()
{
	BW_GUARD;
	// Get the action queue to do its stuff and animate the model
	watchAQ.start();
	bool ret = actionQueue_.effect();
	watchAQ.stop();

	return ret;
}

/**
 *	Draws this model as a single attachment,
 *	using a primed render context transform
 */
void PyModel::drawAsAttachment( Fashion * pCouplingFashion )
{
	BW_GUARD;
	// Figure out where	we are in the world	(for splodges and footprints)
	Matrix	worldMatrix;
	worldMatrix.setRotateY(	unitRotation_ );
	worldMatrix.postMultiply( Moo::rc().world()	);

	// Change the RenderContext's world transform appropriately
	Matrix	mOffset = unitTransform_;
	mOffset.postTranslateBy( unitOffset_ );
	mOffset.postRotateY( unitRotation_ );

	Moo::rc().preMultiply( mOffset );

	if (pSuperModel_)
	{
		FashionVector & fv = actionQueue_.fv();
		uint32 fvRegular = fv.size();

#ifndef _RELEASE
		uint32 fvFull = fvRegular + fashions_.size() + !!pCouplingFashion;
		if (fvFull > fv.capacity())
		{
			fv.resize( fvFull );
			fv.resize( fvRegular );
		}
#endif

		float lastLod = pSuperModel_->lastLod();

		Fashions::iterator fit;
		// Temporarily add all the dyes
		for (fit = fashions_.begin(); fit != fashions_.end(); fit++)
			if (fit->second->fashionEra() == PyFashion::EARLY)
				fv.push_back( fit->second->fashion() );

		// Temporarily add all the trackers
		int nLatecomers = 0;
		for (fit = fashions_.begin(); fit != fashions_.end(); fit++)
			if (fit->second->fashionEra() == PyFashion::LATE)
			{
				fv.push_back( fit->second->fashion() );
				nLatecomers++;
			}

		// And temporarily add any passed in fashion
		if (pCouplingFashion != NULL)
		{
			fv.push_back( pCouplingFashion );
			nLatecomers++;
		}

		// This object ID is setup for masking all the
		// dynamic objects to turn off the foam effect in the
		// water. This solution feels kinda hackish but a better
		// solution would require too many framework changes.
		// (best left to a future version).
		Moo::rc().currentObjectID(1.f);

		// Just draw the thing!
		pSuperModel_->draw( &fv, nLatecomers, -1.f, this->visible() );

		Moo::rc().currentObjectID(0.f);

		// Remove the trackers and dyes from the AQ's fashion vector
		fv.erase( fv.begin() + fvRegular, fv.end() );

		this->drawSkeleton();
		this->drawNodeLabels();
	}
	else
	{
		Moo::NodePtr root = s_getRootNode();
		if (root)
		{
			root->transform( Matrix::identity );
			root->traverse();
		}
	}

	// Update some state
	transformModsDirty_ = false;
}



/**
 *	Constructor
 */
ModelAligner::ModelAligner( SuperModel * pSuperModel, Moo::NodePtr pNode ) :
	pNode_( pNode )
{
	BW_GUARD;
	if (pSuperModel != NULL)
	{
		pSuperModel->dressInDefault();
		staticInverse_.invert( pNode->worldTransform() );
	}
}


/**
 *	Fashion dress method, only used when model is animated.
 */
void ModelAligner::dress( SuperModel & superModel )
{
	BW_GUARD;
	// first invert world transform of node
	Matrix m;
	m.invert( pNode_->worldTransform() );

	// now premultiply by world transform of root node
	// (could avoid by setting world to identity earlier...
	// but in the scope of things this is not our biggest problem)
	m.preMultiply( Moo::rc().world() );

	// now premultiply the moo world by this calculated transform
	Moo::rc().preMultiply( m );

	// and get the supermodel to retraverse itself
	superModel.redress();
}



/*~ function PyModel.origin
 *
 *	Make the named node the origin of the model's coordinate system, if a node
 *  with that name exists.  If it does, return 1, otherwise 0.
 *
 *	@param nodeName	The name of the node to make the origin
 *
 */
/**
 *	Make the given node the origin of the model's coordinate system.
 *	If there is no such node, then an error is logged and false is returned
 *
 *	@note	Sets python error when it returns false.
 */
bool PyModel::origin( const std::string & nodeName )
{
	BW_GUARD;
	if (pSuperModel_ == NULL)
	{
		PyErr_SetString( PyExc_TypeError,
			"Model.origin() is not supported for blank models" );
		return false;
	}

	Moo::NodePtr pNode = pSuperModel_->findNode( nodeName );
	if (!pNode)
	{
		PyErr_Format( PyExc_ValueError, "PyModel.origin(): "
			"No node named %s in this Model.", nodeName.c_str() );
		return false;
	}

	pCouplingFashion_ = NULL;

	if (nodeName != "Scene Root")
	{
		pCouplingFashion_ = new ModelAligner( pSuperModel_, pNode );
	}

	return true;
}


/**
 * Draws attached models.
 *
 * Uses the cached objectToCamera transform from the hard point node
 * to prime the render context before calling drawAttachment
 */
void PyModel::drawAttachments()
{
	BW_GUARD;
	float llod;
	if (pSuperModel_ != NULL)
	{
		llod = pSuperModel_->lastLod();
	}
	else
	{
		llod = 0.f;					// TODO: Get lod for blank models too
		if (!knownNodes_.empty())	// traverse this node for blank models
		{
			Moo::NodePtr pSceneRoot = s_getRootNode();
			pSceneRoot->blendClobber( Model::blendCookie(), Matrix::identity );
			pSceneRoot->visitSelf( Moo::rc().world() );
		}
	}
	PyModelNodes::iterator nit;
	for (nit = knownNodes_.begin(); nit != knownNodes_.end(); nit++)
		(*nit)->latch();
	for (nit = knownNodes_.begin(); nit != knownNodes_.end(); nit++)
		(*nit)->draw( llod );
}



/**
 *	This method adds this model to the scene of the given subspace
 *	(Used to be handled by ModelManager. Soon to be renamed)
 */
/*
void PyModel::addToScene( SubSpace * pSubSpace )
{
	this->enterSpace( &pSubSpace->space() );
	this->enterWorld( pSubSpace );
}
*/

/**
 *	This method removes this model from the scene it is in
 *	(Used to be handled by ModelManager. Soon to be renamed)
 */
/*
void PyModel::removeFromScene()
{
	this->leaveSpace();
	this->leaveWorld();
}
*/

/**
 *	This function lets the model know that it's now in the
 *	world and thus its tick function will start being called.
 */
void PyModel::enterWorld()
{
	BW_GUARD;
	this->PyAttachment::enterWorld();

	// flush out the action queue
	actionQueue().flush();

	// tell all our attachments the same
	for (PyModelNodes::iterator nit = knownNodes_.begin();
		nit != knownNodes_.end();
		nit++)
	{
		(*nit)->enterWorld();
	}
}



/**
 *	This function lets the model know that it's no longer in the
 *	world and thus its tick function will stop being called.
 */
void PyModel::leaveWorld()
{
	BW_GUARD;
	this->PyAttachment::leaveWorld();

	// flush out the action queue
	actionQueue().flush();

	// tell all our attachments the same
	for (PyModelNodes::iterator nit = knownNodes_.begin();
		nit != knownNodes_.end();
		nit++)
	{
		(*nit)->leaveWorld();
	}

#if FMOD_SUPPORT
	// stop all sounds
	this->cleanupSounds();
#endif
}


/**
 *	This method retrieves our local bounding box, adjusting for
 *	the python-overridable height attribute.
 */
void PyModel::localBoundingBoxInternal( BoundingBox& bb )
{
	if (pSuperModel_ != NULL)
		pSuperModel_->localBoundingBox( bb );
	else
		bb = localBoundingBox_;

	if (height_ > 0.f)
	{
		Vector3 maxBounds = bb.maxBounds();
		maxBounds.y = bb.minBounds().y + height_;
		bb.setBounds( bb.minBounds(), maxBounds );
	}
}




static WeakPyPtr<PyModel> s_dpm;

/**
 *	Static method to do any debug stuff
 */
void PyModel::debugStuff( float dTime )
{
	BW_GUARD;	
#if ENABLE_ACTION_QUEUE_DEBUGGER
	if (s_dpm.hasObject())
	{
		PyModel* model = s_dpm.get();

		if (!model)
		{
			s_dpm = NULL;

			return;
		}
		
		if (!model->isInWorld())
		{
			return;
		}

		model->actionQueue().debugTick( dTime );
		model->actionQueue().debugDraw();
	}
#endif
}

/*~ function BigWorld.debugAQ
 *
 *	This function controls the Action Queue debugging graph.  It takes one
 *	argument which is either a PyModel, or None.  None switches the debugging
 *	graph off.  If the argument is a model, then a graph is displayed on
 *	the screen.  It is a line graph, which shows what actions are currently
 *	playing on that model, and what blend weight each action has.
 *
 *	Each line represents one currently playing action.  It will have an
 *	arbitrary colour assigned to it, and have the Actions name printed below
 *	it in the same colour.  The height of the line represents what percentage
 *	of the total blended Actions this Action makes up.
 *
 *	If one model is currently being graphed, and this function is called again
 *	with another model, the graph for the first model stops, and the new model
 *	takes over.
 */
/**
 *	Python debug setting method
 */
static void debugAQ( SmartPointer<PyModel> pm )
{
	BW_GUARD;	
#if ENABLE_ACTION_QUEUE_DEBUGGER
	if (pm)
	{
		pm->actionQueue().debugTick( -1.f );
		s_dpm = pm.getObject();
	}
	else
	{
		s_dpm = NULL;
	}
#else
	ERROR_MSG("Action Queue Debugger is disabled");
#endif
}
PY_AUTO_MODULE_FUNCTION(
	RETVOID, debugAQ, ARG( SmartPointer<PyModel>, END ), BigWorld )

// pymodel.cpp
