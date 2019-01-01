/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "table.hpp"


namespace
{

class HasSubTablesVisitor : public TableVisitor
{
public:
	virtual bool onVisitTable( TableProvider& table )
	{
		return false;
	}
};

} // anonymous namespace


bool TableProvider::hasSubTables()
{
	HasSubTablesVisitor visitor;

	return !this->visitSubTablesWith( visitor );
}


namespace
{

/**
 *	This helper function visits the table and all it's sub-tables using the
 * 	provided visitor i.e. visitor.onVisitTable() will be called for
 * 	this table and all it's sub-tables.
 */
class RecursiveVisitor : public TableVisitor
{
public:
	RecursiveVisitor( TableVisitor & origVisitor ) :
		origVisitor_( origVisitor )
	{ }

	virtual bool onVisitTable( TableProvider& table )
	{
		return origVisitor_.onVisitTable( table ) &&
				table.visitSubTablesWith( *this );
	}

private:
	TableVisitor & origVisitor_;
};

} // anonymous namespace


/**
 *	This method visits this table and all sub-tables recursively.
 */
bool TableProvider::visitSubTablesRecursively( TableVisitor & visitor )
{
	RecursiveVisitor proxyVisitor( visitor );
	return proxyVisitor.onVisitTable( *this );
}



// table.cpp
