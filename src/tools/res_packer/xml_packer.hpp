/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __XML_PACKER_HPP__
#define __XML_PACKER_HPP__


#include "base_packer.hpp"
#include "packers.hpp"

#include "resmgr/datasection.hpp"

#include <string>

/**
 *	This class converts all files begining with a '<' to a packed XML section.
 *	It adds itself to the packers as a LOWEST_PRIORITY packer, so it should be
 *	the last one to be executed, trying to process any file extension that
 *  hasn't been caught by the other packers. It also copies sections that are
 *	already packed.
 *	It is important that this class adds itself as LOWEST_PRIORITY, to allow
 *	other packers, such as the ChunkPacker, to process its XML-based file(s)
 *	first.
 */
class XmlPacker : public BasePacker
{
public:
	virtual bool prepare( const std::string & src, const std::string & dst );
	virtual bool print();
	virtual bool pack();

	static void shouldEncrypt( bool value ) 
		{ s_shouldEncrypt = value; }

	static bool shouldEncrypt() 
		{ return s_shouldEncrypt; }

private:
	void printRecursive( DataSectionPtr pDS, int indent = 0 );

	DECLARE_PACKER()

	DataSectionPtr ds_;
	std::string src_;
	std::string dst_;

	static bool s_shouldEncrypt;

};

#endif // __XML_PACKER_HPP__
