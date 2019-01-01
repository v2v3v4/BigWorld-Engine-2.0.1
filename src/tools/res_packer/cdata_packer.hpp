/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __CDATA_PACKER_HPP__
#define __CDATA_PACKER_HPP__


#include "packers.hpp"
#include "packers.hpp"

#include <string>

/**
 *	This class strips the .cdata files of unwanted data. In the client, it
 *	removes navmesh and thumbnail sections, and in the server only the 
 *	thumbnail section.
 */
class CDataPacker : public BasePacker
{
public:
	virtual bool prepare( const std::string & src, const std::string & dst );
	virtual bool print();
	virtual bool pack();

	static void addStripSection( const char * sectionName );

private:
	DECLARE_PACKER()
	std::string src_;
	std::string dst_;
};

#endif // __CDATA_PACKER_HPP__
