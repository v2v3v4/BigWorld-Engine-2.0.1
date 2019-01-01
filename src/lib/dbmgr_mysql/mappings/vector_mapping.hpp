/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_VECTOR_MAPPING_HPP
#define MYSQL_VECTOR_MAPPING_HPP

#include "property_mapping.hpp"

#include "../namer.hpp"
#include "../result_set.hpp"
#include "../table.hpp"
#include "../wrapper.hpp"

#include "resmgr/datasection.hpp"

template <class Vec, int DIM>
class VectorMapping : public PropertyMapping
{
public:
	VectorMapping( const Namer & namer, const std::string & propName,
				DataSectionPtr pDefaultValue ) :
		PropertyMapping( propName ),
		colNameTemplate_( namer.buildColumnName( "vm_%i", propName ) ),
		defaultValue_()
	{
		if (pDefaultValue)
			defaultValue_ = pDefaultValue->as<Vec>();
	}

	virtual void fromStreamToDatabase( StreamToQueryHelper & helper,
			BinaryIStream & strm,
			QueryRunner & queryRunner ) const
	{
		Vec v;
		strm >> v;

		for (int i = 0; i < DIM; ++i)
		{
			queryRunner.pushArg( v[i] );
		}
	}

	virtual void fromDatabaseToStream( ResultToStreamHelper & helper,
				ResultStream & results,
				BinaryOStream & strm ) const
	{
		Vec v;

		for (int i = 0; i < DIM; ++i)
		{
			results >> v[i];
		}

		strm << v;
	}

	virtual void defaultToStream( BinaryOStream & strm ) const
	{
		strm << defaultValue_;
	}

	virtual bool visitParentColumns( ColumnVisitor & visitor )
	{
		char buffer[ 512 ];

		for (int i=0; i < DIM; i++)
		{
			bw_snprintf( buffer, sizeof(buffer), colNameTemplate_.c_str(), i );

			std::string colName( buffer );
			ColumnType colType( MYSQL_TYPE_FLOAT , false, 0,
					StringConv::toStr( defaultValue_[i] ) );
			ColumnDescription column( colName, colType );

			if (!visitor.onVisitColumn( column ))
			{
				return false;
			}
		}

		return true;
	}

private:
	std::string colNameTemplate_;
	Vec defaultValue_;
};

#endif // MYSQL_VECTOR_MAPPING_HPP
