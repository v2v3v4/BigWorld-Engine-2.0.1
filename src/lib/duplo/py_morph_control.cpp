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

#pragma warning(disable: 4355)	// 'this' used in initialiser

#include "py_morph_control.hpp"
#include "model/super_model.hpp"
#include "moo/morph_vertices.hpp"

DECLARE_DEBUG_COMPONENT2( "PyModel", 0 )


// -----------------------------------------------------------------------------
// Section: MorphFashion
// -----------------------------------------------------------------------------

/**
 *	This class is used to apply the actual fashion to the SuperModel,
 *	at the appropriate point in the drawing process
 */
class MorphFashion : public Fashion
{
public:
	MorphFashion( PyMorphControl & owner ) : owner_( owner )
	{
	}

private:
	virtual void dress( class SuperModel & superModel )
	{
		owner_.apply();
	}

	PyMorphControl & owner_;
};


// -----------------------------------------------------------------------------
// Section: PyMorphControl
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyMorphControl )

PY_BEGIN_METHODS( PyMorphControl )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyMorphControl )
	/*~ attribute PyMorphControl.input
	 *
	 *	This attribute takes a MatrixProvider, whose elements are mapped to 
	 *	the items in the targetNames attribute, with the first row being 
	 *	mapped to the first four items, the next row being mapped to the next
	 *	four items, and so on, for up to sixteen items.  Elements beyond the
	 *	last item of targetNames are ignored.
	 *
	 *	Each element of the MatrixProvider is treated as the blend weight
	 *	for the morph target named in the corresponding targetName item.
	 *
	 *	@type	MatrixProvider
	 */
	PY_ATTRIBUTE( input )
	/*~ attribute PyMorphControl.targetNames
	 *
	 *	This attribute is a list of names of morph targets that this 
	 *	PyMorphControl will blend between. It will blend using the weights
	 *	supplied in the input attribute, which is a MatrixProvider, whose
	 *	items are mapped to the names in this attribute.
	 *
	 *	All the named targets should have been exported as morph targets in
	 *	the models primitive file, or undefined behaviour will result.
	 *
	 *	@type List of Strings
	 */
	PY_ATTRIBUTE( targetNames )
PY_END_ATTRIBUTES()

/*~ function BigWorld.PyMorphControl
 *
 *	This function creates a new PyMorphControl.  This can be applied to a 
 *	model, and used to blend between a specified list of morph targets
 *	using weights supplied in a MatrixProvider.
 *
 *	@return		a new PyMorphControl
 */
PY_FACTORY( PyMorphControl, BigWorld )


/**
 *	Constructor.
 */
PyMorphControl::PyMorphControl( PyTypePlus * pType ) :
	PyFashion( pType ),
	pFashion_( new MorphFashion( *this ) ),
	targetNamesHolder_( targetNames_, this, true )
{
}


/**
 *	Destructor.
 */
PyMorphControl::~PyMorphControl()
{
}


/**
 *	This method applies the current state of the morph
 */
void PyMorphControl::apply() const
{
	BW_GUARD;
	Matrix m;
	if (input_)
		input_->matrix( m );
	else
		m.setZero();

	TargetNames::const_iterator it;
	int i;
	for (it = targetNames_.begin(), i = 0;
		it != targetNames_.end() && i < 16;
		it++, i++)
	{
		Moo::MorphVertices::addMorphValue( (*it).c_str(), m[0][i], 1.0f );
		// figure out what to do with blend later ... maybe when input
		// is overwritten use it to crossfade ... or have an element in
		// the matrix for it
	}
}


/**
 *	Static factory method
 */
PyObject * PyMorphControl::New()
{
	BW_GUARD;
	return new PyMorphControl();
}


/**
 *	Python get attribute method
 */
PyObject * PyMorphControl::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return this->PyFashion::pyGetAttribute( attr );
}


/**
 *	Python set attribute method
 */
int PyMorphControl::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return this->PyFashion::pySetAttribute( attr, value );
}

// py_morph_control.cpp
