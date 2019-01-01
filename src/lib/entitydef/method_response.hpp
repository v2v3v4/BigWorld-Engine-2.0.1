/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef METHOD_RESPONSE_HPP
#define METHOD_RESPONSE_HPP

#include "Python.h"

#include "network/basictypes.hpp"
#include "network/misc.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include <map>
#include <string>

class MethodDescription;
namespace Mercury
{
class NetworkInterface;
}


/**
 *	Instances of this class are used to supply return values back to remote
 *	callers.  It has Python attributes that correspond to return values
 *	described by
 *	the MethodDescription.
 *
 *	@ingroup entity
 */
class MethodResponse: public PyObjectPlus
{
	Py_Header( MethodResponse, PyObjectPlus )

public:
	MethodResponse( int replyID,
		const Mercury::Address & replyAddr,
		Mercury::NetworkInterface & networkInterface,
		const MethodDescription & methodDesc );

	virtual ~MethodResponse();

	PY_METHOD_DECLARE( py_done )

	// Overrides from PyObjectPlus
	virtual PyObject * pyGetAttribute( const char * attr );

	virtual int pySetAttribute( const char * attr, PyObject* value );

private:
	Mercury::ReplyID				replyID_;
	Mercury::Address 				replyAddr_;
	Mercury::NetworkInterface & 	interface_;
	const MethodDescription & 		methodDesc_;
	
	typedef std::map< std::string, PyObjectPtr > ReturnValueData;
	ReturnValueData 				returnValueData_;
};

PY_SCRIPT_CONVERTERS_DECLARE( MethodResponse )

#endif // METHOD_RESPONSE_HPP
