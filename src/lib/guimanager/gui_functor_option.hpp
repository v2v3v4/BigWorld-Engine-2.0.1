/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_FUNCTOR_OPTION_HPP__
#define GUI_FUNCTOR_OPTION_HPP__

#include "gui_functor.hpp"

BEGIN_GUI_NAMESPACE

class OptionMap
{
public:
	virtual ~OptionMap(){}
	virtual std::string get( const std::string& key ) const = 0;
	virtual bool exist( const std::string& key ) const = 0;
	virtual void set( const std::string& key, const std::string& value ) = 0;
};

class OptionFunctor : public Functor
{
	OptionMap* option_;
public:
	OptionFunctor();
	void setOption( OptionMap* option );
	virtual const std::string& name() const;
	virtual bool text( const std::string& textor, ItemPtr item, std::string& result );
	virtual bool update( const std::string& updater, ItemPtr item, unsigned int& result );
	virtual DataSectionPtr import( const std::string& importer, ItemPtr item );
	virtual bool act( const std::string& action, ItemPtr item, bool& result );
};

END_GUI_NAMESPACE

#endif//GUI_FUNCTOR_OPTION_HPP__
