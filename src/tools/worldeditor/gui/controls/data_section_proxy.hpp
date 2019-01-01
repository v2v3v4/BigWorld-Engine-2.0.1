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
#include "common/editor_views.hpp"
#include "gizmo/general_properties.hpp"
#include "math/colour.hpp"
#include "worldeditor/framework/world_editor_app.hpp"
#include "worldeditor/scripting/we_python_adapter.hpp"

class GeneralEditor;
typedef SmartPointer<GeneralEditor> GeneralEditorPtr;


#define DeclareDataSectionProxy( t, T )										\
class DataSection##T##Proxy : public T##Proxy								\
{																			\
public:																		\
	DataSection##T##Proxy( DataSectionPtr pSect, const std::string& tag,	\
					const std::string& callbackName, t defaultVal ):		\
		pSection_( pSect ),													\
		callback_( callbackName ),											\
		tag_( tag ),														\
		defaultVal_( defaultVal ),											\
		firstChange_( true )												\
	{																		\
	}																		\
																			\
	t EDCALL get() const													\
	{																		\
		return pSection_->read##T##( tag_, defaultVal_ );					\
	}																		\
																			\
	void EDCALL set( t f, bool transient, bool addBarrier = true )			\
	{																		\
		BW_GUARD;															\
																			\
		WEPythonAdapter* p = WorldEditorApp::instance().pythonAdapter();	\
		if (firstChange_)													\
		{																	\
			p->callString( "saveUndoState_" + callback_, tag_ );			\
			firstChange_ = false;											\
		}																	\
		pSection_->write##T##( tag_, f );									\
		p->callString( callback_, tag_ );									\
		if (!transient)														\
		{																	\
			firstChange_ = true;											\
		}																	\
	}																		\
																			\
protected:																	\
	DataSectionPtr pSection_;												\
	std::string tag_;														\
	std::string callback_;													\
	t defaultVal_;															\
	bool firstChange_;														\
};


#define DeclareClampedDataSectionProxy( t, T )								\
DeclareDataSectionProxy( t, T )												\
class ClampedDataSection##T##Proxy : public DataSection##T##Proxy 			\
{																			\
public:																		\
	ClampedDataSection##T##Proxy( DataSectionPtr pSect, const std::string& tag,	\
							const std::string& callbackName, t defaultVal,	\
							t minVal, t maxVal, int precision = 1 ):		\
		DataSection##T##Proxy( pSect, tag, callbackName, defaultVal ),		\
		minVal_( minVal ),													\
		maxVal_( maxVal ),													\
		precision_( precision )												\
	{																		\
	}																		\
																			\
	virtual bool getRange( t& min, t& max, int& digits ) const				\
	{																		\
		min = minVal_;														\
		max = maxVal_;														\
		digits = precision_;												\
		return true;														\
	}																		\
																			\
	virtual bool getRange( t& min, t& max ) const							\
	{																		\
		min = minVal_;														\
		max = maxVal_;														\
		return true;														\
	}																		\
																			\
protected:																	\
	t minVal_;																\
	t maxVal_;																\
	int precision_;															\
};


DeclareClampedDataSectionProxy( int, Int );
DeclareClampedDataSectionProxy( float, Float );
DeclareClampedDataSectionProxy( Vector2, Vector2 );
DeclareDataSectionProxy( Vector4, Vector4 );
DeclareDataSectionProxy( std::string, String );


class DataSectionColourProxy : public ColourProxy
{
public:
	DataSectionColourProxy( DataSectionPtr pSect, const std::string& tag,
		const std::string& callback, const Vector4& defaultVal ):
		pSection_( pSect ),
		callback_( callback ),
		tag_( tag ),
		defaultVal_( defaultVal ),
		firstChange_( true )
	{
	}

	Moo::Colour EDCALL get() const
	{
		BW_GUARD;

		return Colour::getUint32FromNormalised( pSection_->readVector4( tag_, defaultVal_ ) );
	}

	void EDCALL set( Moo::Colour f, bool transient )
	{
		BW_GUARD;

		WEPythonAdapter* p = WorldEditorApp::instance().pythonAdapter();
		if (firstChange_)
		{
			p->callString( "saveUndoState_" + callback_, tag_ );
			firstChange_ = false;
		}

		pSection_->writeVector4( tag_, Colour::getVector4Normalised(f) );
		p->callString( callback_, tag_ );

		if (!transient)		
		{			
			firstChange_ = true;
		}		
	}
protected:
	DataSectionPtr pSection_;
	std::string callback_;
	std::string tag_;
	Vector4 defaultVal_;
	bool firstChange_;
};


class ClampedDataSectionColourScalarProxy : public ColourScalarProxy
{
public:
	ClampedDataSectionColourScalarProxy(DataSectionPtr pSect,
			const std::string& tag,
			const std::string& callback,
			const Vector4& defaultVal,
			float minVal,
			float maxVal,
			int precis):
			pSection_( pSect ),
			callback_( callback ),
			tag_( tag ),
			defaultVal_( defaultVal ),
			minVal_( minVal ),
			maxVal_( maxVal ),
			precision_( precis ),
			firstChange_( true )
	{
	}

	Vector4 EDCALL get() const
	{
		BW_GUARD;

		return pSection_->readVector4( tag_, defaultVal_ );
	}

	void EDCALL set( Vector4 f, bool transient, bool addBarrier )
	{
		BW_GUARD;

		WEPythonAdapter* p = WorldEditorApp::instance().pythonAdapter();
		if (firstChange_)
		{
			p->callString( "saveUndoState_" + callback_, tag_ );
			firstChange_ = false;
		}
		
		pSection_->writeVector4( tag_, f );
		p->callString( callback_, tag_ );

		if (!transient)
		{
			firstChange_ = true;
		}
	}

	virtual bool getRange( float& min, float& max, int& digits ) const
	{
		min = (float)minVal_;
		max = (float)maxVal_;
		digits = precision_;
		return true;
	}
protected:
	DataSectionPtr pSection_;
	std::string callback_;
	std::string tag_;
	float minVal_;
	float maxVal_;
	int precision_;
	Vector4 defaultVal_;
	bool firstChange_;
};
