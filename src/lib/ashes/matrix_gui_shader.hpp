/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MATRIX_GUI_SHADER_HPP
#define MATRIX_GUI_SHADER_HPP

#include "gui_shader.hpp"

#include "math/blend_transform.hpp"
#include "pyscript/script_math.hpp"


/*~ class GUI.MatrixGUIShader
 *	@components{ client, tools }
 *
 *	This class is a GUI shader that transforms the four corners of
 *	each component it is applied to by a matrix.  This affects the
 *	position the components are drawn on the screen, without actually
 *	changing the components themselves.
 *
 *	The four corners have Vector3 positions in screen space, which has
 *	(-1,-1,z) in the bottom left and (1,1,z) in the top right.  The
 *	z component is used for sorting purposes only.  This matrix transforms
 *	those points within screen space.
 *	
 *	The target matrix is a MatrixProvider, so it can be updated dynamically.  
 *	
 *	The MatrixGUIShader can also blend(linearly interpolate) from its current target
 *	to a newly specified target.  It takes eta seconds from the last time
 *	the target changed before the applied transform reaches the transform
 *	specified by the target.
 *
 *	A new MatrixGUIShader is created using GUI.MatrixShader function.
 */
/**
 *	This class is a GUI shader that multiplies the positions of the
 *	components it is attached to by a matrix. It can modify the
 *	matrix over time.
 */
class MatrixGUIShader : public GUIShader
{
	Py_Header( MatrixGUIShader, GUIShader )

public:
	MatrixGUIShader( PyTypePlus * pType = &s_type_ );
	///use this constructor for simple non-blended transforms ( static; ignores eta )
	MatrixGUIShader( MatrixProviderPtr pTrn, PyTypePlus * pType = &s_type_ );
	~MatrixGUIShader();

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_FACTORY_DECLARE()

	PY_RW_ATTRIBUTE_DECLARE( target_, target )
	PY_RW_ATTRIBUTE_DECLARE( eta_, eta )
	PY_RW_ATTRIBUTE_DECLARE( blend_, blend )

	void				target( MatrixProviderPtr m )	{ target_ = m; }
	MatrixProviderPtr   target()						{ return target_; }
	void				blend( bool state )				{ blend_ = state; }
	void				eta( float t )					{ eta_ = t; }


	void reset();
	PY_AUTO_METHOD_DECLARE( RETVOID, reset, END )

	bool processComponent( SimpleGUIComponent& component, float dTime );

private:
	MatrixGUIShader( const MatrixGUIShader& );
	MatrixGUIShader& operator=( const MatrixGUIShader& );

	BlendTransform		current_;
	MatrixProviderPtr	target_;
	float				eta_;
	bool				blend_;

	uint32				lastUpdated_;

	virtual void value( float ) { };	// unused virtual function

	virtual bool	load( DataSectionPtr pSect );
	virtual void	save( DataSectionPtr pSect );

	SHADER_FACTORY_DECLARE( MatrixGUIShader() )
};


#ifdef CODE_INLINE
#include "matrix_gui_shader.ipp"
#endif

#endif // MATRIX_GUI_SHADER_HPP
