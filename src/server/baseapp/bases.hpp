/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASES_HPP
#define BASES_HPP

#include "network/basictypes.hpp"
#include <map>

class Base;


/**
 *	This class represents a collection Base entities mapped by EntityID.
 */
class Bases
{
private:
	typedef std::map< EntityID, Base * > Container;

public:
	typedef Container::iterator iterator;
	typedef Container::const_iterator const_iterator;

	typedef Container::size_type size_type;
	typedef Container::mapped_type mapped_type;
	typedef Container::key_type key_type;

	iterator begin()					{ return container_.begin(); }
	const_iterator begin() const		{ return container_.begin(); }

	iterator end()						{ return container_.end(); }
	const_iterator end() const			{ return container_.end(); }

	size_type size() const				{ return container_.size(); }

	bool empty() const					{ return container_.empty(); }

	iterator find( key_type key )		{ return container_.find( key ); }

	Base * findEntity( EntityID id ) const;

	void add( Base * pBase );
	void erase( EntityID id )			{ container_.erase( id ); }
	void clear()						{ return container_.clear(); }

	void discardAll();

private:
	Container container_;
};

#endif // BASES_HPP
