/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYMODEL_HPP
#define PYMODEL_HPP

#pragma warning( disable:4786 )


#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script_math.hpp"
#include "pyscript/stl_to_py.hpp"

#include "py_attachment.hpp"
#include "action_queue.hpp"
#include "cstdmf/bgtask_manager.hpp"

namespace Moo
{
	class Node;
	typedef SmartPointer<Node> NodePtr;
}

class Attachment;
class SuperModel;
class PyFxSound;
class SoundSource;
class PyFashion;
class Motor;
class SubSpace;

class PySound;
class PySoundList;

#include "fmodsound/py_sound.hpp"
#include "math/boundbox.hpp"
#include "math/matrix.hpp"

class PyModelNode;
typedef std::vector<PyModelNode*>	PyModelNodes;

class PyModel;
typedef SmartPointer<PyModel> PyModelPtr;

/**
 *	This class properly aligns an attachment whose hardpoint node is
 *	animated (either directly or indirectly by parents moving)
 */
class ModelAligner : public Fashion, public Aligned
{
public:
	ModelAligner( SuperModel * pSuperModel, Moo::NodePtr pNode );

	const Matrix & staticInverse() const	{ return staticInverse_; }

private:
	virtual void dress( SuperModel & superModel );

	Moo::NodePtr	pNode_;
	Matrix			staticInverse_;
};

typedef SmartPointer<ModelAligner> ModelAlignerPtr;

/**
 *	Helper class to create pyModel for scheduled model loading
 */
class ScheduledModelLoader : public BackgroundTask
{
public:
	ScheduledModelLoader( PyObject * bgLoadCallback,
			const std::vector< std::string > & modelNames );

	virtual void doBackgroundTask( BgTaskManager & mgr );
	virtual void doMainThreadTask( BgTaskManager & mgr );

private:
	PyObjectPtr pBgLoadCallback_;
	SuperModel * pSuperModel_;
	std::vector< std::string >	modelNames_;
};

/*~ class BigWorld.PyModel
 *
 *	PyModels are renderable objects which can be moved around the world,
 *  animated, dyed (have materials programatically assigned) and attached to.
 *
 *	PyModels are constructed using the BigWorld.Model function, which takes the
 *  resource id of one or more .model files. Each of these individual models is
 *  loaded, and combined into a supermodel, which is the PyModel.
 *
 *	For example, to load a body and a head model into one supermodel:
 *
 *	@{
 *	model = BigWorld.Model( "models/body.model", "models/head.model" )
 *	@}
 *
 *	Models which are attached to entities can have Motors.  These are objects
 *  which control how the model moves in response to its parent entity moving.
 *	The default Motor for a PyModel is an ActionMatcher which also takes care
 *  of synchronising animations with the movement of the model (for example,
 *  playing Idle when it stands still, and Walk when it moves at walking
 *  speed).
 *
 *	In order to animate a PyModel, you play actions on it.  You do this by
 *	calling an ActionQueuer object.  These are available as named attributes from the
 *  PyModel, as well as by using the action function to look up the name.
 *
 *	For example:
 *	@{
 *	ac1 = model.Walk
 *	ac2 = model.action("Walk")
 *	@}
 *	both obtain the same ActionQueuer object.  The action can be played by
 *  calling the object, as follows:
 *	@{
 *	model.Walk()
 *	@}
 *
 *	Actions are defined in the .model file.
 *
 *	A PyModel can also have one or more instances of PyDye.  These are
 *  available as named attributes, in exactly the same way that actions are.
 *	The PyDye can be assigned a string which names a particular tint, which
 *  will change the material which is applied to a particular piece of geometry
 *  on the model.
 *
 *	For example:
 *	@{
 *	# Apply the Red tint (which is defined in the .model file) to the Chest
 *  # PyDye.
 *	model.Chest = "Red"
 *	@}
 *
 *	In addition, a PyModel can have hardpoints.  A hardpoint is a node
 *	that has a prefix of "HP_" on its name.  This is defined in the .model file.
 *	A PyAttachment can be attached to a hardpoint by assigning it to an attribute
 *	with that hardpoints name.
 *
 *	For example, if there were a node named HP_RightHand:
 *	@{
 *	# attach the sword model to the biped's right hand
 *	biped.RightHand = sword
 *	@}
 */
/**
 *	A PyModel is a Scene Node with an animation player,
 *	an action queue and the ability to attach models to hardPoints.
 *
 *	A PyModel provides script access to a SuperModel.
 *
 *	PyModels do a lot of other cools things too.
 *
 * PyModelBase extends Aligned, so we don't have to
 */
class PyModel : public PyAttachment, public Aligned
{
	Py_Header( PyModel, PyAttachment )

public:
	~PyModel();

	// PyAttachment overrides
	void tick( float dTime );
	void draw( const Matrix & worldTransform, float lod );

	virtual void tossed( bool outside );

	bool visible() const			{ return visible_; }
	void visible( bool v )			{ visible_ = v; }

	bool visibleAttachments() const			{ return visibleAttachments_; }
	void visibleAttachments( bool va )		{ visibleAttachments_ = va; }

