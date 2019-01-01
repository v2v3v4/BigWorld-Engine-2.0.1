/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma once

#include "appmgr/scripted_module.hpp"

#include <string>

class CSliderCtrl;

class PythonAdapter
{
public:
	PythonAdapter();
	~PythonAdapter();

	bool hasScriptObject() const;

	void reloadUIAdapter();

	bool call( const std::string & fnName );
	bool callString( const std::string & fnName, const std::string & param );
	bool callString2( const std::string & fnName, const std::string & param1, 
		const std::string & param2 );

	bool ActionScriptExecute( const std::string & functionName );
	bool ActionScriptUpdate( const std::string & actionName, int & enabled, 
		int & checked );

	void onSliderAdjust( const std::string & name, int pos, int min, int max );
	void sliderUpdate( CSliderCtrl * slider, const std::string & sliderName );

	void onListItemSelect( const std::string & name, int index );
	void onListSelectUpdate( CListBox * listBox, 
		const std::string & listBoxName );

	void onListItemToggleState( const std::string & name, unsigned int index,
		bool state );

	void contextMenuGetItems( const std::wstring & type, const std::wstring & path, 
		std::map<int,std::wstring> & items );
	void contextMenuHandleResult( const std::wstring & type, 
		const std::wstring & path, int result );

protected:
	PyObject* pScriptObject_;
    bool proActive_;
};
