/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __PACKERS_HPP__
#define __PACKERS_HPP__


#include <string>
#include <vector>
#include "base_packer.hpp"

/**
 *  Packers
 */
class Packers
{
private:
	typedef std::pair<unsigned short,BasePacker*> Item;
	typedef std::vector<Item> Items;

public:
	static const unsigned short HIGHEST_PRIORITY	= 0;
	static const unsigned short LOWEST_PRIORITY		= 0xFFFF;

	static Packers& instance();

	void add( BasePacker* packer, unsigned short priority );

	BasePacker* find( const std::string& src, const std::string& dst );

private:
	Packers() {};

	Items packers_;
};


class PackerFactory
{
public:
	PackerFactory( BasePacker* packer, unsigned short priority = Packers::HIGHEST_PRIORITY )
	{
		Packers::instance().add( packer, priority );
	}
};

#define DECLARE_PACKER()		\
	static PackerFactory s_packer_factory_;
#define IMPLEMENT_PACKER( P )	\
	PackerFactory P::s_packer_factory_( new P() );

#define IMPLEMENT_PRIORITISED_PACKER( P, PRIORITY )		\
	PackerFactory P::s_packer_factory_( new P(), PRIORITY );

#endif // __PACKERS_HPP__
