/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef META_DATA_HELPER_HPP
#define META_DATA_HELPER_HPP


#include "meta_data.hpp"

namespace MetaData
{

void updateCreationInfo( MetaData& metaData,
	time_t time = Environment::instance().time(),
	const std::string& username = Environment::instance().username() );

void updateModificationInfo( MetaData& metaData,
	time_t time = Environment::instance().time(),
	const std::string& username = Environment::instance().username() );

}

#endif//META_DATA_HELPER_HPP
