/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NAMED_OBJECT_HPP
#define NAMED_OBJECT_HPP

#include <string>
#include "cstdmf/stringmap.hpp"


/**
 *	This class keeps named objects that register themselves into
 *	a global pool. The pool is searched by name.
 */
template <class Type> class NamedObject
{
public:
	typedef NamedObject<Type> This;
	
	NamedObject( const std::string & name, Type object ) :
		name_( name ),
		object_( object )
	{
		This::add( this );
	}
	~NamedObject()
	{
		This::del( this );
	}

	static Type get( const std::string & name )
	{
		if (pMap_ == NULL) return NULL;

		ObjectMap::iterator it = pMap_->find( name );
		if (it == pMap_->end()) return NULL;

		return it->second->object_;
	}

	const std::string & name() const { return name_; }

private:
	NamedObject( const NamedObject & no );
	NamedObject & operator=( const NamedObject & no );

	std::string name_;
	Type		object_;

	static void add( This * f )
	{
		if (pMap_ == NULL) pMap_ = new ObjectMap();
		(*pMap_)[ f->name_ ] = f;
	}

	static void del( This * f )
	{
		if (pMap_ == NULL) return;

		ObjectMap::iterator it = pMap_->find( f->name_ );
		if (it == pMap_->end() || it->second != f) return;

		pMap_->erase( it );

		if (pMap_->empty())
		{
			delete pMap_;
			pMap_ = NULL;
		}
	}

	typedef typename StringHashMap<This*> ObjectMap;
	static ObjectMap * pMap_;
};




#endif // NAMED_OBJECT_HPP