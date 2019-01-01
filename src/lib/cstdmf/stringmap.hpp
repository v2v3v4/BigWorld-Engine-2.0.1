/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STRING_MAP_HPP
#define STRING_MAP_HPP

#include "cstdmf/stdmf.hpp"
#include <vector>
#include <string>
#include <cstring>
#ifdef _WIN32
#include <hash_map>
#elif defined( PLAYSTATION3 )
#else
#include <ext/hash_map>
#endif

//#define STRINGMAP_DEBUG

#ifdef STRINGMAP_DEBUG
	#include <iostream>
	extern void watchTypeCount( const char * basePrefix,
		const char * typeName, int & var );
	#include "typeinfo"
#endif


#if !defined( PLAYSTATION3 )

#ifdef _WIN32
#define HASH_MAP_NAMESPACE stdext
#else
#define HASH_MAP_NAMESPACE __gnu_cxx
#endif

// This provides specialisation for hashing std::string under GCC
#ifndef _WIN32
namespace __gnu_cxx
{
	template<>
	struct hash<std::string>
	{
		size_t operator()(const std::string &str) const
		{
			return hash_(str.c_str());
		}

	private:
		hash<const char *>		hash_;
	};
}
#endif


/**
 *	This class provides a fast-lookup for objects based on a std::string.
 *
 *  This class has 90% of the functionality of StringMap with less issues in
 *  terms of what strings are allowed, capacities etc.
 */
template<typename _Ty>
class StringHashMap : public HASH_MAP_NAMESPACE::hash_map<std::string, _Ty>
{
};

template<typename _Ty>
class WStringHashMap : public HASH_MAP_NAMESPACE::hash_map<std::wstring, _Ty>
{
};

#endif

/**
 *	Please use StringHashMap defined above instead of this class.  It has a 
 *	similar interface and speed but has less issues with non-English languages 
 *	and uses less memory.
 *
 *	This map is intended to be fast to look up. (i.e. StringMap::find).
 *	It tries to use memory wisely but doesn't let it impact speed:
 *
 *		base memory overhead = 256 bytes
 *		for each string added, it can be split up into these segments:
 *			'existing shared prefix' + 'new shared prefix' +
 *				'unshared remainder'
 *			extra memory overhead for each string =
 *				256 * len(new shared prefix) bytes
 *	Addition and removal is not brilliant, but it's not terribly
 *		slow either. Unless values are deleted they will remain in the
 *		order they were added.
 *	There are two size limits:
 *		#(entries) <= numeric_limits<_I>::max(), and
 *		#(sum(len(new shared prefix))) < numeric_limits<_I>::max()
 *	These could be removed (and memory overhead doubled) by doubling the
 *		size of the index type; the second template argument.
 *	Alternatively, you could use half the memory by halving the size
 *		of the index type, with a max of 127 entries.
 *	Whatever size you make it, the type should be a signed integer.
 *
 *	@note No allocator because I can't figure out how to get it working with
 *	vector and pair properly.
 */
