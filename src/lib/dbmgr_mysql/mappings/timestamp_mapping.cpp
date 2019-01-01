/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "timestamp_mapping.hpp"

/**
 *	Constructor.
 */
TimestampMapping::TimestampMapping() :
	PropertyMapping( TIMESTAMP_COLUMN_NAME )
{
}


/*
 *	Override from PropertyMapping.
 */
bool TimestampMapping::visitParentColumns( ColumnVisitor & visitor )
{
	ColumnType type( MYSQL_TYPE_TIMESTAMP );
	type.defaultValue = "CURRENT_TIMESTAMP";
	type.onUpdateCmd = "CURRENT_TIMESTAMP";

	ColumnDescription description( TIMESTAMP_COLUMN_NAME_STR,
			type,
			INDEX_TYPE_NONE,
			/* shouldIgnore: */ true );

	return visitor.onVisitColumn( description );
}

// timestamp_mapping.cpp
