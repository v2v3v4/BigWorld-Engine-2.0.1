/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DEVICE_CALLBACK_HPP
#define DEVICE_CALLBACK_HPP

#include <iostream>
#include <list>

namespace Moo
{

/**
 *	Classes which implement this interface will receive notification of device
 *	loss/destruction through the various delete/create calls. This class should
 *	be inherited/implemented by any object utilising D3D resources. For examples
 *	of it's use, see the various texture / render target classes.
 */
class DeviceCallback
{
public:
	typedef std::list< DeviceCallback* > CallbackList;

	DeviceCallback();
	~DeviceCallback();

	virtual void deleteUnmanagedObjects( );
	virtual void createUnmanagedObjects( );
	virtual void deleteManagedObjects( );
	virtual void createManagedObjects( );

	static void deleteAllUnmanaged( );
	static void createAllUnmanaged( );
	static void deleteAllManaged( );
	static void createAllManaged( );

	static void fini();
private:
/*	DeviceCallback(const DeviceCallback&);
	DeviceCallback& operator=(const DeviceCallback&);*/

	static CallbackList* callbacks_;

	friend std::ostream& operator<<(std::ostream&, const DeviceCallback&);
};

/**
 *	A Generic implementation of the DeviceCallback interface for unmanaged 
 *	resources. The function callbacks passed into the destructor are used 
 *	for the creation/destruction of the resources.
 */
class GenericUnmanagedCallback : public DeviceCallback
{
public:

	typedef void Function( );

	GenericUnmanagedCallback( Function* createFunction, Function* destructFunction  );
	~GenericUnmanagedCallback();

	void deleteUnmanagedObjects( );
	void createUnmanagedObjects( );

private:

	Function* createFunction_;
	Function* destructFunction_;

};

};

#ifdef CODE_INLINE
#include "device_callback.ipp"
#endif




#endif // DEVICE_CALLBACK_HPP
