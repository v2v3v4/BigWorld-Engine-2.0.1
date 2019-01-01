/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ACTION_QUEUE_HPP
#define ACTION_QUEUE_HPP


#include <vector>
#include "moo/moo_math.hpp"
#include "moo/animation.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "model/model_action.hpp"
#include "model/model_animation.hpp"
#include "model/super_model.hpp"
#include "model/super_model_action.hpp"
#include "moo/cue_channel.hpp"

class PyModel;
class MatrixLiaison;

namespace Moo
{
	class CueBase;
	typedef SmartPointer<CueBase> CueBasePtr;
}

class CueResponse;
typedef std::vector<CueResponse> CueResponses;

/**
 *	This class determines what our response to a given cue will be
 */
class CueResponse
{
public:
	CueResponse( Moo::CueBasePtr cueBase, PyObject * function );

	static void respond( uint firstCue, const CueResponses & responses,
		class PyModel *pModel );

	void respond( float missedBy, const std::basic_string<float> * args ) const;

	Moo::CueBasePtr			cueBase_;
	SmartPointer<PyObject>	function_;
};



/**
 *	This class coordinates and controls the queue of actions
 *	in effect on a Model. It is split to do just the necessary time
 *	management (and script callback scheduling) in its tick function,
 *	with the actual animation of the Model's visual occuring only in
 *	the effect function (for efficiency when Models are not drawn)
 */
class ActionQueue
{
public:
	ActionQueue();
	~ActionQueue();

	// Queue management

	void flush();

	void saveActionState( DataSectionPtr state );
	bool restoreActionState( DataSectionPtr state, SuperModel* pSuperModel, MatrixLiaison* pMatrix );
	
	void add( SuperModelActionPtr action, float afterTime = 0.f,
		CueResponses * responses = NULL, bool promoteMotion = false,
		SuperModelActionPtr link = NULL );

	void stop( const ModelAction * pActionSource );

	void setMatch( SuperModelActionPtr action, float dTime = 0.f, float actionTime = -1.f,
		bool promoteMotion = false );

	bool lastMatchAccepted()		{ return lastMatchAccepted_; }

	Vector3 lastPromotion()			{ return lastPromotion_; }
	Angle lastYawPromotion()		{ return lastYawPromotion_; }

	float matchCycleCount();

	FashionVector & fv()			{ return fashionState_; }
	void getCurrentActions( std::vector<SuperModelActionPtr>& actionsInUse );

	// Queue processing

	void tick( float dTime, PyModel* pModel );
	void tick( float dTime, SuperModel* supermodel, MatrixLiaison* pMatrix );

	bool effect();

	bool cue( const std::string & identifier, SmartPointer<PyObject> function );

	PyObject * queueState() const;

private:
	/**
	 *	This class maintains auxilliary info about an action ptr
	 */
	struct AQElement
	{
		SuperModelActionPtr	action_;
		CueResponses *		responses_;
		bool				promoteMotion_;
		AQElement *			link_;

		AQElement() :
			responses_( NULL ),
			link_( NULL )
		{
		}

		AQElement( SuperModelActionPtr a, CueResponses * r, bool p ) :
			action_( a ),
			responses_( r ),
			promoteMotion_( p ),
			link_( NULL )
		{
		}
		AQElement( const AQElement & other ) :
			action_( other.action_ ),
			responses_( NULL ),
			promoteMotion_( other.promoteMotion_ ),
			link_( NULL )
		{
			if (other.responses_ != NULL)
				responses_ = new CueResponses( *other.responses_ );
			if (other.link_ != NULL)
				link_ = new AQElement( *other.link_ );
		}
		AQElement & operator=( const AQElement & other )
		{
			if (&other != this)
			{
				CueResponses * oresponses = responses_;
				AQElement * olink = link_;
				action_ = other.action_;
				responses_ = NULL;
				promoteMotion_ = other.promoteMotion_;
				link_ = NULL;
				if (other.responses_ != NULL)
					responses_ = new CueResponses( *other.responses_ );
				if (other.link_ != NULL)
					link_ = new AQElement( *other.link_ );
				if (oresponses != NULL) delete oresponses;
				if (olink != NULL) delete olink;
			}
			return *this;
		}
		~AQElement()
		{
			if (responses_ != NULL) delete responses_;
			if (link_ != NULL) delete link_;
		}

		void doCallback( float missedBy );
		void followLink();
	};
	typedef std::vector<AQElement>	AQElements;
	AQElements			waiting_;
	AQElements			actionState_;
	void saveAQElement( AQElement& aqe, DataSectionPtr actionState );

	FashionVector		fashionState_;

	float				matchActionTime_;
	bool				lastMatchAccepted_;

