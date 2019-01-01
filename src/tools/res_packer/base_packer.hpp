/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __BASE_PACKER_HPP__
#define __BASE_PACKER_HPP__

#include <string>

class BasePacker
{
public:
	/**
	 *	Returns true if it can process the files, and if so, prepares itself.
	 */
	virtual bool prepare( const std::string& src, const std::string& dst ) = 0;

	/**
	 *	Output the class's string representation to stdout. Useful for XML.
	 */
	virtual bool print() = 0;

	/**
	 *	Pack the resource. Usualy requires copying the file to the destination,
	 *	and doing whatever processing is required.
	 */
	virtual bool pack() = 0;
};

#endif // __BASE_PACKER_HPP__