	bool outsideOnly() const		{ return outsideOnly_; }
	void outsideOnly( bool oo )		{ outsideOnly_ = oo; }

	bool stipple() const			{ return stipple_; }
	void stipple( bool s )			{ stipple_ = s; }

	bool shimmer() const			{ return shimmer_; }
	void shimmer( bool s )			{ shimmer_ = s; }

	float moveScale() const			{ return moveScale_; }
	void moveScale( float m )		{ moveScale_ = m; }

	float actionScale() const		{ return actionScale_; }
	void actionScale( float a )		{ actionScale_ = a; }

	float height() const; 
	void height( float h ); 

	float yaw() const; 
	void yaw(float yaw); 

	// PyObjectPlus overrides
	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );
	int pyDelAttribute( const char * attr );

	// Script related methods
	PY_FACTORY_DECLARE()

	PY_METHOD_DECLARE_WITH_DOC( py_action, "This method looks up an action by name from an internal list of actions and "
										   "returns an ActionQueuer object if it finds it." );

	PyObject* getSound( const std::string& s );
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETOWN, getSound,
		ARG( std::string, END ), "Returns a Sound object for a 3D sound event at the location of the model." );

	PyObject* playSound( const std::string& s );
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETOWN, playSound,
		ARG( std::string, END ), "Plays a 3D sound at the location of the model." );

	bool stopSoundsOnDestroy() const { return cleanupSounds_; }
	void stopSoundsOnDestroy( bool enable ){ cleanupSounds_ = enable; }
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool,
		stopSoundsOnDestroy, stopSoundsOnDestroy );

#if FMOD_SUPPORT
	bool attachSound( PySound *pySound );
	PySoundList &sounds();

protected:
	// Sound-related helper methods
	PySound* createSoundInternal( const std::string& s );
	void cleanupSounds();
#endif // FMOD_SUPPORT

public:

	PY_METHOD_DECLARE_WITH_DOC( py_addMotor, "This method adds the specified object to the model's list of Motors." );
	PY_METHOD_DECLARE_WITH_DOC( py_delMotor, "This method removes the specified object to the models list of Motors." );
	PyObject * pyGet_motors();
	int pySet_motors( PyObject * value );

	// This function is for a hack to keep entity collisions working...
	Motor * motorZero() { return motors_.size() > 0 ? motors_[0] : NULL; }

	//  Attributes
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, visible, visible )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, visibleAttachments, visibleAttachments )
	PY_RW_ATTRIBUTE_DECLARE( moveAttachments_, moveAttachments )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, outsideOnly, outsideOnly )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, stipple, stipple )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, shimmer, shimmer )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, moveScale, moveScale )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, actionScale, actionScale )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, height, height ) 
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, yaw, yaw ) 

	PyObject * pyGet_sources();
	PY_RO_ATTRIBUTE_SET( sources )

	PyObject * pyGet_position();
	int pySet_position( PyObject * value );

	PyObject * pyGet_scale();
	int pySet_scale( PyObject * value );

	PY_RO_ATTRIBUTE_DECLARE( unitRotation_, unitRotation )

	void straighten();
	void rotate( float angle, const Vector3 & vAxis,
		const Vector3 & vCentre = Vector3( 0.f, 0.f, 0.f ) );
	void alignTriangle( const Vector3 & vertex0, const Vector3 & vertex1,
		const Vector3 & vertex2, bool randomYaw = true );
	Vector3 reflectOffTriangle( const Vector3 & vertex0,
		const Vector3 & vertex1,
		const Vector3 & vertex2,
		const Vector3 & fwd );
	void zoomExtents( bool upperFrontFlushness, float xzMutiplier = 1.f );

	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, straighten, END, "This method straightens a model, i.e. removes all rotations (including yaw) from it." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, rotate, ARG( float, ARG( Vector3,
		OPTARG( Vector3, Vector3( 0.f, 0.f, 0.f ), END ) ) ),
		"This method rotates a model about an arbitrary axis (and optionally an arbitrary centre)." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, alignTriangle, ARG( Vector3, ARG( Vector3,
		ARG( Vector3, OPTARG( bool, true, END ) ) ) ),
		"This method aligns a model to the given triangle (specified as three points in world space, taken in a clockwise direction)." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETDATA, reflectOffTriangle, ARG( Vector3, ARG( Vector3,
		ARG( Vector3, ARG( Vector3, END ) ) ) ),
		"This method reflects the Vector3 specified by the fwd argument off the "
		"specified triangle, and places the model in the centre of the triangle "
		"facing in the direction of the reflected vector." )
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETVOID, zoomExtents, ARG( bool, OPTARG( float, 1.f, END ) ),
		"This method places the model at the origin, scaled to fit within a 1 metre "
		"cube; that is, its longest dimension will be exactly 1 metre." )

	PY_RO_ATTRIBUTE_DECLARE( actionQueue_.queueState(), queue )

	PyObject * pyGet_bounds();
	PY_RO_ATTRIBUTE_SET( bounds )

	PY_RW_ATTRIBUTE_DECLARE( tossCallback_, tossCallback )

	PyObject * node( const std::string & nodeName, MatrixProviderPtr local = NULL );
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETOWN, node, ARG( std::string, OPTARG( MatrixProviderPtr, NULL, END ) ), "Returns the named node." );

	PY_RO_ATTRIBUTE_DECLARE( SmartPointer<PyObject>( this->node(""), true ), root );

	bool cue( const std::string & identifier, SmartPointer<PyObject> function );
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETOK, cue, ARG( std::string,
		ARG( SmartPointer<PyObject>, END ) ),
		"This method sets the default response for the given cue identifier. "
		"Sets a python exception on error.")

	PY_METHOD_DECLARE_WITH_DOC( py_saveActionQueue, "This function is used to save the state of the pyModel's action queue." )
	PY_METHOD_DECLARE_WITH_DOC( py_restoreActionQueue, "This function is used to restore the state of the pyModel's action queue." )

	// accessors
	SuperModel *		pSuperModel() const;

	ActionQueue &		actionQueue( void );

	// action queue methods
	void				unitTransform( const Matrix& transform );

	// action matcher methods
	void				unitOffset( const Vector3& offset );
	void				unitRotation( float yaw );
	float				unitRotation();

	// entity method (called from Entity's tick)
	void				move( float dTime );

	// world existence methods
