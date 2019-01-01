/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "config.hpp"
#if ENABLE_WATCHERS

#include "watcher.hpp"

#include "fini_job.hpp"
#include "watcher_path_request.hpp"



// -----------------------------------------------------------------------------
// Section: Watcher Methods
// -----------------------------------------------------------------------------

namespace
{
// Cannot be a smart pointer because of static init ordering.
Watcher * g_pRootWatcher = NULL;

class WatcherFini : public FiniJob
{
public:
	virtual bool fini()
	{
		Watcher::fini();

		return true;
	}
};


// This object deletes itself
FiniJobPtr pFini = new WatcherFini();

} // anonymous namespace


/**
 *	This method returns the root watcher.
 *
 *	@return	A reference to the root watcher.
 */
Watcher & Watcher::rootWatcher()
{
	if (g_pRootWatcher == NULL)
	{
		g_pRootWatcher = new DirectoryWatcher();
		g_pRootWatcher->incRef();
	}

	return *g_pRootWatcher;
}

void Watcher::fini() 
{
	if (g_pRootWatcher)
	{
		g_pRootWatcher->decRef();
		g_pRootWatcher = NULL;
	}
}


bool Watcher::visitChildren( const void * base, const char * path,
		WatcherVisitor & visitor )
{
	WatcherPathRequestWatcherVisitor wpr( visitor );
	return this->visitChildren( base, path, wpr );
}

#include <assert.h>

#define SORTED_WATCHERS

/*
#ifndef CODE_INLINE
#include "debug_value.ipp"
#endif
*/

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "CStdMF", 0 )


// -----------------------------------------------------------------------------
// Section: DirectoryWatcher Methods
// -----------------------------------------------------------------------------

/**
 *	Constructor for DirectoryWatcher.
 */
DirectoryWatcher::DirectoryWatcher()
{
}

DirectoryWatcher::~DirectoryWatcher()
{
}


/*
 *	This method is an override from Watcher.
 */
bool DirectoryWatcher::getAsString( const void * base,
								const char * path,
								std::string & result,
								std::string & desc,
								Watcher::Mode & mode ) const
{
	if (isEmptyPath(path))
	{
		result = "<DIR>";
		mode = WT_DIRECTORY;
		desc = comment_;
		return true;
	}
	else
	{
		DirData * pChild = this->findChild( path );

		if (pChild != NULL)
		{
			const void * addedBase = (const void*)(
				((const uintptr)base) + ((const uintptr)pChild->base) );
			return pChild->watcher->getAsString( addedBase, this->tail( path ),
				result, desc, mode );
		}
		else
		{
			return false;
		}
	}
	// never gets here
}

bool DirectoryWatcher::getAsStream( const void * base,
								const char * path,
								WatcherPathRequestV2 & pathRequest ) const
{
	if (isEmptyPath(path))
	{
		Watcher::Mode mode = WT_DIRECTORY;
		watcherValueToStream( pathRequest.getResultStream(), "<DIR>", mode );
		pathRequest.setResult( comment_, mode, this, base );
		return true;
	}
	else if (isDocPath( path ))
	{
		Watcher::Mode mode = Watcher::WT_READ_ONLY;
		watcherValueToStream( pathRequest.getResultStream(), comment_, mode );
		pathRequest.setResult( comment_, mode, this, base );
		return true;
	}
	else
	{
		DirData * pChild = this->findChild( path );

		if (pChild != NULL)
		{
			const void * addedBase = (const void*)(
				((const uintptr)base) + ((const uintptr)pChild->base) );
			return pChild->watcher->getAsStream( addedBase, this->tail( path ),
				pathRequest );
		}
		else
		{
			return false;
		}
	}
	// never gets here
}



/*
 *	Override from Watcher.
 */
bool DirectoryWatcher::setFromString( void * base,
	const char * path, const char * value )
{
	DirData * pChild = this->findChild( path );

	if (pChild != NULL)
	{
		void * addedBase = (void*)( ((uintptr)base) + ((uintptr)pChild->base) );
		return pChild->watcher->setFromString( addedBase, this->tail( path ),
			value );
	}
	else
	{
		return false;
	}
}



/*
 *	Override from Watcher.
 */
