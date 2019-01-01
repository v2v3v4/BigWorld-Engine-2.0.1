/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EGEXTRA_HPP
#define EGEXTRA_HPP

#include "cellapp/entity_extra.hpp"

/**
 * Simple example entity extra... can print a message to the screen.
 */

#undef PY_METHOD_ATTRIBUTE_WITH_DOC
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_ENTITY_EXTRA_WITH_DOC

class EgExtra : public EntityExtra
{
	Py_EntityExtraHeader( EgExtra );

public:
	EgExtra( Entity & e );
	~EgExtra();

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_AUTO_METHOD_DECLARE( RETVOID, helloWorld, END );
	void helloWorld();

	PY_AUTO_METHOD_DECLARE( RETOWN, addEgController, ARG( int, END ) );
	PyObject * addEgController( int userArg );

	static const Instance<EgExtra> instance;
};

#undef PY_METHOD_ATTRIBUTE_WITH_DOC
#define PY_METHOD_ATTRIBUTE_WITH_DOC PY_METHOD_ATTRIBUTE_BASE_WITH_DOC

#endif // EGEXTRA_HPP
