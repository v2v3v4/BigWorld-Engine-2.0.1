/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_CHUNK_LIGHT_HPP
#define PY_CHUNK_LIGHT_HPP

#include "chunk/chunk_light.hpp"
#include "moo/omni_light.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "pyscript/script_math.hpp"


namespace Script
{
	int setData( PyObject * pObject, Moo::Colour & colour,
		const char * varName = "" );
	PyObject * getData( const Moo::Colour & colour );
}

/*~ class BigWorld.PyChunkLight
 *  This is a script controlled omni light which can be used as a diffuse 
 *  and/or specular light source for models and shells. This is not the class 
 *  used for environmental lights or lights which have been placed in the 
 *  editor.
 *
 *  The colour, radius, and position properties of the light can be set to
 *  static values via the colour, innerRadius and outerRadius, and position
 *  attributes. Alternatively, They can be set to dynamic values via the
 *  source, bounds, and shader attributes. These will override the static
 *  values when specified.  This gives you the ability to dynamically
 *  alter the light's look, or attach it to moving models in the scene.
 *
 *  The strength of a PyChunkLight at a point is determined by it's
 *  innerRadius, outerRadius, and the distance between the point and
 *  the PyChunkLight. If the distance is less than innerRadius, then the light
 *  is applied at full strength (regardless of outerRadius). The strength of
 *  the light falls off linearly from full to 0 between innerRadius and
 *  outerRadius, and is 0 from outerRadius onwards.
 *
 *  Code Example:
 *  @{
 *  # This sets up a few PyChunkLight objects which show some of the things
 *  # that they can do.
 *
 *  # import Math module
 *  import Math
 *
 *  # create a simple diffuse light source at position (0, 0, 0)
 *  simpleDiffuse = BigWorld.PyChunkLight()
 *  simpleDiffuse.innerRadius = 1
 *  simpleDiffuse.outerRadius = 3
 *  simpleDiffuse.position = (0, 0, 0)
 *  simpleDiffuse.colour = (255, 255, 255, 0) # white
 *  simpleDiffuse.specular = 0
 *  simpleDiffuse.diffuse = 1
 *  simpleDiffuse.visible = True
 *
 *  # create a simple specular light source
 *  simpleSpecular = BigWorld.PyChunkLight()
 *  simpleSpecular.innerRadius = 1
 *  simpleSpecular.outerRadius = 3
 *  simpleSpecular.position = (5, 0, 0)
 *  simpleSpecular.colour = (255, 255, 255, 0) # white
 *  simpleSpecular.specular = 1
 *  simpleSpecular.diffuse = 0
 *  simpleSpecular.visible = True
 *
 *  # create a light source which does not disperse
 *  noDissipate = BigWorld.PyChunkLight()
 *  noDissipate.innerRadius = 2
 *  noDissipate.outerRadius = 1.5
 *  noDissipate.position = (10, 0, 0)
 *  noDissipate.colour = (255, 255, 255, 0) # white
 *  noDissipate.specular = 1
 *  noDissipate.diffuse = 1
 *  noDissipate.visible = True
 *
 *  # create a light source which follows the player entity
 *  followPlayer = BigWorld.PyChunkLight()
 *  followPlayer.innerRadius = 1
 *  followPlayer.outerRadius = 3
 *  followPlayer.source = BigWorld.player().matrix
 *  followPlayer.colour = (255, 255, 255, 0) # white
 *  followPlayer.specular = 1
 *  followPlayer.diffuse = 1
 *  followPlayer.visible = True
 *
 *  # create a light source which is attached to the player's hand
 *  # NB: the hand node is assumed to be called "HP_right_hand" in this
 *  # example.  Examine the model in ModelViewer to determine the node name
 *  # in your model.
 *  followPlayerHand = BigWorld.PyChunkLight()
 *  followPlayerHand.innerRadius = 1
 *  followPlayerHand.outerRadius = 3
 *  followPlayerHand.source = BigWorld.player().model.node("HP_right_hand")
 *  followPlayerHand.colour = (255, 255, 255, 0) # white
 *  followPlayerHand.specular = 1
 *  followPlayerHand.diffuse = 1
 *  followPlayerHand.visible = True
 *
 *  # create a light source which changes colour over time at position 
 *  # (15, 0, 0)
 *  pulseShader = Math.Vector4Animation()
 *  pulseShader.duration = 1.0
 *  pulseShader.keyframes = [ ( 0.0, (200, 0,   0, 0) ),    # red at 0.0s
 *                            ( 0.5, (0,   0, 200, 0) ),    # blue at 0.5s
 *                            ( 1.0, (200, 0,   0, 0) ) ]   # red at 1.0s
 *  pulse = BigWorld.PyChunkLight()
 *  pulse.innerRadius = 1
 *  pulse.outerRadius = 3
 *  pulse.position = (15, 0, 0)
 *  pulse.shader = pulseShader
 *  pulse.specular = 1
 *  pulse.diffuse = 1
 *  pulse.visible = True
 *
 *  # create a light source which changes radius over time
 *  growBounds = Math.Vector4Animation()
 *  growBounds.duration = 1.0
 *  growBounds.keyframes = [ ( 0.0, ( 0, 0, 0, 0 ) ),    # 0 at 0.0s
 *                           ( 0.5, ( 2, 1, 0, 0 ) ),    # 2 at 0.5s
 *                           ( 1.0, ( 0, 0, 0, 0 ) ) ]   # 0 at 1.0s
 *  grow = BigWorld.PyChunkLight()
 *  grow.position = (20, 0, 0)
 *  grow.colour = (255, 255, 255, 0) # white
 *  grow.bounds = growBounds
 *  grow.specular = 1
 *  grow.diffuse = 1
 *  grow.visible = True
 *  @}
 */
