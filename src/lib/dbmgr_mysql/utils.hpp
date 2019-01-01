/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include <string>

#include "mappings/property_mapping.hpp"

class BinaryOStream;

std::string buildCommaSeparatedQuestionMarks( int num );

void defaultSequenceToStream( BinaryOStream & strm, int seqSize,
		PropertyMappingPtr pChildMapping_ );

std::string createInsertStatement( const std::string & tbl,
		const PropertyMappings & properties );

std::string createUpdateStatement( const std::string & tbl,
		const PropertyMappings & properties );

std::string createSelectStatement( const std::string & tbl,
		const PropertyMappings & properties,
		const std::string & where );

// mysql_utils.hpp
