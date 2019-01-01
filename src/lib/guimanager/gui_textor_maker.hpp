/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_TEXTOR_MAKER_HPP__
#define GUI_TEXTOR_MAKER_HPP__


#include "gui_manager.hpp"


BEGIN_GUI_NAMESPACE


template< typename T, int index = 0 >
struct TextorMaker: public Textor
{
	std::string (T::*func_)( ItemPtr item );
	TextorMaker( const std::string& name, std::string (T::*func)( ItemPtr item ) )
		: func_( func )
	{
		BW_GUARD;

		if (GUI::Manager::pInstance())
		{
			GUI::Manager::instance().cppFunctor().set( name, this );
		}
	}
	~TextorMaker()
	{
		BW_GUARD;

		if (GUI::Manager::pInstance())
		{
			GUI::Manager::instance().cppFunctor().remove( this );
		}
	}
	virtual std::string text( ItemPtr item )
	{
		BW_GUARD;

		return ( ( ( T* )this ) ->* func_ )( item );
	}
};


END_GUI_NAMESPACE


#endif //GUI_TEXTOR_MAKER_HPP__