	void endPlayingAction( AQElement & aqe, float dTime, float endat, SuperModel* supermodel, MatrixLiaison* pMatrix );
	void tickPromotion( AQElement & aqe, float timeNow, MatrixLiaison* pMatrix );
	bool mollifyWorldTransform( MatrixLiaison* pMatrix );

	void addMatchedActionPlaceholder();

	Vector3				lastPromotion_;
	Angle				lastYawPromotion_;

	CueResponses		responses_;

	bool				needsMollifying_;

public:
	void debugTick( float dTime );
	void debugDraw();
};




/*~ class BigWorld.ActionQueuer
 *
 *	An ActionQueuer is an action that can be queued on a model.  It has many
 *  read only attributes which are defined in the .model file.  An
 *	ActionQueuer can be obtained by either calling the PyModel.action function,
 *  or treating the name of the action as a property of a PyModel.
 *	The only two methods that actually effect the state of the ActionQueuer are
 *  ActionQueuer.stop and calling the actionqueuer as a function object, which
 *  makes it play.
 *
 *	Actions play on a track, that is specified in the .model file.
 *	Actions that are on different tracks play concurrently.  Actions that
 *	are on the same track either overwrite the current action, or queue up
 *	behind it.  Actions can be queued by calling the ActionQueuer through
 *	an ActionQueuer that is either queued or playing.
 *
 *	This class inherits from PyFashion.
 *
 *	Example:
 *	@{
 *	model = BigWorld.Model( "objects/models/characters/biped.model" )
 *	aq1 = model.action( "Walk" )
 *	aq2 = model.Walk
 *	# Both aq1 and aq2 have the Walk ActionQueuer.
 *
 *	# now we play the walk action though aq1, and queue it up to play
 *	# using aq2 once aq1 has finished.
 *	aq1().aq2()
 *	@}
 *
 */
/**
 *	This class implements a simple helper Python type. Objects of this type are
 *	used to represent actions that the client can queue on a model.
 */
class ActionQueuer : public PyObjectPlus
{
	Py_Header( ActionQueuer, PyObjectPlus );

public:
	ActionQueuer( SmartPointer<PyModel> model, SuperModelActionPtr action,
		SuperModelActionPtr link, PyTypePlus * pType = &ActionQueuer::s_type_ );

	~ActionQueuer();

	PY_KEYWORD_METHOD_DECLARE_WITH_DOC( pyCall, "This method is called when the function object is called from Python" );

	PY_METHOD_DECLARE_WITH_DOC( py_stop, "This function stops all instances of the input action on the queue" );

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PyObject * action( const std::string & aname, bool errIfNotFound = true );
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETOWN, action, ARG( std::string, END ), "This function returns the specified action as an ActionQueuer object." )


	PY_RO_ATTRIBUTE_DECLARE( daction()->pFirstAnim_->duration_, duration );
	PY_RO_ATTRIBUTE_DECLARE( daction()->pSource_->blendInTime_, blendInTime );
	PY_RO_ATTRIBUTE_DECLARE( daction()->pSource_->blendOutTime_, blendOutTime );
	PY_RO_ATTRIBUTE_DECLARE( daction()->pSource_->track_, track );
	PY_RO_ATTRIBUTE_DECLARE( daction()->pSource_->filler_, filler );
	PY_RO_ATTRIBUTE_DECLARE( (daction()->pSource_->track_ == -1), blended );

	PyObject * pyGet_displacement();
	PY_RO_ATTRIBUTE_SET( displacement );

	PyObject * pyGet_rotation();
	PY_RO_ATTRIBUTE_SET( rotation );

	PY_METHOD_DECLARE_WITH_DOC( py_getSeek, " This method returns the seek position for another entity wishing to "
											"co-ordinate using this action on the provided position with this "
											"action's counterpart on this model." );
	PyObject * pyGet_seek();
	PY_RO_ATTRIBUTE_SET( seek );

	PY_METHOD_DECLARE_WITH_DOC( py_getSeekInv, "This method returns the seek position for another entity"
											   "wishing to co-ordinate using this action's counterpart on"
											   "this position with this action on its own model.");
	PyObject * pyGet_seekInv();
	PY_RO_ATTRIBUTE_SET( seekInv );

	PyObject * pyGet_impact();
	PY_RO_ATTRIBUTE_SET( impact );

	SuperModelActionPtr		daction()
		{ return action_ ? action_ : link_; }

private:
	SmartPointer<PyModel>	model_;
	SuperModelActionPtr		action_;
	SuperModelActionPtr		link_;

	Vector4 getSeek( const Matrix & worldIn, bool invert );
};

#endif