/**
 *	This class is a quick wrapper around a chunk omni light.
 */
class PyChunkLight : public PyObjectPlusWithVD, public ChunkOmniLight
{
	Py_Header( PyChunkLight, PyObjectPlusWithVD )

public:
	PyChunkLight( PyTypePlus * pType = & s_type_);
	~PyChunkLight();

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, visible, visible )

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3, position, position )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Moo::Colour, pLight_->colour, colour )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int, pLight_->priority, priority )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, innerRadius, innerRadius )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, outerRadius, outerRadius )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, specularLight, specular )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, dynamicLight, diffuse )

	PY_RW_ATTRIBUTE_DECLARE( pSource_, source )
	PY_RW_ATTRIBUTE_DECLARE( pBounds_, bounds )
	PY_RW_ATTRIBUTE_DECLARE( pShader_, shader )

	PY_AUTO_METHOD_DECLARE( RETVOID, recalc, END )

	static PyObject * New() { return new PyChunkLight(); }
	PY_AUTO_FACTORY_DECLARE( PyChunkLight, END )

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	bool visible() const;
	void visible( bool v );

	const Vector3 & position() const;
	void position( const Vector3 & v );

	float innerRadius() const;
	void innerRadius( float v );

	float outerRadius() const;
	void outerRadius( float v );

	void recalc()							{ this->position( this->position() ); }

	void rev();
	static void revAll();

	// ChunkItem overrides
	virtual void tick( float dTime );
	virtual void nest( ChunkSpace * pSpace );
	virtual void incRef() const		{ this->PyObjectPlus::incRef(); }
	virtual void decRef() const		{ this->PyObjectPlus::decRef(); }

private:
	MatrixProviderPtr		pSource_;
	Vector4ProviderPtr		pBounds_;
	Vector4ProviderPtr		pShader_;

	bool					visible_;

	typedef std::vector<PyChunkLight*> PyChunkLights;
	static PyChunkLights	s_currentLights_;

	PyChunkLight( const PyChunkLight& );
	PyChunkLight& operator=( const PyChunkLight& );

	void radiusUpdated( float orad );
};


#endif // PY_CHUNK_LIGHT_HPP
