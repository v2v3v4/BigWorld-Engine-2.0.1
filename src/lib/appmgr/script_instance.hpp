/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCRIPT_INSTANCE_HPP
#define SCRIPT_INSTANCE_HPP

#include <iostream>

#include "resmgr/datasection.hpp"
#include "moo/moo_math.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

/**
 *	This class is a common base class to all instance script objects.
 */
class ScriptInstance : public PyInstancePlus
{
public:
	ScriptInstance( PyTypeObject * pType );

	bool init( DataSectionPtr pSection,
		const char * moduleName, const char * defaultTypeName );

private:
	ScriptInstance( const ScriptInstance& );
	ScriptInstance& operator=( const ScriptInstance& );
};

#ifdef CODE_INLINE
#include "script_instance.ipp"
#endif

#endif // SCRIPT_INSTANCE_HPP
