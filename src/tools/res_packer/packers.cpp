/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#include "packers.hpp"


Packers& Packers::instance()
{
	static Packers s_instance_;
	return s_instance_;
}

void Packers::add( BasePacker* packer, unsigned short priority )
{
	if ( !packer )
		return;

	// insert ordered by priority
	Items::iterator i = packers_.begin();
	for( ; i != packers_.end(); ++i )
	{
		if ( (*i).first > priority )
			break;
	}
	packers_.insert( i, Item( priority, packer ) );
}

BasePacker* Packers::find( const std::string& src, const std::string& dst )
{
	// return the first packer that can handle the data
	for( Items::iterator i = packers_.begin();
		i != packers_.end(); ++i )
	{
		if ( (*i).second->prepare( src, dst ) )
			return (*i).second;
	}
	return NULL;
}
