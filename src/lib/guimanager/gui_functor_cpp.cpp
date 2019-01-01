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
#include "gui_functor_cpp.hpp"
#include "gui_item.hpp"

BEGIN_GUI_NAMESPACE

void CppFunctor::set( const std::string& name, Textor* textor )
{
	BW_GUARD;

	textors_[ name ] = textor;
}

void CppFunctor::set( const std::string& name, Updater* updater )
{
	BW_GUARD;

	updaters_[ name ] = updater;
}

void CppFunctor::set( const std::string& name, Importer* importer )
{
	BW_GUARD;

	importers_[ name ] = importer;
}

void CppFunctor::set( const std::string& name, Action* action )
{
	BW_GUARD;

	actions_[ name ] = action;
}

template<typename Cont, typename Elem>
void remove_all( Cont& cont, const Elem& elem )
{
	BW_GUARD;

	for (Cont::iterator iter = cont.begin(); iter != cont.end();)
	{
		if (iter->second == elem)
		{
			iter = cont.erase( iter );
		}
		else
		{
			++iter;
		}
	}
}

void CppFunctor::remove( Textor* textor )
{
	BW_GUARD;

	remove_all( textors_, textor );
}

void CppFunctor::remove( Updater* updater )
{
	BW_GUARD;

	remove_all( updaters_, updater );
}

void CppFunctor::remove( Importer* importer )
{
	BW_GUARD;

	remove_all( importers_, importer );
}

void CppFunctor::remove( Action* action )
{
	BW_GUARD;

	remove_all( actions_, action );
}

const std::string& CppFunctor::name() const
{
	static std::string name = "C++";
	return name;
}

bool CppFunctor::text( const std::string& textor, ItemPtr item, std::string& result )
{
	BW_GUARD;

	if( textors_.find( textor ) != textors_.end() )
	{
		result = textors_[ textor ]->text( item );
		return true;
	}
	return false;
}

bool CppFunctor::update( const std::string& updater, ItemPtr item, unsigned int& result )
{
	BW_GUARD;

	if( updaters_.find( updater ) != updaters_.end() )
	{
		result = updaters_[ updater ]->update( item );
		return true;
	}
	return false;
}

DataSectionPtr CppFunctor::import( const std::string& importer, ItemPtr item )
{
	BW_GUARD;

	if( importers_.find( importer ) != importers_.end() )
		return importers_[ importer ]->import( item );
	return NULL;
}

bool CppFunctor::act( const std::string& action, ItemPtr item, bool& result )
{
	BW_GUARD;

	if( actions_.find( action ) != actions_.end() )
	{
		result = actions_[ action ]->act( item );
		return true;
	}
	return false;
}


END_GUI_NAMESPACE