template < class _Ty, class _I = int16 > class StringMap
{
public:
	// Typedefs
	/// This type is a shorthand for the map type.
	typedef StringMap<_Ty,_I> _Myt;

	/// This type is the type of the elements in the map.
	typedef std::pair< const char *, _Ty > value_type;
	/// This type is the type of the key.
	typedef const char * key_type;
	/// This type is the type of the referent.
	typedef _Ty referent_type;
	/// This type is a reference to referent type.
	typedef _Ty & _Tref;
	/// This type is the index type.
	typedef _I index_type;

	/// Type of the container.
	typedef std::vector<value_type> _Imp;
	/// Type of the size.
	typedef typename _Imp::size_type size_type;
	/// Type of a difference
	typedef typename _Imp::difference_type difference_type;
	/// Type of a reference.
	typedef typename _Imp::reference reference;
	/// Type of a const reference.
	typedef typename _Imp::const_reference const_reference;
	/// The iterator type.
	typedef typename _Imp::iterator iterator;
	/// The const iterator type.
	typedef typename _Imp::const_iterator const_iterator;
	/// The reverse iterator type.
	typedef typename _Imp::reverse_iterator reverse_iterator;
	/// The const reverse iterator type.
	typedef typename _Imp::const_reverse_iterator const_reverse_iterator;

	/// @name Construction/Destruction
	//@{
	StringMap()
	{
		tables_.push_back(new Table());
	}

	StringMap( const _Myt & toCopy )
	{
		this->copy( toCopy );
	}

	~StringMap()
	{
		this->destruct();
	}

	_Myt & operator=( const _Myt & toCopy )
	{
		if (this != &toCopy)
		{
			this->total_clear();
			this->copy( toCopy );
		}
		return *this;
	}
	//@}

	// ---- Iterators and others handled by the implementor class ----

	/// This method returns an iterator for STL style iteration.
	iterator begin()						{ return (entries_.begin()); }
	/// This method returns an iterator for STL style iteration.
	const_iterator begin() const			{ return (entries_.begin()); }
	/// This method returns an iterator for STL style iteration.
	iterator end()							{ return (entries_.end()); }
	/// This method returns an iterator for STL style iteration.
	const_iterator end() const				{ return (entries_.end()); }

	/// This method returns a reverse iterator for STL style iteration.
	reverse_iterator rbegin()				{ return (entries_.rbegin()); }
	/// This method returns a reverse iterator for STL style iteration.
	const_reverse_iterator rbegin() const	{ return (entries_.rbegin()); }
	/// This method returns a reverse iterator for STL style iteration.
	reverse_iterator rend()					{ return (entries_.rend()); }
	/// This method returns a reverse iterator for STL style iteration.
	const_reverse_iterator rend() const		{ return (entries_.rend()); }

	/// This method returns the number of elements in this container.
	size_type size() const					{ return (entries_.size()); }

	/**
	 *	This method returns whether or not this container is empty.
	 *
	 *	@return True if empty, otherwise false.
	 */
	bool empty() const						{ return (entries_.empty()); }

	// ---- std::string methods ----

	/**
	 *	This method erases the element associated with the input key.
	 *
	 *	@param key	The key associated with the element to delete.
	 */
	bool erase( const std::string & key )
		{ return this->erase( key.c_str() ); }

	/**
	 *	This method returns an iterator referring to the element associated with
	 *	the input key.
	 *
	 *	@param key	The key associated with the element to find.
	 *
	 *	@return	If found, an iterator referring to the found element, otherwise
	 *		an iterator referring to the end.
	 */
	iterator find( const std::string key )
		{ return this->find( key.c_str() ); }

	/**
	 *	This method returns an iterator referring to the element associated with
	 *	the input key.
	 *
	 *	@param key	The key associated with the element to find.
	 *
	 *	@return	If found, an iterator referring to the found element, otherwise
	 *		an iterator referring to the end.
	 */
	const_iterator find( const std::string key ) const
		{ return this->find( key.c_str() ); }

	/**
	 *	This method implements the index operator that takes a std::string.
	 *
	 *	@param key	The key of the value to look up.
	 *
	 *	@return The value associated with the input key. If no such value is
	 *		found, a new one with a default value is added to the map and
	 *		returned.
	 */
	referent_type & operator[]( const std::string key )
		{ return this->operator[]( key.c_str() ); }

	/**
	 *	This method returns the number of bytes of dynamic memory
	 *	allocated by this map. Note that it is quite slow.
	 */
	uint32 sizeInBytes() const
	{
		uint32 sz = 0;
		sz += entries_.capacity() * sizeof(value_type);
		sz += tables_.capacity() * sizeof(Table*);
		sz += tables_.size() * sizeof(Table);
		for (uint i = 0; i < entries_.size(); i++)
			sz += strlen( entries_[i].first ) + 1;
		return sz;
	}

	/**
	 *	This method returns the sume of dynamic memory used by the map,
	 *	including heap overheads. It is not fast.
	 */
	uint32 memoryUse() const;

	/**
	 *	This method returns the maximum size the map can achieve.
	 */
	size_type max_size() const
	{
		return 1UL << ((8 * sizeof(index_type)) - 1);
	}

	/**
	 *	This method inserts the element between the input iterators into the
	 *	map.
	 *
	 *	@param _F	An iterator referring to the first element to insert.
	 *	@param _L	An iterator referring to the element after the last element
	 *				to insert.
	 */
	void insert( iterator _F, iterator _L )
	{
		for (; _F != _L; ++_F)
			this->insert(*_F);
	}

	/**
	 *	This method removes the elements between the input iterators from this
	 *	map.
	 *
	 *	@param _F	An iterator referring to the first element to remove.
	 *	@param _L	An iterator referring to the element after the last element
	 *				to remove.
	 */
	void erase( iterator _F, iterator _L )
	{
		for (; _F != _L; --_L)
			this->erase(_L);
	}

	/**
	 *	This method erases the element associated with the input key.
	 *
	 *	@param key	The key associated with the element to erase.
	 *
	 *	@return	True if the element was erased, otherwise false.
	 */
	bool erase( const key_type key )
	{
		iterator found = this->find( key );
		if (found == this->end()) return false;
		this->erase( found );
		return true;
	}


	/**
	 *	This method clears this map. After the call, there are no elements in
	 *	the map.
	 */
	void clear()
	{
		this->total_clear();
		tables_.push_back(new Table());
	}

	/**
	 *	This method returns a iterator that refers to the element associated
	 *	with the input key.
	 *
	 *	@param key	The key of the element to find.
	 *
	 *	@return	If found, returns an iterator referring to the element,
	 *			otherwise returns an iterator to the end of the map.
	 */
	iterator find( const key_type key )
	{
		return this->find_and_insert( key, NULL );
	}

	/**
	 *	This method returns a iterator that refers to the element associated
	 *	with the input key.
	 *
	 *	@param key	The key of the element to find.
	 *
	 *	@return	If found, returns an iterator referring to the element,
	 *			otherwise returns an iterator to the end of the map.
	 */
	const_iterator find( const key_type key ) const
	{
		return const_cast<_Myt*>(this)->find( key );
	}		// it is, trust me!

	/**
	 *	This method inserts the input element into the map.
	 *
	 *	@param val	The element to insert.
	 *
	 *	@return	An iterator pointing to the newly inserted element.
	 */
	iterator insert( const value_type& val )
	{
		return this->find_and_insert( val.first, &val.second );
	}

	/**
	 *	This method erases the element referred to by the input iterator.
	 */
	void erase( iterator iter )
	{
		this->erase_iterator( iter );
	}

	/**
	 *	This method implements the index operator.
	 */
	referent_type & operator[]( const key_type key )
	{
		iterator i = this->find_and_insert( key, NULL );
		if (i == end())
		{
			referent_type	rt = referent_type();
			i = this->find_and_insert( key, &rt );
		}
		return i->second;
	}

private:

	// Some utility functions
	void copy( const _Myt & toCopy )
	{
		for (typename Tables::const_iterator i = toCopy.tables_.begin();
			i != toCopy.tables_.end();
			i++)
		{
			tables_.push_back( new Table(**i) );
		}

		for (typename _Imp::const_iterator i = toCopy.entries_.begin();
			i != toCopy.entries_.end();
			i++)
		{
			entries_.push_back( *i );
			value_type & rvt = entries_.back();
			int len = strlen(i->first) + 1;
			rvt.first = new char[len];
			memcpy( (char*)rvt.first, i->first, len );
		}
	}

	void destruct()
	{
		for (typename Tables::iterator i = tables_.begin();
				i != tables_.end(); i++)
		{
			delete *i;
		}

		for (typename _Imp::iterator i = entries_.begin();
				i != entries_.end(); i++)
		{
			delete [] (char*)(i->first);
		}
	}

	void total_clear()
	{
		this->destruct();
		tables_.clear();
		entries_.clear();
	}

	// The guts of it...


	// the big find and insert
	iterator find_and_insert(const key_type key_in, const referent_type * pRef )
	{
		key_type key = key_in;

		index_type	tableIndex = 0;
		index_type	lastTableIndex;
		unsigned char	kk;
		do
		{
			kk = *key++;	// post-increment intentional
			lastTableIndex = tableIndex;
			tableIndex = tables_[tableIndex]->e[kk];
		} while (tableIndex > 0 && kk != 0);
			// theoretically we don't need to check kk!=0, because
			// table->e[0] will always be 0 or negative

		// if we stopped because we ran out of string it's not there.
		// same if we stopped because there was a zero there
		if (tableIndex >= 0)
		{
			//if (kk == 0) { MF_ASSERT( tableIndex == 0 ); }

			if (pRef == NULL) return entries_.end();
			return this->simple_insert( *pRef, lastTableIndex, key_in, kk );
		}

		// ok, we know where to look now
		iterator i = entries_.begin() + ~tableIndex;

		// check anything that's left of the string
		//  (pretty much just strcmp now)
		int key_dist = (key - key_in) - 1;
		key_type ekey = i->first + key_dist;
		unsigned char ekk = *ekey++;
		while( kk != 0 && ekk == kk )
		{
			kk = *key++;
			ekk = *ekey++;
		}

		// if they both got to their ends simultaneously, then we found it
		if (kk == 0 && ekk == 0)
		{			// ok, exact replacement ... easy
			if (pRef != NULL) i->second = *pRef;
			return i;
		}
		else
		{
			if (pRef == NULL) return entries_.end();
			return this->complicated_insert( *pRef, lastTableIndex, key_in, i,
				key_dist );
		}
	}


	// insert into the table indicated
	iterator simple_insert( const referent_type & ref, index_type tableIndex,
		key_type key_in, unsigned char kk )
	{
		// first put it in entries
		int len = std::strlen( key_in ) + 1;
		// cast to char* so borland can compile.
		entries_.push_back( std::make_pair( (const char *)(new char[len]), ref ) );
		// damn - I wish I could avoid the double-copy of the
		//  referent type above, but I can't see how to do it
		//  without using its (possibly nonexistent) default constructor.
		iterator i = entries_.begin() + (entries_.size() - 1);
		memcpy( (char*)i->first, key_in, len );

		// now put it in our tables
		tables_[tableIndex]->e[kk] = ~( i - entries_.begin() );

		return i;
	}


	// create new tables for as long as we match with our contender
	// note that this function assumes that the two strings are NOT identical
	iterator complicated_insert( const referent_type & ref, index_type tableIndex,
		key_type key_in, iterator contender, int key_dist )
	{
		key_type	new_key = key_in + key_dist;
		key_type	ext_key = contender->first + key_dist;

		// add as many tables as are necessary
		Table * pOldTable = tables_[tableIndex];

		unsigned char	nkk,	ekk;
		while ((nkk = *new_key++) == (ekk = *ext_key++))
		{
			Table * pNewTable = new Table();
			tables_.push_back( pNewTable );

			tableIndex = (&tables_.back()) - (&tables_.front());
			pOldTable->e[nkk] = tableIndex;

			pOldTable = pNewTable;
		}

		// put the existing one back in the latest table
		pOldTable->e[ekk] = ~( contender - entries_.begin() );

		// and add the new one in the normal fashion
		return this->simple_insert( ref, tableIndex, key_in, nkk );
	}


	// your everyday erase
	void erase_iterator( iterator iter )
	{
		key_type	key = iter->first;

		// First erase it from the tables

		// find the entry in the tables (assumes it's there)
		index_type	tableIndex = 0;
		index_type	lastTableIndex;
		unsigned char	kk;
		do
		{
			kk = *key++;	// post-increment intentional
			lastTableIndex = tableIndex;
			tableIndex = tables_[tableIndex]->e[kk];
		} while (tableIndex > 0);

		// ok, we found it, so set it to 0
		tables_[lastTableIndex]->e[kk] = 0;

		// free the memory for the string
		delete [] (char*)(iter->first);

		// note that tables are never deleted once created,
		//  unless clear is used.

		// Now erase it from the entries

		// is this the last entry in the vector of entries?
		if (iter != entries_.begin() + entries_.size() - 1)
		{
			// ok, copy the last entry on top of ourselves
			*iter = entries_.back();
				// we can't erase the back of the vector until
				// we've finished with this iterator, 'coz it
				// could be made invalid if the vector moves
				// (not that it should move when being made smaller,
				// but you never know)

			// and fix it up in the tables (assumes it's there)
			key = iter->first;
			tableIndex = 0;
			do
			{
				kk = *key++;
				lastTableIndex = tableIndex;
				tableIndex = tables_[tableIndex]->e[kk];
			} while (tableIndex > 0);

			// ok, we found it, so set it to its new value
			tables_[lastTableIndex]->e[kk] = ~( iter - entries_.begin() );
		}

		// now erase the last iterator
		entries_.erase( entries_.begin() + entries_.size() - 1);

		// And that's it. phew!
	}




	// Data structures

	struct Table
	{
#ifndef STRINGMAP_DEBUG
		Table() { memset(e,0,sizeof(e)); }
#else
		Table()	{ memset(e,0,sizeof(e)); modTableCount( 1 ); }
		Table( const Table & t ) { *this = t; modTableCount( 1 ); }
		~Table() { modTableCount( -1 ); }
#endif

		index_type	e[256];
	};

	typedef std::vector<Table*>	Tables;
	Tables		tables_;
	_Imp		entries_;

#ifdef STRINGMAP_DEBUG
	static void modTableCount( int way )
	{
		static int tableCount = 0x80000000;
		if (tableCount == 0x80000000)
		{
			tableCount = 0;
			watchTypeCount(
				"StringMapTables/", typeid(_Ty).name(), tableCount );
		}
		tableCount += way;
	}

	friend std::ostream& operator<<(std::ostream&, const _Myt&);
#endif
};


