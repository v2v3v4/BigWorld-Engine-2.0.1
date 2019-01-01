/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_FUNCTOR_HPP__
#define GUI_FUNCTOR_HPP__

#include "gui_forward.hpp"
#include "resmgr/datasection.hpp"
#include <map>
#include <string>


BEGIN_GUI_NAMESPACE

class Functor
{
public:
	virtual ~Functor();
	virtual const std::string& name() const = 0;
	virtual bool text( const std::string& textor, ItemPtr item, std::string& result ) = 0;
	virtual bool update( const std::string& updater, ItemPtr item, unsigned int& result ) = 0;
	virtual DataSectionPtr import( const std::string& importer, ItemPtr item ) = 0;
	virtual bool act( const std::string& action, ItemPtr item, bool& result ) = 0;
};

class FunctorManager
{
	std::map< std::string, Functor* >& functors();
public:
	FunctorManager();
	~FunctorManager();
	std::string text( const std::string& textor, ItemPtr item );
	unsigned int update( const std::string& updater, ItemPtr item );
	DataSectionPtr import( const std::string& importer, ItemPtr item );
	bool act( const std::string& action, ItemPtr item );

	void registerFunctor( Functor* functor );
	void unregisterFunctor( Functor* functor );

	static void trim( std::string& str );
};

END_GUI_NAMESPACE

#endif//GUI_FUNCTOR_HPP__