bool DirectoryWatcher::setFromStream( void * base, const char * path,
		WatcherPathRequestV2 & pathRequest )
{
	DirData * pChild = this->findChild( path );

	if (pChild != NULL)
	{
		void * addedBase = (void*)( ((uintptr)base) + ((uintptr)pChild->base) );
		return pChild->watcher->setFromStream( addedBase, this->tail( path ),
			pathRequest );
	}
	else
	{
		return false;
	}
}



/*
 *	Override from Watcher
 */
bool DirectoryWatcher::visitChildren( const void * base, const char * path,
	WatcherPathRequest & pathRequest ) 
{
	bool handled = false;

	// Inform the path request of the number of children 
	pathRequest.addWatcherCount( container_.size() );

	if (isEmptyPath(path))
	{
		Container::iterator iter = container_.begin();

		while (iter != container_.end())
		{
			const void * addedBase = (const void*)(
				((const uintptr)base) + ((const uintptr)(*iter).base) );

			if (!pathRequest.addWatcherPath( addedBase, NULL,
									(*iter).label, *(*iter).watcher ))
			{
				break;
			}

			iter++;
		}

		handled = true;
	}
	else
	{
		DirData * pChild = this->findChild( path );

		if (pChild != NULL)
		{
			const void * addedBase = (const void*)(
				((const uintptr)base) + ((const uintptr)pChild->base) );

			handled = pChild->watcher->visitChildren( addedBase,
				this->tail( path ), pathRequest );
		}
	}

	return handled;
}



/*
 * Override from Watcher.
 */
bool DirectoryWatcher::addChild( const char * path, WatcherPtr pChild,
	void * withBase )
{
	bool wasAdded = false;

	if (isEmptyPath( path ))						// Is it invalid?
	{
		ERROR_MSG( "DirectoryWatcher::addChild: "
			"tried to add unnamed child (no trailing slashes please)\n" );
	}
	else if (strchr(path,'/') == NULL)				// Is it for us?
	{
		// Make sure that we do not add it twice.
		if (this->findChild( path ) == NULL)
		{
			DirData		newDirData;
			newDirData.watcher = pChild;
			newDirData.base = withBase;
			newDirData.label = path;

#ifdef SORTED_WATCHERS
			Container::iterator iter = container_.begin();
			while ((iter != container_.end()) &&
					(iter->label < newDirData.label))
			{
				++iter;
			}
			container_.insert( iter, newDirData );
#else
			container_.push_back( newDirData );
#endif
			wasAdded = true;
		}
		else
		{
			ERROR_MSG( "DirectoryWatcher::addChild: "
				"tried to replace existing watcher %s\n", path );
		}
	}
	else											// Must be for a child
	{
		DirData * pFound = this->findChild( path );

		if (pFound == NULL)
		{
			char * pSeparator = strchr( (char*)path, WATCHER_SEPARATOR );

			int compareLength = (pSeparator == NULL) ?
				strlen( (char*)path ) : (pSeparator - path);

			Watcher * pChildWatcher = new DirectoryWatcher();

			DirData		newDirData;
			newDirData.watcher = pChildWatcher;
			newDirData.base = NULL;
			newDirData.label = std::string( path, compareLength );

#ifdef SORTED_WATCHERS
			Container::iterator iter = container_.begin();
			while ((iter != container_.end()) &&
					(iter->label < newDirData.label))
			{
				++iter;
			}
			pFound = &*container_.insert( iter, newDirData );
#else
			container_.push_back( newDirData );
			pFound = &container_.back();
#endif
		}

		if (pFound != NULL)
		{
			wasAdded = pFound->watcher->addChild( this->tail( path ),
				pChild, withBase );
		}
	}

	return wasAdded;
}


/**
 *	This method removes a child identifier in the path string.
 *
 *	@param path		This string must start with the identifier that you are
 *					searching for. For example, "myDir/myValue" would match the
 *					child with the label "myDir".
 *
 *	@return	true if remove, false if not found
 */
