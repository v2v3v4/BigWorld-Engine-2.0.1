/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "meta_data.hpp"
#include "cstdmf/date_time_utils.hpp"


namespace
{

class DateTimeProxy : public StringProxy
{
	const std::string& name_;
	MetaData::MetaData& metaData_;
	bool useFullDateFormat_;

public:
	DateTimeProxy( const std::string& name, MetaData::MetaData& metaData, bool useFullDateFormat )
		: name_( name ), metaData_( metaData ), useFullDateFormat_( useFullDateFormat )
	{}
	virtual std::string EDCALL get() const
	{
		BW_GUARD;

		std::string formattedTime;
		DateTimeUtils::format( formattedTime, (time_t)metaData_[ name_ ]->get(), useFullDateFormat_ );
		return formattedTime;
	}
	virtual void EDCALL set( std::string, bool, bool )
	{}
};


class DateTimeProperty : public MetaData::Property
{
	time_t time_;
	std::string represent_;
public:
	DateTimeProperty( const MetaData::PropertyDesc& desc, MetaData::MetaData& metaData )
		: MetaData::Property( desc, metaData ), time_( 0 )
	{}
	void updateRepresent()
	{
		BW_GUARD;

		DateTimeUtils::format( represent_, time_ );
	}
	virtual GeneralProperty* createProperty( bool, bool useFullDateFormat )
	{
		BW_GUARD;

		return new StaticTextProperty( desc().description(),
			new DateTimeProxy( desc().name(), metaData(), useFullDateFormat ),
			true /*is date */ );
	}
	virtual void set( const MetaData::Any& value, bool isCreating = false )
	{
		BW_GUARD;

		time_ = value;
		updateRepresent();
		if (!isCreating)
		{
			MetaData::Environment::instance().changed( metaData().owner() );
		}
	}
	virtual MetaData::Any get() const
	{
		return time_;
	}
	virtual bool load( DataSectionPtr ds )
	{
		BW_GUARD;

		if (ds)
		{
			time_ = (time_t)ds->readInt64( desc().name() );
		}
		else
		{
			time_ = 0;
		}

		updateRepresent();

		return true;
	}
	virtual bool save( DataSectionPtr ds ) const
	{
		BW_GUARD;

		ds->deleteSections( desc().name() );

		if (time_ != 0)
		{
			ds->writeInt64( desc().name(), time_ );
		}

		return true;
	}
	virtual MetaData::Property* clone() const
	{
		BW_GUARD;

		return new DateTimeProperty( *this );
	}
};


class DateTimePropertyType : public MetaData::PropertyType
{
public:
	DateTimePropertyType() : MetaData::PropertyType( "DATETIME" )
	{}
	virtual MetaData::PropertyPtr create( const MetaData::PropertyDesc& desc,
		MetaData::MetaData& metaData, DataSectionPtr ds ) const
	{
		BW_GUARD;

		MetaData::PropertyPtr property = new DateTimeProperty( desc, metaData );

		if (property->load( ds ))
		{
			return property;
		}

		return NULL;
	}
};


static DateTimePropertyType s_DateTimePropertyType;

}

int metaDataDateTimePropertyTypeToken;
