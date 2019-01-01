/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_MORPH_CONTROL_HPP
#define PY_MORPH_CONTROL_HPP

#include "pyfashion.hpp"
#include "pyscript/script_math.hpp"
#include "pyscript/stl_to_py.hpp"

/*~ class BigWorld.PyMorphControl
 *
 *	This class is used to blend morph targets onto a PyModel, using blend
 *	weights specified in a MatrixProvider.  The list of morph targets
 *	is specified by the targetNames attribute, the MatrixProvider is 
 *	specified by the input attribute.
 *
 *	To apply the PyMorphControl to a model, it should be assigned to an
 *	arbitrarily named attribute of that model.  In order to function 
 *	correctly, the named morph targets should all have been exported as part
 *	of the model's primitives.
 *	
 *	If more than one PyMorphControl is applied to a model at once, then
 *	the morph targets of both of them are weighted together.
 *
 *	A new PyMorphControl is created using the BigWorld.PyMorphControl function.
 *
 *	For example:
 *	@{
 *	mat = Math.Matrix()
 *	mat.setElement( 0, 0, 2.0 ) # set the blend weight for "openLips" to 2.0
 *	mat.setElement( 0, 1, 1.0 ) # set the blend weight for "closedLips" to 1.0
 *	self.model.myMorphControl = BigWorld.PyMorphControl()
 *	self.model.myMorphControl.input = mat
 *	self.model.myMorphContro.targetNames = [ "openLips", "closedLips" ]
 *	@}
 *	This example sets the model to have its lips two thirds open,
 *	assuming that the two named morph targets existed on the model.
 */
/**
 *	This class controls morph target values specified in a matrix.
 */
class PyMorphControl : public PyFashion
{
	Py_Header( PyMorphControl, PyFashion )

public:
	PyMorphControl( PyTypePlus * pType = &s_type_ );
	~PyMorphControl();

	virtual FashionPtr fashion()		{ return pFashion_; }

	PY_RW_ATTRIBUTE_DECLARE( input_, input )

	PY_RW_ATTRIBUTE_DECLARE( targetNamesHolder_, targetNames );

	static PyObject * New();
	PY_AUTO_FACTORY_DECLARE( PyMorphControl, END )

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	void apply() const;

private:
	SmartPointer<MatrixProvider>	input_;
	FashionPtr						pFashion_;

	typedef std::vector<std::string>	TargetNames;
	TargetNames		targetNames_;
	PySTLSequenceHolder<TargetNames>	targetNamesHolder_;
};



#endif // PY_MORPH_CONTROL_HPP
