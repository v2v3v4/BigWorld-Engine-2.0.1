/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BOX_ATTACHMENT_HPP
#define BOX_ATTACHMENT_HPP

#include "py_attachment.hpp"
#include "resmgr/datasection.hpp"

/**
 *	This class implements a PyAttachment with hit-testing
 *	capabilities.  Use this to attach to dynamic skeletal
 *	objects for collision tests.
 */
class BoxAttachment : public PyAttachment, public Aligned
{
	Py_Header( BoxAttachment, PyAttachment )

public:
	BoxAttachment( PyTypePlus * pType = &s_type_ );
	~BoxAttachment();

	PY_FACTORY_DECLARE()
	PY_RW_ATTRIBUTE_DECLARE( minBounds_, minBounds )
	PY_RW_ATTRIBUTE_DECLARE( maxBounds_, maxBounds )

	PyObject * pyGet_name();
	int pySet_name( PyObject * pValue );

	PY_RW_ATTRIBUTE_DECLARE( hit_, hit )
	PY_RO_ATTRIBUTE_DECLARE( worldTransform_, worldTransform )

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	void name( const std::string& name );
	const std::string& name() const;

	virtual void tick( float dTime );
	virtual void draw( const Matrix & worldTransform, float lod );
	bool collide( const Vector3& start, const Vector3& direction, float& distance );

	void	save( DataSectionPtr pSection );
	void	load( DataSectionPtr pSection );

private:
	BoxAttachment( const BoxAttachment& );
	BoxAttachment& operator=( const BoxAttachment& );

	Vector3		minBounds_;
	Vector3		maxBounds_;
	Matrix		worldTransform_;
	uint16		nameIndex_;
	bool		hit_;
};

PY_SCRIPT_CONVERTERS_DECLARE( BoxAttachment )


#endif // BOX_ATTACHMENT_HPP
