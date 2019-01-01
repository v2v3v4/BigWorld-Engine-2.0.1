/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PERSONALITY_HPP
#define PERSONALITY_HPP

#include "pyobject_plus.hpp"

#define DEFAULT_PERSONALITY_NAME "BWPersonality"

/**
 *	This namespace manages the personality script.
 */
namespace Personality
{

extern const char *DEFAULT_NAME;
PyObject* import( const std::string &name );
PyObject* instance();

bool callOnInit( bool isReload = false );

}

#endif // PERSONALITY_HPP
