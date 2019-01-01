/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_CHUNK_SPOT_LIGHT_HPP
#define PY_CHUNK_SPOT_LIGHT_HPP

#include "chunk/chunk_light.hpp"
#include "moo/omni_light.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "pyscript/script_math.hpp"

#include "py_chunk_light.hpp"



/*~ class BigWorld.PyChunkSpotLight
 *  This is a script controlled spot light which can be used as a diffuse 
 *  and/or specular light source for models and shells. This is not the class 
 *  used for environmental lights or lights which have been placed in the 
 *  editor.
 *
 *  The colour, radius, cone size, direction and position properties of the light can be set to
 *  static values via the colour, innerRadius and outerRadius, cosConeAngle, direction and position
 *  attributes. Alternatively, They can be set to dynamic values via the
 *  source, bounds, and shader attributes. These will override the static
 *  values when specified.  This gives you the ability to dynamically
 *  alter the light's look, or attach it to moving models in the scene.
 *
 *  The strength of a PyChunkSpotLight at a point is determined by it's
 *  innerRadius, outerRadius, and the distance between the point and
 *  the PyChunkSpotLight. If the distance is less than innerRadius, then the light
 *  is applied at full strength (regardless of outerRadius). The strength of
 *  the light falls off linearly from full to 0 between innerRadius and
 *  outerRadius, and is 0 from outerRadius onwards.
 *
 *  Code Example:
 *  @{
 *  # This sets up a PyChunkSpotLight object which shows some of the things
 *  # that it can do.
 *
 *
 *  # create a light source which is attached to the player's hand
 *  # NB: the hand node is assumed to be called "HP_right_hand" in this
 *  # example.  Examine the model in ModelViewer to determine the node name
 *  # in your model. In the example models this points out from the palm.
 *  followPlayerHand = BigWorld.PyChunkSpotLight()
 *  followPlayerHand.innerRadius = 1
 *  followPlayerHand.outerRadius = 10
 *	followPlayerHand.cosConeAngle = 0.9 # ~25 degrees from center
 *  followPlayerHand.source = BigWorld.player().model.node("HP_right_hand")
 *  followPlayerHand.colour = (255, 255, 255, 0) # white
 *  followPlayerHand.specular = 1
 *  followPlayerHand.diffuse = 1
 *  followPlayerHand.visible = True
 *  @}
 */
/**
 *	This class is a quick wrapper around a chunk spot light.
 */
class PyChunkSpotLight : public PyObjectPlusWithVD, public ChunkSpotLight
{
	Py_Header( PyChunkSpotLight, PyObjectPlusWithVD )

public:
	PyChunkSpotLight( PyTypePlus * pType = & s_type_);
	~PyChunkSpotLight();

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, visible, visible )

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3, position, position )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3, direction, direction )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Moo::Colour, pLight_->colour, colour )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( int, pLight_->priority, priority )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, innerRadius, innerRadius )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, outerRadius, outerRadius )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, pLight_->cosConeAngle, cosConeAngle )
	
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, specularLight, specular )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, dynamicLight, diffuse )

	PY_RW_ATTRIBUTE_DECLARE( pSource_, source )
	PY_RW_ATTRIBUTE_DECLARE( pBounds_, bounds )
	PY_RW_ATTRIBUTE_DECLARE( pShader_, shader )

	PY_AUTO_METHOD_DECLARE( RETVOID, recalc, END )

	static PyObject * New() { return new PyChunkSpotLight(); }
	PY_AUTO_FACTORY_DECLARE( PyChunkSpotLight, END )

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	bool visible() const;
	void visible( bool v );

	const Vector3 & position() const;
	void position( const Vector3 & v );

	const Vector3 & direction() const;
	void direction( const Vector3 & v );

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

	typedef std::vector<PyChunkSpotLight*> PyChunkSpotLights;
	static PyChunkSpotLights	s_currentLights_;

	PyChunkSpotLight( const PyChunkSpotLight& );
	PyChunkSpotLight& operator=( const PyChunkSpotLight& );

	void radiusUpdated( float orad );
};


#endif // PY_CHUNK_SPOT_LIGHT_HPP
