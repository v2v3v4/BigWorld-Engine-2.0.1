/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DB_CONSOLIDATOR_ERRORS_HPP
#define DB_CONSOLIDATOR_ERRORS_HPP

#include "cstdmf/debug.hpp"

#include <set>
#include <string>

/**
 * 	This class collects all the errors during consolidation.
 */
class DBConsolidatorErrors
{
public:
	void addSecondaryDB( const std::string & secondaryDBFileName )
	{
		HACK_BACKTRACE();

		erroneousSecondaryDBs_.insert( secondaryDBFileName );
	}

	bool hasErrors() const 
		{ return !erroneousSecondaryDBs_.empty(); }

	bool secondaryDBHasError( const std::string & secondaryDBFileName ) const
	{
		return (erroneousSecondaryDBs_.find( secondaryDBFileName ) !=
				erroneousSecondaryDBs_.end());
	}

private:
	typedef std::set< std::string > Container;
	Container erroneousSecondaryDBs_;
};

#endif // DB_CONSOLIDATOR_ERRORS_HPP
