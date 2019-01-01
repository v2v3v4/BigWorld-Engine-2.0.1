/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_MANAGER_HPP__
#define GUI_MANAGER_HPP__

#include <string>
#include <vector>
#include <map>
#include <set>
#include "cstdmf/init_singleton.hpp"
#include "cstdmf/guard.hpp"
#include "resmgr/datasection.hpp"
#include "gui_forward.hpp"
#include "gui_item.hpp"
#include "gui_functor.hpp"
#include "gui_functor_cpp.hpp"
#include "gui_functor_option.hpp"
#include "gui_functor_python.hpp"
#include "gui_functor_datasection.hpp"
#include "gui_bitmap.hpp"


BEGIN_GUI_NAMESPACE


class Subscriber : public SafeReferenceCount
{
	std::string root_;
	ItemPtr rootItem_;
public:
	Subscriber( const std::string& root );
	virtual ~Subscriber(){}
	const std::string& root() const;
	ItemPtr rootItem();
	void rootItem( ItemPtr item );
	virtual void changed( ItemPtr item ) = 0;
};

typedef SmartPointer<Subscriber> SubscriberPtr;


class Manager : public InitSingleton<Manager>, public Item
{
	std::map<std::string,std::set<SubscriberPtr> > subscribers_;
public:
	Manager();
	virtual ~Manager();

	// Methods from InitSingleton
	/*virtual*/ bool doInit();
	/*virtual*/ bool doFini();

	using Item::add;
	void add( SubscriberPtr subscriber );
	void remove( SubscriberPtr subscriber );
	virtual ItemPtr operator()( const std::string& path );

	void act( unsigned short commandID );
	void update( unsigned short commandID );
	void update();
	void changed( ItemPtr item );

	static bool resolveFileName( const std::string& fileName );

	FunctorManager& functors() { return functorManager_; }

	CppFunctor& cppFunctor() { return cppFunctor_; }
	OptionFunctor& optionFunctor() { return optionFunctor_; }
	PythonFunctor& pythonFunctor() { return pythonFunctor_; }
	DataSectionFunctor& dataSectionFunctor() { return dataSectionFunctor_; }

	BitmapManager& bitmaps() { return bitmapManager_; }

private:
	Manager( const DataSectionPtr section );

	FunctorManager functorManager_;

	CppFunctor cppFunctor_;
	OptionFunctor optionFunctor_;
	PythonFunctor pythonFunctor_;
	DataSectionFunctor dataSectionFunctor_;

	BitmapManager bitmapManager_;
};

END_GUI_NAMESPACE

#endif//GUI_MANAGER_HPP__
