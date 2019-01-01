/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROJECTION_ACCESS_HPP
#define PROJECTION_ACCESS_HPP


#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"


/*~ class BigWorld.ProjectionAccess
 *
 *	This class exposes the projection matrix, and allows manipulation of the
 *  clipping planes and the field of view.
 *
 *	There is one projection matrix for the client, which is accessed using 
 *	BigWorld.projection().  This is automatically created by BigWorld.
 *
 */
/**
 *	This class manages the projection matrix, and allows script access
 *	to its parameters.
 */
class ProjectionAccess : public PyObjectPlus
{
	Py_Header( ProjectionAccess, PyObjectPlus )

public:
	ProjectionAccess( PyTypePlus * pType = &s_type_ );
	~ProjectionAccess();

	void update( float dTime );

	void rampFov( float val, float time );

	float nearPlane() const;
	void nearPlane( float val );

	float farPlane() const;
	void farPlane( float val );

	float fov() const;
	void fov( float val );


	PyObject *	pyGetAttribute( const char * attr );
	int			pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, nearPlane, nearPlane )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, farPlane, farPlane )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, fov, fov )

	PY_AUTO_METHOD_DECLARE( RETVOID, rampFov, ARG( float, ARG( float, END ) ) )
	
private:
	ProjectionAccess( const ProjectionAccess& );
	ProjectionAccess& operator=( const ProjectionAccess& );

	bool	smoothFovTransition_;

	float	fovTransitionStart_;
	float	fovTransitionEnd_;
	float	fovTransitionTime_;
	float	fovTransitionTimeElapsed_;
};

typedef SmartPointer<ProjectionAccess> ProjectionAccessPtr;

PY_SCRIPT_CONVERTERS_DECLARE( ProjectionAccess )



#ifdef CODE_INLINE
#include "projection_access.ipp"
#endif

#endif // PROJECTION_ACCESS_HPP