//	void				addToScene( SubSpace * pSubSpace );
//	void				removeFromScene();

	void				enterWorld();
	void				leaveWorld();

	virtual void		localBoundingBox( BoundingBox & bb, bool skinny = false );
	virtual void		localVisibilityBox( BoundingBox & bb, bool skinny = false );
	virtual void		worldBoundingBox( BoundingBox & bb, const Matrix& world, bool skinny = false );
	virtual void		worldVisibilityBox( BoundingBox & vbb, const Matrix& world, bool skinny = false );

	void				shouldDrawSkeleton( bool drawIt )
									{ shouldDrawSkeleton_ = drawIt; }

	void				setDebugString( const char* id,
							const std::string& str )
							{
								if (pDebugInfo_ == NULL)
								{
									pDebugInfo_ = new DebugStrings;
								}
								(*pDebugInfo_)[id] = str;
							}

	std::string			name() const;

	const Matrix &		initialItinerantContext() const
								{ return initialItinerantContext_; }
	const Matrix &		initialItinerantContextInverse() const
								{ return initialItinerantContextInverse_; }

	bool origin( const std::string & nodeName );
	PY_AUTO_METHOD_DECLARE_WITH_DOC( RETOK, origin, ARG( std::string, END ), 
		"Make the named node the origin of the model's coordinate system, if a node "
		"with that name exists." )

	static PyModel *pCurrent() { return s_pCurrent_; }
	static void pCurrent( PyModel *pModel ){ s_pCurrent_ = pModel; }

private:
	PyModel( SuperModel * pSuperModel, PyTypePlus* pType = &PyModel::s_type_ );

	PyModel( const PyModel & );
	PyModel & operator=( const PyModel & );

	void calculateBoundingBoxes();
	void localBoundingBoxInternal( BoundingBox& bb );

	bool animateAsAttachment();
	void drawAsAttachment( Fashion * pCouplingFashion = NULL );
	void drawAttachments();

	void detach();

	PyModelNode * accessNode( Moo::NodePtr pNode, MatrixProviderPtr = NULL );

	typedef StringHashMap< SmartPointer<PyFashion> > Fashions;

	SuperModel			*pSuperModel_;
	ActionQueue			actionQueue_;

	BoundingBox			localBoundingBox_;
	BoundingBox			localVisibilityBox_;
	float				height_;
	bool				visible_;
	bool				visibleAttachments_;
	bool				moveAttachments_;
	bool				outsideOnly_;
	bool				stipple_;
	bool				shimmer_;

	float				moveScale_;
	float				actionScale_;

	Fashions			fashions_;
	ModelAlignerPtr		pCouplingFashion_;

	PyModelNodes		knownNodes_;

	Matrix				unitTransform_;
	Vector3				unitOffset_;
	float				unitRotation_;
	bool				transformModsDirty_;

	void				drawSkeleton();
	void				drawNodeLabels();

	void				drawDebugInfo();

	bool				shouldDrawSkeleton_;

	typedef StringHashMap<std::string> DebugStrings;
	DebugStrings *		pDebugInfo_;

#if FMOD_SUPPORT
	PySoundList        *pSounds_;
#endif
	bool				cleanupSounds_;

	typedef std::vector<Motor*>	Motors;
	Motors				motors_;

	Matrix				initialItinerantContext_;
	Matrix				initialItinerantContextInverse_;

	SmartPointer<PyObject>	tossCallback_;

	static PyModel *s_pCurrent_;

	friend class ScheduledModelLoader;
public:
	static bool			drawSkeletons_;
	static bool			drawNodeLabels_;
	static void			debugStuff( float dTime );
};

#ifndef PyModel_CONVERTERS
	PY_SCRIPT_CONVERTERS_DECLARE( PyModel )
	#define PyModel_CONVERTERS
#endif





#ifdef CODE_INLINE
#include "pymodel.ipp"
#endif




#endif // PYMODEL_HPP
