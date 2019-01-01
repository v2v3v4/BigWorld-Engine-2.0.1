/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_PYTHON_MAPPING_HPP
#define MYSQL_PYTHON_MAPPING_HPP

#include "string_like_mapping.hpp"

class Pickler;

/**
 *	This class maps the PYTHON type to the database.
 */
class PythonMapping : public StringLikeMapping
{
public:
	PythonMapping( const Namer & namer, const std::string & propName,
			bool isNameIndex, int length, DataSectionPtr pDefaultValue );

	virtual bool isBinary() const	{ return true; }

private:
	// This method evaluates expr as a Python expression, pickles the
	// resulting object and stores it in output.
	static void pickleExpression( std::string & output,
			const std::string & expr );

	static Pickler & getPickler();
};

#endif // MYSQL_PYTHON_MAPPING_HPP
