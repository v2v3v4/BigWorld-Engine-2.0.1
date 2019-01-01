/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UPDATABLES_HPP
#define UPDATABLES_HPP

#include <vector>

class Updatable;

class Updatables
{
public:
	Updatables( int numLevels = 2 );

	bool add( Updatable * pObject, int level );
	bool remove( Updatable * pObject );

	void call();

	size_t size() const	{ return objects_.size(); }

private:
	Updatables( const Updatables & );
	Updatables & operator=( const Updatables & );

	void adjustForAddedObject();

	std::vector< Updatable * > objects_;
	std::vector< int > levelSize_;
	bool inUpdate_;
	int deletedObjects_;
};

#endif // UPDATABLES_HPP
