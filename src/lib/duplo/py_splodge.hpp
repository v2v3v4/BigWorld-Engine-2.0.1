/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_SPLODGE_HPP
#define PY_SPLODGE_HPP


#include "py_attachment.hpp"

/*~ class BigWorld.PySplodge
 *
 *  A PyAttachment which draws a "splodge" shadow on the ground
 *  below the point where it is attached. The shape of this shadow
 *  moves in relation to the position of the sun, and is only visible
 *  outside. The actual shadow is drawn using the material specified
 *  in resource.xml/environment/splodgeMaterial
 *
 *	A new PySplodge is created using BigWorld.Splodge function.
 */
/**
 *	This class is an attachment that draws a splodge on the ground
 *	below the point where it is attached.
 */
class PySplodge : public PyAttachment
{
	Py_Header( PySplodge, PyAttachment )

public:
	PySplodge( Vector3 bbSize, PyTypePlus * pType = &s_type_ );

	virtual void tick( float dTime ) { }
	virtual void draw( const Matrix & worldTransform, float lod );

	virtual void tossed( bool outside );

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_REF_DECLARE( bbSize_, size )
	PY_RW_ATTRIBUTE_DECLARE( maxLod_, maxLod )

	static PySplodge * New( Vector3 bbSize );
	PY_AUTO_FACTORY_DECLARE( PySplodge, OPTARG( Vector3, Vector3(0,0,0), END ) )

private:
	Vector3			bbSize_;
	float			maxLod_;
	bool			outsideNow_;

public:
	static bool		s_ignoreSplodge_;
	static uint32	s_splodgeIntensity_;
};



#endif // PY_SPLODGE_HPP