bool DirectoryWatcher::removeChild( const char * path )
{
	if (path == NULL)
		return false;

	char * pSeparator = strchr( (char*)path, WATCHER_SEPARATOR );

	unsigned int compareLength =
		(pSeparator == NULL) ? strlen( path ) : (pSeparator - path);

	if (compareLength != 0)
	{
		Container::iterator iter = container_.begin();

		while (iter != container_.end())
        {
			if (compareLength == (*iter).label.length() &&
				strncmp(path, (*iter).label.data(), compareLength) == 0)
			{
				if( pSeparator == NULL )
				{
					container_.erase( iter );
					return true;
				}
				else
				{
					DirData* pChild = const_cast<DirData*>(&(const DirData &)*iter);
					return pChild->watcher->removeChild( tail( path ) );
				}
			}
			iter++;
		}
	}

	return false;
}

/**
 *	This method finds the immediate child of this directory matching the first
 *	identifier in the path string.
 *
 *	@param path		This string must start with the identifier that you are
 *					searching for. For example, "myDir/myValue" would match the
 *					child with the label "myDir".
 *
 *	@return	The watcher matching the input path. NULL if one was not found.
 */
DirectoryWatcher::DirData * DirectoryWatcher::findChild( const char * path ) const
{
	if (path == NULL)
	{
		return NULL;
	}

	char * pSeparator = strchr( (char*)path, WATCHER_SEPARATOR );

	unsigned int compareLength =
		(pSeparator == NULL) ? strlen( path ) : (pSeparator - path);

    DirData * pChild = NULL;

	if (compareLength != 0)
	{
		Container::const_iterator iter = container_.begin();

		while (iter != container_.end() && pChild == NULL)
        {
			if (compareLength == (*iter).label.length() &&
				strncmp(path, (*iter).label.data(), compareLength) == 0)
			{
				pChild = const_cast<DirData*>(&(const DirData &)*iter);
			}
			iter++;
		}
	}

	return pChild;
}


/**
 *	This method is used to find the string representing the path without the
 *	initial identifier.
 *
 *	@param path		The path that you want to find the tail of.
 *
 *	@return	A string that represents the remainder of the path.
 */
const char * DirectoryWatcher::tail( const char * path )
{
	if (path == NULL)
		return NULL;

	char * pSeparator = strchr( (char*)path, WATCHER_SEPARATOR );

	if (pSeparator == NULL)
		return NULL;

	return pSeparator + 1;
}




// -----------------------------------------------------------------------------
// Section: Constructors/Destructor
// -----------------------------------------------------------------------------

/**
 *	Constructor for Watcher.
 */
Watcher::Watcher( const char * comment ) :
	comment_( comment )
{
}


/**
 *	The destructor deletes all of its children.
 */
Watcher::~Watcher()
{
}


// -----------------------------------------------------------------------------
// Section: Static methods of Watcher
// -----------------------------------------------------------------------------

/**
 *	This is a simple helper method used to partition a path string into the name
 *	and directory strings. For example, "section1/section2/hello" would be split
 *	into "hello" and "section1/section2/".
 *
 *	@param path				The string to be partitioned.
 *	@param resultingName	A reference to a string that is to receive the base
 *							name.
 *	@param resultingDirectory	A reference to a string that is to receive the
 *							directory string.
 */
void Watcher::partitionPath( const std::string path,
		std::string & resultingName,
		std::string & resultingDirectory )
{
	int pos = path.find_last_of( WATCHER_SEPARATOR );

	if ( 0 <= pos && pos < (int)path.size( ) )
	{
		resultingDirectory	= path.substr( 0, pos + 1 );
		resultingName		= path.substr( pos + 1, path.length() - pos - 1 );
	}
	else
	{
		resultingName		= path;
		resultingDirectory	= "";
	}
}


/**
 *	Utility method to watch the count of instances of a type
 */
void watchTypeCount( const char * basePrefix, const char * typeName, int & var )
{
	std::string str = basePrefix;
	str += typeName;
	addWatcher( str.c_str(), var );
}


// -----------------------------------------------------------------------------
// Section: CallableWatcher
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
CallableWatcher::ArgDescription::ArgDescription(
		WatcherDataType type, const std::string & description ) :
	type_( type ),
	description_( description )
{
}


/**
 *	Constructor.
 */
CallableWatcher::CallableWatcher( ExposeHint hint, const char * comment ) :
	Watcher( comment ),
	exposeHint_( hint )
{
}


/**
 *	This method adds an argument description to this callable object.
 *
 *	@param type	The type of the argument.
 *	@param description A brief description of the argument.
 */
