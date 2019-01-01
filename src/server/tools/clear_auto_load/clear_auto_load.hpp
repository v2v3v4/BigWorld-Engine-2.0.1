/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CLEAR_AUTO_LOAD_HPP
#define CLEAR_AUTO_LOAD_HPP

#include "dbmgr_mysql/database_tool_app.hpp"

#include "dbmgr_mysql/mappings/entity_type_mappings.hpp"

class ClearAutoLoad : public DatabaseToolApp
{
public:
	ClearAutoLoad();
	virtual ~ClearAutoLoad();

	bool init( bool isVerbose );

	virtual bool run();

private:
	bool checkTablesExist();
	void deleteAutoLoadEntities();

	EntityTypeMappings entityTypeMappings_;
};

#endif // CLEAR_AUTO_LOAD_HPP
