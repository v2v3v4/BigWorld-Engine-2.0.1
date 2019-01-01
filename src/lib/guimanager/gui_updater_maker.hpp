/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_UPDATER_MAKER_HPP__
#define GUI_UPDATER_MAKER_HPP__


#include "gui_manager.hpp"


BEGIN_GUI_NAMESPACE


template< typename T, int index = 0 >
struct UpdaterMaker: public Updater
{
	unsigned int (T::*func_)( ItemPtr item );
	UpdaterMaker( std::string names, unsigned int (T::*func)( ItemPtr item ) )
		: func_( func )
	{
		BW_GUARD;

		if (GUI::Manager::pInstance())
		{
			while (!names.empty())
			{
				std::string::size_type delim = names.find_first_of( GUI_DELIMITER );
				std::string name = names.substr( 0, delim );

				GUI::Manager::instance().cppFunctor().set( name, this );

				if (delim == names.npos)
				{
					break;
				}

				names = names.substr( delim + 1 );
			}
		}
	}
	~UpdaterMaker()
	{
		BW_GUARD;

		if (GUI::Manager::pInstance())
		{
			GUI::Manager::instance().cppFunctor().remove( this );
		}
	}
	virtual unsigned int update( ItemPtr item )
	{
		BW_GUARD;

		return ( ( ( T* )this ) ->* func_ )( item );
	}
};


END_GUI_NAMESPACE


#endif //GUI_UPDATER_MAKER_HPP__