void CallableWatcher::addArg( WatcherDataType type, const char * description )
{
	argList_.push_back( ArgDescription( type, description ) );
}


/*
 *	Override from Watcher.
 */
bool CallableWatcher::getAsString( const void * base, const char * path,
	std::string & result, std::string & desc, Watcher::Mode & mode ) const
{
	if (isEmptyPath( path ))
	{
		result = "Callable function";
		mode = WT_CALLABLE;
		desc = comment_;

		return true;
	}

	return false;
}


/*
 *	Override from Watcher.
 */
bool CallableWatcher::setFromString( void * base, const char * path,
	const char * valueStr )
{
	ERROR_MSG( "PyFunctionWatcher::setFromString: Attempt to call python "
			"function watcher with old protocol (v1) not supported\n" );
	return false;
}


/*
 *	This static method returns whether the path is the __args__ path.
 */
bool CallableWatcher::isArgsPath( const char * path )
{
	if (path == NULL)
		return false;

	return (strcmp(path, "__args__") == 0);
}


/**
 *	This static method returns whether the path is the __expose__ path.
 */
bool CallableWatcher::isExposePath( const char * path )
{
	if (path == NULL)
		return false;

	return (strcmp(path, "__expose__") == 0);
}


/*
 *	Override from Watcher.
 */
bool CallableWatcher::getAsStream( const void * base, const char * path,
	WatcherPathRequestV2 & pathRequest ) const
{
	if (isEmptyPath( path ))
	{
		Watcher::Mode mode = Watcher::WT_CALLABLE;
		std::string value( "Callable function" );
		watcherValueToStream( pathRequest.getResultStream(),
			value, mode );
		pathRequest.setResult( comment_, mode, this, base );
		return true;
	}
	else if (isDocPath( path ))
	{
		// <watcher>/__doc__ handling
		Watcher::Mode mode = Watcher::WT_READ_ONLY;
		watcherValueToStream( pathRequest.getResultStream(), comment_, mode );
		pathRequest.setResult( comment_, mode, this, base );
		return true;
	}
	else if (this->isArgsPath( path ))
	{
		// <watcher>/__args__ handling
		int numArgs = argList_.size();
		Watcher::Mode mode = WT_CALLABLE;

		BinaryOStream & resultStream = pathRequest.getResultStream();

		resultStream << (uchar)WATCHER_TYPE_TUPLE;
		resultStream << (uchar)mode;

		// In order to make decoding of the stream re-useable, we throw the
		// size of the next segment (ie: the count and the tuple types) on
		// before the count of the number of tuple elements. It seems like
		// a tiny bit of bloat but makes the decoding code a lot easier.

		MemoryOStream tmpResult;

		// Add the number of arguments for the function
		tmpResult.writeStringLength( numArgs );

		// Now for each argument add the name / type
		for (int i = 0; i < numArgs; i++)
		{
			// tuple
			tmpResult << (uchar)WATCHER_TYPE_TUPLE;
			// mode
			tmpResult << (uchar)Watcher::WT_READ_ONLY;

			// Create the contents of the argument element tuple
			MemoryOStream argStream;
			// count = 2
			argStream.writeStringLength( 2 ); // Arg name + type
			watcherValueToStream( argStream, argList_[i].description(),
									Watcher::WT_READ_ONLY );
			watcherValueToStream( argStream, argList_[i].type(),
									Watcher::WT_READ_ONLY );

			// size
			tmpResult.writeStringLength( argStream.size() );
			tmpResult.addBlob( argStream.data(), argStream.size() );
		}

		// Prefix the entire argument description with the desc size.
		resultStream.writeStringLength( tmpResult.size() );
		resultStream.addBlob( tmpResult.data(), tmpResult.size() );
		pathRequest.setResult( comment_, mode, this, base );

		return true;
	}
	else if (this->isExposePath( path ))
	{
		Watcher::Mode mode = Watcher::WT_READ_ONLY;
		watcherValueToStream( pathRequest.getResultStream(),
				(int32)exposeHint_, mode );
		pathRequest.setResult( comment_, mode, this, base );
		return true;
	}

	return false;
}


