/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_BILLING_SYSTEM_HPP
#define PY_BILLING_SYSTEM_HPP

#include <Python.h>
#include "pyscript/pyobject_pointer.hpp"

#include "dbmgr_lib/billing_system.hpp"

class EntityDefs;


/**
 *	This class is used to integrate with a billing system via Python.
 */
class PyBillingSystem : public BillingSystem
{
public:
	PyBillingSystem( PyObject * pPyObject, const EntityDefs & entityDefs );
	~PyBillingSystem();

	virtual void getEntityKeyForAccount(
		const std::string & username, const std::string & password,
		const Mercury::Address & clientAddr,
		IGetEntityKeyForAccountHandler & handler );

	virtual void setEntityKeyForAccount( const std::string & username,
		const std::string & password, const EntityKey & ekey );

	virtual bool isOkay() const
	{
		return getEntityKeyFunc_;
	}

private:
	static PyObjectPtr getMember( PyObject * pPyObject, const char * member );

	const EntityDefs & entityDefs_;

	PyObjectPtr getEntityKeyFunc_;
	PyObjectPtr setEntityKeyFunc_;
};

#endif // PY_BILLING_SYSTEM_HPP
