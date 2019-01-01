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
#include "cstdmf/string_utils.hpp"
#include "resmgr/string_provider.hpp"

namespace
{

class ArrayModifyGuard
{
	MetaData::MetaData& metaData_;
public:
	ArrayModifyGuard( MetaData::MetaData& metaData )
		: metaData_( metaData )
	{
		BW_GUARD;

		UndoRedo::instance().add( new MetaData::Operation( metaData_ ) );
		MetaData::Environment::instance().changed( metaData_.owner() );
		UndoRedo::instance().barrier(
			LocaliseUTF8( L"GIZMO/METADATA/MODIFY_META_DATA" ), false );
	}
};


class Comment
{
public:
	std::string username_;
	time_t time_;
	std::string content_;

	Comment( const std::string& username, time_t time,
		const std::string& content )
		: username_( username ), time_( time ),
		content_( content )
	{}
};


typedef std::vector<Comment> Comments;


class CommentProxy : public StringProxy
{
	std::string& value_;
	std::string transient_;
	MetaData::MetaData& metaData_;
public:
	CommentProxy( std::string& value, MetaData::MetaData& metaData )
		: value_( value ), transient_( value ), metaData_( metaData )
	{}
	virtual std::string EDCALL get() const
	{
		return transient_;
	}
	virtual void EDCALL set( std::string value, bool transient,
							bool addBarrier )
	{
		BW_GUARD;

		transient_ = value;

		if (!transient)
		{
			UndoRedo::instance().add( new MetaData::Operation( metaData_ ) );

			value_ = value;
			MetaData::Environment::instance().changed( metaData_.owner() );

			if (addBarrier)
			{
				UndoRedo::instance().barrier(
					LocaliseUTF8( L"GIZMO/METADATA/MODIFY_META_DATA" ), false );
			}
		}
	}
};


class CommentsProxy : public ArrayProxy
{
	typedef std::vector<GeneralProperty*> Properties;
	Properties properties_;
	Comments& comments_;
	MetaData::MetaData& metaData_;
	bool readOnly_;
public:
	CommentsProxy( Comments& comments, MetaData::MetaData& metaData, bool readOnly ) :
		comments_( comments ), metaData_( metaData ), readOnly_( readOnly )
	{}

	~CommentsProxy()
	{}

	virtual bool readOnly()	{	return readOnly_;	}

	virtual std::string asString() const
	{
		BW_GUARD;

		std::string ret;

		for (Comments::iterator iter = comments_.begin();
			iter != comments_.end(); ++iter)
		{
			if (!ret.empty())
			{
				ret += ", ";
			}

			ret += iter->username_ + ": '" + iter->content_ + "'";
		}

		return ret;
	}

	virtual void elect( GeneralProperty* parent )
	{
		BW_GUARD;

		int index = 0;

		for (Comments::iterator iter = comments_.begin();
			iter != comments_.end(); ++iter)
		{
			++index;

			std::string formattedTime;
			DateTimeUtils::format( formattedTime, iter->time_ );

			std::string name = bw_format( "%03d - %s ( %s )", index,
				iter->username_.c_str(), formattedTime.c_str() );

			if (readOnly_)
			{
				properties_.push_back( new StaticTextProperty( name,
					new CommentProxy( iter->content_, metaData_ ) ) );
			}
			else
			{
				properties_.push_back( new TextProperty( name,
					new CommentProxy( iter->content_, metaData_ ) ) );
			}
			properties_.back()->setGroup(
				parent->getGroup() + bw_utf8tow( parent->name() ) );
			properties_.back()->elect();
		}
	}

	virtual void expel( GeneralProperty* parent )
	{
		BW_GUARD;

		for (Properties::iterator iter = properties_.begin();
			iter != properties_.end(); ++iter)
		{
			(*iter)->expel();
			(*iter)->deleteSelf();
		}

		properties_.clear();
	}

	virtual void select( GeneralProperty* )
	{}

	virtual bool addItem()
	{
		BW_GUARD;

		ArrayModifyGuard arrayModifyGuard( metaData_ );
		std::string username = MetaData::Environment::instance().username();
		time_t time = MetaData::Environment::instance().time();

		comments_.insert( comments_.begin(), Comment( username, time, "" ) );

		return true;
	}

	virtual void delItem( int index )
	{
		BW_GUARD;

		if (index >= 0 && index < (int)comments_.size())
		{
			ArrayModifyGuard arrayModifyGuard( metaData_ );

			comments_.erase( comments_.begin() + index );
		}
	}

	virtual bool delItems()
	{
		BW_GUARD;

		if (!comments_.empty())
		{
			ArrayModifyGuard arrayModifyGuard( metaData_ );

			comments_.clear();
		}

		return true;
	}
};


class CommentsProperty : public MetaData::Property
{
	Comments comments_;
public:
	CommentsProperty( const MetaData::PropertyDesc& desc, MetaData::MetaData& metaData )
		: MetaData::Property( desc, metaData )
	{}
	virtual GeneralProperty* createProperty( bool readOnly, bool )
	{
		BW_GUARD;

		return new ArrayProperty( desc().description(),
			new CommentsProxy( comments_, metaData(), readOnly ) );
	}
	virtual void set( const MetaData::Any& value, bool isCreating = false )
	{}
	virtual MetaData::Any get() const
	{
		return 0;
	}
	virtual bool load( DataSectionPtr ds )
	{
		BW_GUARD;

		comments_.clear();

		if (ds)
		{
			DataSectionPtr prop = ds->openSection( desc().name() );

			if (prop)
			{
				for (int i = 0; i < prop->countChildren(); ++i)
				{
					DataSectionPtr comment = prop->openChild( i );

					if (comment->sectionName() == "comment")
					{
						std::string username = comment->readString( "username" );
						time_t time = (time_t)comment->readInt64( "time" );
						std::string content = comment->readString( "content" );

						if (!username.empty() && time != 0)
						{
							comments_.push_back( Comment( username, time, content ) );
						}
					}
				}
			}
		}

		return true;
	}
	virtual bool save( DataSectionPtr ds ) const
	{
		BW_GUARD;

		ds->deleteSections( desc().name() );

		DataSectionPtr prop;

		for (Comments::const_iterator iter = comments_.begin();
			iter != comments_.end(); ++iter)
		{
			if (!prop)
			{
				 prop = ds->newSection( desc().name() );
			}

			DataSectionPtr comment = prop->newSection( "comment" );

			comment->writeString( "username", iter->username_ );
			comment->writeInt64( "time", iter->time_ );
			comment->writeString( "content", iter->content_ );
		}

		return true;
	}
	virtual MetaData::Property* clone() const
	{
		BW_GUARD;

		return new CommentsProperty( *this );
	}
};


class CommentsPropertyType : public MetaData::PropertyType
{
public:
	CommentsPropertyType() : MetaData::PropertyType( "COMMENTS" )
	{}
	virtual MetaData::PropertyPtr create( const MetaData::PropertyDesc& desc,
		MetaData::MetaData& metaData, DataSectionPtr ds ) const
	{
		BW_GUARD;

		MetaData::PropertyPtr property = new CommentsProperty( desc, metaData );

		if (property->load( ds ))
		{
			return property;
		}

		return NULL;
	}
};


CommentsPropertyType s_CommentsPropertyType;

}

int metaDataCommentsPropertyTypeToken;
