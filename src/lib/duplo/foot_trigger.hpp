/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FOOT_TRIGGER_HPP
#define FOOT_TRIGGER_HPP

#include "py_attachment.hpp"
#include "particle/actions/source_psa.hpp"
#include "fmodsound/py_sound.hpp"
#include <deque>


/*~ class BigWorld.FootTrigger
 *
 *	A FootTrigger is a PyAttachment which can be attached to nodes in a model
 *	to test for them hitting the ground.  It is capable of both creating
 *	particles from a SourcePSA which it stores, and also calling a function on
 *	each footstep.
 *
 *	The FootTrigger works by tracking the height of the node it is attached
 *	to, relative to a rest position. Whenever this height gets bellow a given
 *	threshold, the trigger does a coarse collision test for a potential
 *	footstep. The test is performed againt any object in the collision scene
 *	which the trigger approaches from above. This includes terrain, shells and
 *	static models. It doesn't include entities, however. If the collision test
 *	yields true, a footstep is flagged. The threshold level can be set using
 *	the field &lt;environment/footprintThreshold&gt; in resources.xml.
 *
 *	In normal biped usage, two would be used, one attached to the right foot node,
 *	one to the left.  More than two can be used, however, the odd attribute is
 *	treated as boolean, so processing of leg specific effects is limited to
 *	right and left.
 *
 *	FootTriggers are created by the BigWorld.FootTrigger() function.
 *
 *	Example:
 *	@{
 *	leftTrigger  = BigWorld.FootTrigger( 1 )
 *	rightTrigger = BigWorld.FootTrigger( 0 )
 *	# dustSourcePSA is the particle system action to create the dust
 *	leftTrigger.dustSource =  dustSourcePSA
 *	rightTrigger.dustSource = dustSourcePSA
 *	# model is the model we want to add FootTriggers to
 *	lfoot = model.node( "biped R Foot" )
 *	rfoot = model.node( "biped L Foot" )
 *  lfoot.attach( leftTrigger )
 *  rfoot.attach( rightTrigger )
 *	@}
 */
/**
 *	This class is an attachment that tests for feet hitting the ground.
 *	When they hit it does stuff. Yeah.
 */
class FootTrigger : public PyAttachment
{
	Py_Header( FootTrigger, PyAttachment )

public:
	FootTrigger( bool odd, std::string soundTagPrefix, PyTypePlus * pType = &s_type_ );

	virtual void tick( float dTime );
	virtual void draw( const Matrix & worldTransform, float lod );

	virtual void detach();

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( maxLod_, maxLod );
	PY_RW_ATTRIBUTE_DECLARE( odd_, odd );

	PY_RW_ATTRIBUTE_DECLARE( dustSource_, dustSource );
	PY_RW_ATTRIBUTE_DECLARE( footstepCallback_, footstepCallback );

	static FootTrigger *New( bool odd, std::string soundTagPrefix )
		{ return new FootTrigger(odd, soundTagPrefix); }

	PY_AUTO_FACTORY_DECLARE( FootTrigger,
		OPTARG( bool, false, OPTARG( std::string, "", END ) ) );

private:
	float			maxLod_;	// load out from here
	float			rest_;		// height at rest
	bool			armed_;		// is this foot armed
	bool			odd_;		// is this foot odd
	std::string		soundTagPrefix_;	// sound tag prefix (eg "footstep_")

	Vector3			lastPosition_;
	float			dTime_;
	int				plotIndex_;

	SmartPointer<PySourcePSA>	dustSource_;
	SmartPointer<PyObject>		footstepCallback_;

#if FMOD_SUPPORT
	PySoundPtr pSound_;
#endif
	uint32 lastMaterialKind_;

	static int instanceCounter_;

public:
	static bool  plotEnabled_;
};


#endif // FOOT_TRIGGER_HPP
