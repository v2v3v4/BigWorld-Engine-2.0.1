/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DOC_WATCHER_HPP
#define DOC_WATCHER_HPP

#include "cstdmf/config.hpp"
#include "cstdmf/watcher.hpp"

#if ENABLE_WATCHERS

#include "cstdmf/smartpointer.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/datasection.hpp"

class DataSection;

typedef SmartPointer< DataSection > DataSectionPtr;

namespace WatcherDoc
{
	void initWatcherDoc( const std::string & appName );
	void setWatcherDoc( WatcherPtr pWatcher, const std::string & path );
}

#undef MF_WATCH
#undef MF_WATCH_REF

#define BW_INIT_WATCHER_DOC		WatcherDoc::initWatcherDoc

#ifndef _WIN32
#define MF_WATCH( PATH, ... )					\
	WatcherDoc::setWatcherDoc( ::addWatcher( PATH, __VA_ARGS__ ), PATH )

#define MF_WATCH_REF( PATH, ... )					\
	WatcherDoc::setWatcherDoc( ::addReferenceWatcher( PATH, __VA_ARGS__ ), PATH )

#else //VC++ does not support C99 standard. i.e. no macro with variable arguments.

#define MF_WATCH addWatcherWrap

template <class RETURN_TYPE, class OBJECT_TYPE, class DUMMY_TYPE>
void addWatcherWrap( const char * path,
			OBJECT_TYPE & rObject,
			RETURN_TYPE (OBJECT_TYPE::*getMethod)() const,
			void (OBJECT_TYPE::*setMethod)(DUMMY_TYPE),
			const char * comment = NULL )
{
	WatcherDoc::setWatcherDoc(
		::addWatcher( path, rObject, getMethod, setMethod, comment ), path );
}

template <class RETURN_TYPE, class OBJECT_TYPE>	
void addWatcherWrap( const char * path,
			OBJECT_TYPE & rObject,
			RETURN_TYPE (OBJECT_TYPE::*getMethod)() const,
			const char * comment = NULL )
{
	WatcherDoc::setWatcherDoc(
		::addWatcher( path, rObject, getMethod, comment ), path );
}

template <class RETURN_TYPE>
void addWatcherWrap( const char * path,
			RETURN_TYPE (*getFunction)(),
			void (*setFunction)( RETURN_TYPE ) = NULL,
			const char * comment = NULL )
{
	WatcherDoc::setWatcherDoc(
		::addWatcher( path, getFunction, setFunction, comment ), path );
}

template <class TYPE>
void addWatcherWrap( const char * path,
			TYPE & rValue,
			Watcher::Mode access = Watcher::WT_READ_WRITE,
			const char * comment = NULL )
{
	WatcherDoc::setWatcherDoc(
		::addWatcher( path, rValue, access, comment ), path );
}

#define MF_WATCH_REF addReferenceWatcherWrap

template <class RETURN_TYPE, class OBJECT_TYPE>
void addReferenceWatcherWrap( const char * path,
			OBJECT_TYPE & rObject,
			const RETURN_TYPE & (OBJECT_TYPE::*getMethod)() const,
			const char * comment = NULL )
{
	WatcherDoc::setWatcherDoc(
		::addReferenceWatcher( path, rObject, getMethod, comment ),
		path );
}

template <class RETURN_TYPE, class OBJECT_TYPE, class DUMMY_TYPE>
void addReferenceWatcherWrap( const char * path,
			OBJECT_TYPE & rObject,
			const RETURN_TYPE & (OBJECT_TYPE::*getMethod)() const,
			void (OBJECT_TYPE::*setMethod)(DUMMY_TYPE),
			const char * comment = NULL )
{
	WatcherDoc::setWatcherDoc(
		::addReferenceWatcher( path, rObject, getMethod, setMethod, comment ),
		path );
}

#endif		// _WIN32

#define BW_WATCH MF_WATCH		// should we replace MF with BW?
#define BW_WATCH_REF MF_WATCH_REF		// should we replace MF with BW?



#else // ENABLE_WATCHERS


#define BW_INIT_WATCHER_DOC( x )


#endif // ENABLE_WATCHERS



#endif		//DOC_WATCHER_HPP
