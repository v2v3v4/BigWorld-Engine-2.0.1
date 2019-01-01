/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_GRAPHICS_SETTING_HPP
#define PY_GRAPHICS_SETTING_HPP

#include "cstdmf/smartpointer.hpp"
#include "moo/graphics_settings.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"


/**
 *	This class implements a GraphicsSetting that
 *	calls into python when its option is changed.
 *	The callback takes one parameter, which is the newly selected
 *	option for this graphics setting.
 */
class PyCallbackGraphicsSetting : public Moo::GraphicsSetting
{
public:
	PyCallbackGraphicsSetting(
		const std::string & label, 
		const std::string & desc,
		int                 activeOption,
		bool                delayed,
		bool                needsRestart,
		PyObject*			callback);

	~PyCallbackGraphicsSetting()	{};

	//callback from the graphics settings system.
	void onOptionSelected(int optionIndex);

	void pCallback( PyObjectPtr pc );
	PyObjectPtr pCallback() const;

private:
	PyObjectPtr	pCallback_;
};


typedef SmartPointer<PyCallbackGraphicsSetting> PyCallbackGraphicsSettingPtr;


/*~ class BigWorld.PyGraphicsSetting
 *	@components{ client,  tools }
 *
 *	This class is a graphics setting that calls into python
 *	to allow arbitrary script code implementation.
 *
 *  Each GraphicsSetting object offers a number of options.
 *	There is always one and only one active option at any one time.
 *
 *	The callback function is called whenever the option
 *	is changed.  It takes one parameter, which is the newly selected
 *	option for this graphics setting.
 */
/**
 *	This class is a graphics setting that calls into python
 *	to allow arbitrary script code implementation.
 *
 *  Each GraphicsSetting object offers a number of options.
 *	There is always one and only one active option at any one time.
 *
 *	The callback function is called whenever the option
 *	is changed.  It takes one parameter, which is the newly selected
 *	option for this graphics setting.
 */
class PyGraphicsSetting : public PyObjectPlus
{
	Py_Header( PyGraphicsSetting, PyObjectPlus )

public:
	PyGraphicsSetting( PyCallbackGraphicsSettingPtr pSetting, PyTypePlus * pType = &s_type_ );
	~PyGraphicsSetting() {};

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	int addOption( const std::string & label, const std::string & desc, bool isSupported );
	void registerSetting();
	void onOptionSelected(int optionIndex);

	void pCallback( PyObjectPtr pc ){ pSetting_->pCallback(pc); }
	PyObjectPtr pCallback() const	{ return pSetting_->pCallback(); }

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( PyObjectPtr, pCallback, callback )
	PY_AUTO_METHOD_DECLARE( RETDATA, addOption,
		ARG(std::string,
		ARG(std::string,
		ARG( bool, END ) ) ) );
	PY_AUTO_METHOD_DECLARE( RETVOID, registerSetting, END );

	PY_FACTORY_DECLARE()

protected:
	PyCallbackGraphicsSettingPtr		pSetting_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyGraphicsSetting )

typedef SmartPointer<PyGraphicsSetting>	PyGraphicsSettingPtr;

#endif // PY_GRAPHICS_SETTING_HPP
