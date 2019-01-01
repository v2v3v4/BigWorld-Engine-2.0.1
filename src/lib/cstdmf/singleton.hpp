/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include "debug.hpp"

/**
 *	This class is used to implement singletons. Generally singletons should be
 *	avoided. If they _need_ to be used, try to use this as the base class.
 *
 *	If creating a singleton class MyApp:
 *
 *	class MyApp : public Singleton< MyApp >
 *	{
 *	};
 *
 *	In cpp file,
 *	BW_SINGLETON_STORAGE( MyApp )
 *
 *	To use:
 *	MyApp app; // This will be registered as the singleton
 *
 *	...
 *	MyApp * pApp = MyApp::pInstance();
 *	MyApp & app = MyApp::instance();
 */
template <class T>
class Singleton
{
protected:
	static T * s_pInstance;

public:
	Singleton()
	{
		MF_ASSERT( NULL == s_pInstance );
		s_pInstance = static_cast< T * >( this );
	}

	~Singleton()
	{
		MF_ASSERT( this == s_pInstance );
		s_pInstance = NULL;
	}

	static T & instance()
	{
		MF_ASSERT( s_pInstance );
		return *s_pInstance;
	}

	static T * pInstance()
	{
		return s_pInstance;
	}
};

/**
 *	This should appear in the cpp of the derived singleton class.
 */
#define BW_SINGLETON_STORAGE( TYPE )						\
template <>													\
TYPE * Singleton< TYPE >::s_pInstance = NULL;				\

#endif // SINGLETON_HPP
