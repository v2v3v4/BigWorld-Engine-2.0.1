/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_FUNCTOR_PYTHON_HPP__
#define GUI_FUNCTOR_PYTHON_HPP__

#include <map>
#include <string>
#include "gui_functor.hpp"
#include "pyscript/script.hpp"

BEGIN_GUI_NAMESPACE

class PythonFunctor : public Functor
{
	std::string defaultModule_;
	std::map<std::string, PyObject*> modules_;

	PyObject* call( std::string function, ItemPtr item );

public:
	PythonFunctor() : defaultModule_( "UIExt" ) {}

	virtual const std::string& name() const;
	virtual bool text( const std::string& textor, ItemPtr item, std::string& result );
	virtual bool update( const std::string& updater, ItemPtr item, unsigned int& result );
	virtual DataSectionPtr import( const std::string& importer, ItemPtr item );
	virtual bool act( const std::string& action, ItemPtr item, bool& result );
	virtual void defaultModule( const std::string& module );
};

END_GUI_NAMESPACE

#endif//GUI_FUNCTOR_PYTHON_HPP__