// -----------------------------------------------------------------------------
// Section: NoArgCallableWatcher
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
NoArgCallableWatcher::NoArgCallableWatcher(
		ExposeHint hint, const char * comment ) :
	CallableWatcher( hint, comment )
{
}


/*
 *	Override from Watcher.
 */
bool NoArgCallableWatcher::setFromStream( void * base,
		const char * path,
		WatcherPathRequestV2 & pathRequest )
{
	// TODO: Should check that there are not too many arguments.
	std::string value = "No value";
	std::string output = "";

	if (this->onCall( output, value ))
	{
		BinaryOStream & resultStream = pathRequest.getResultStream();
		resultStream << (uint8)WATCHER_TYPE_TUPLE;
		resultStream << (uint8)WT_READ_ONLY;

		const int TUPLE_COUNT = 2;

		int resultSize = BinaryOStream::calculatePackedIntSize( TUPLE_COUNT ) +
			this->tupleElementStreamSize( output.size() ) +
			this->tupleElementStreamSize( value.size() );

		resultStream.writeStringLength( resultSize );

		resultStream.writeStringLength( TUPLE_COUNT );

		watcherValueToStream( resultStream, output, WT_READ_ONLY );
		watcherValueToStream( resultStream, value, WT_READ_ONLY );

		pathRequest.setResult( "", WT_READ_ONLY, this, base );

		return true;
	}
	else
	{
		WARNING_MSG( "NoArgCallableWatcher::setFromStream: "
				"Function call failed\n" );
		return false;
	}
}


// -----------------------------------------------------------------------------
// Section: NoArgFuncCallableWatcher
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
NoArgFuncCallableWatcher::NoArgFuncCallableWatcher( Function func,
		ExposeHint hint, const char * comment ) :
	NoArgCallableWatcher( hint, comment ),
	func_( func )
{
}

bool NoArgFuncCallableWatcher::onCall(
		std::string & output, std::string & value )
{
	return (*func_)( output, value );
}


// -----------------------------------------------------------------------------
// Section: DereferenceWatcher
// -----------------------------------------------------------------------------

DereferenceWatcher::DereferenceWatcher( WatcherPtr watcher, void * withBase ) :
	watcher_( watcher ),
	sb_( (uintptr)withBase )
{ }

bool DereferenceWatcher::getAsString( const void * base, const char * path,
									  std::string & result, std::string & desc,
									  Watcher::Mode & mode ) const
{ 
	return (base == NULL) ? false :
		watcher_->getAsString( (void*)(sb_ + dereference(base)), path, 
							   result, desc, mode ); 
}

bool DereferenceWatcher::setFromString( void * base, const char * path,
										const char * valueStr )
{ 
	return (base == NULL) ? false :
		watcher_->setFromString( (void*)(sb_ + dereference(base)), path, 
								 valueStr ); 
}

bool DereferenceWatcher::getAsStream( const void * base, const char * path,
									  WatcherPathRequestV2 & pathRequest ) const
{ 
	return (base == NULL) ? false :
		watcher_->getAsStream( (void*)(sb_ + dereference(base)), path, 
							   pathRequest ); 
}

bool DereferenceWatcher::setFromStream( void * base, const char * path,
										WatcherPathRequestV2 & pathRequest )
{
	return (base == NULL) ? false :
		watcher_->setFromStream( (void*)(sb_ + dereference(base)), path, 
								 pathRequest ); 
}

bool DereferenceWatcher::visitChildren( const void * base, const char * path,
										WatcherPathRequest & pathRequest ) 
{
	return (base == NULL) ? false :
		watcher_->visitChildren( (void*)(sb_ + dereference(base)), path, 
								 pathRequest ); 
}


bool DereferenceWatcher::addChild( const char * path, WatcherPtr pChild,
								   void * withBase )
{ 
	return watcher_->addChild( path, pChild, withBase ); 
}


// -----------------------------------------------------------------------------
// Section: Helper functions
// -----------------------------------------------------------------------------

/**
 *	This helper function is used to watch the component priorities.
 */
int bwWatchInt( int & value, const char * path )
{
	WatcherPtr pWatcher = new DataWatcher<int>( value,
										Watcher::WT_READ_WRITE );
	Watcher::rootWatcher().addChild( path, pWatcher );

	return 0;
}

#endif // ENABLE_WATCHERS


// watcher.cpp
