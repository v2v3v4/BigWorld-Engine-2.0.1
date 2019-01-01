/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RETRYING_REMOTE_METHOD_HPP
#define RETRYING_REMOTE_METHOD_HPP

#include "cstdmf/smartpointer.hpp"
#include "pyscript/pyobject_plus.hpp"

#include <memory>

class BlockingResponseHandler;
class MethodDescription;
class WebEntityMailBox;

class RetryingRemoteMethod : public PyObjectPlus
{
	Py_Header( RetryingRemoteMethod, PyObjectPlus )

public:
	RetryingRemoteMethod( WebEntityMailBox * pMailBox, 
		const MethodDescription * pMethodDescription,
		PyTypePlus * pType = &s_type_ );
	~RetryingRemoteMethod();

	PY_METHOD_DECLARE( pyCall )

private:
	bool createMethodCallStream( PyObject * args );
	
	PyObject * sendRetrying( PyObject * args, bool retry = true );
	
	
	SmartPointer< WebEntityMailBox > 			pMailBox_;
	const MethodDescription * 					pMethodDescription_;
	std::auto_ptr< BlockingResponseHandler > 	pHandler_;
};


#endif // RETRYING_REMOTE_METHOD_HPP