#ifdef STRINGMAP_DEBUG
template< class _Ty, class _I > std::ostream & operator<<(
	std::ostream& o, const StringMap<_Ty,_I> & dmap)
{
	typedef StringMap<_Ty,_I>	MAP;

	o << "Entries: " << std::endl;
	int	n = 0;
	for (MAP::const_iterator i = dmap.entries_.begin();
		i != dmap.entries_.end(); i++)
	{
		o << n << ": '" << i->first << "' -> " << i->second << std::endl;
		n++;
	}
	o << std::endl;

	n = 0;
	for (MAP::Tables::const_iterator i = dmap.tables_.begin();
		i != dmap.tables_.end(); i++ )
	{
		o << "Table " << n << ":" << std::endl;
		for (int j = 0; j < 128; j++)
		{
			MAP::index_type	k = (*i)->e[j];
			if (k>0)
				o << j << ": " << "Table " << k << std::endl;
			else if (k<0)
				o << j << ": " << "Entry " << ~k << std::endl;
		}
		o << std::endl;
		n++;
	}
	o << "Done" << std::endl << std::endl;

	return o;
}
#endif // STRINGMAP_DEBUG


template <class T, class I> uint32 memoryUse( const StringMap<T,I> & obj )
{ return obj.memoryUse(); }

template < class _Ty, class _I >
uint32 StringMap< _Ty, _I >::memoryUse() const
{
	uint32 sz = 0;
	sz += ::memoryUse( entries_ );
	for (uint i = 0; i < entries_.size(); i++)
		sz += ::memoryUse( entries_[i].first );
	sz += ::memoryUse( tables_ );
	for (uint i = 0; i < tables_.size(); i++)
		sz += ::memoryUse( tables_[i] );
	return sz;
}


#endif // STRING_MAP_HPP
