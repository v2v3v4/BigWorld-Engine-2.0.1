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

//TODO: Remove this...
#include "guimanager/gui_manager.hpp"

#include "common/material_properties.hpp"
#include "material_preview.hpp"
#include "me_error_macros.hpp"

typedef SmartPointer< class MatrixProxy > MatrixProxyPtr;
typedef SmartPointer< class FloatProxy > FloatProxyPtr;
typedef SmartPointer< class StringProxy > StringProxyPtr;


class MeMaterialFlagProxy: public IntProxy
{
public:
	MeMaterialFlagProxy( const std::wstring& flagName, const std::wstring & materialName, const std::wstring* matterName = NULL, const std::wstring* tintName = NULL):
	  flagName_(flagName),
	  materialName_(materialName),
	  matterName_(matterName),
	  tintName_(tintName)
	{}
	
	virtual int32 EDCALL get() const
	{
		if ((matterName_) && (*matterName_ != L"") && (tintName_) && (*tintName_ != L"Default"))
		{
			return MeApp::instance().mutant()->tintFlag( bw_wtoutf8(*matterName_), bw_wtoutf8(*tintName_), bw_wtoutf8(flagName_) );
		}
		else
		{
			return MeApp::instance().mutant()->materialFlag( bw_wtoutf8( materialName_ ), bw_wtoutf8( flagName_ ) );
		}
	}
	
	virtual void EDCALL set( int32 v, bool transient, bool addBarrier = true )
	{
		// set it
		if ((matterName_) && (*matterName_ != L"") && (tintName_) && (*tintName_ != L"Default"))
		{
			MeApp::instance().mutant()->tintFlag( bw_wtoutf8( *matterName_ ), bw_wtoutf8( *tintName_ ), bw_wtoutf8( flagName_ ), v );
		}
		else
		{
			MeApp::instance().mutant()->materialFlag( bw_wtoutf8( materialName_ ), bw_wtoutf8( flagName_ ), v );
		}
	}
private:
	std::wstring materialName_;
	std::wstring flagName_;
	const std::wstring* matterName_;
	const std::wstring* tintName_;
};


class MeMaterialTextureProxy: public StringProxy
{
public:
	MeMaterialTextureProxy( BaseMaterialProxyPtr proxy, MaterialPropertiesUser::GetFnTex getFn, MaterialPropertiesUser::SetFnTex setFn, const std::string& uiName, const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& descName ):
	  proxyHolder_( proxy ),
	  getFn_(getFn),
	  setFn_(setFn),
	  uiName_(uiName),
	  materialName_(materialName),
	  matterName_(matterName),
	  tintName_(tintName),
	  descName_(descName)
	{}
	
	virtual std::string EDCALL get() const
	{
		return (*getFn_)();
	}
	
	virtual void EDCALL set( std::string v, bool transient, bool addBarrier = true )
	{
#if MANAGED_CUBEMAPS
		// Make sure there isn't a mismatch between the supplied texture and the expected
		// texture type
		//
		// Get the managed effect from the mutant
		Moo::EffectMaterialPtr pEffectMaterial;
		if ( !(pEffectMaterial =
				MeApp::instance().mutant()->getEffectForTint( matterName_, tintName_, materialName_ )) )
			return;
		
		// Get the managed effect
		Moo::ManagedEffectPtr pManagedEffect = pEffectMaterial->pEffect();

		// Get a handle to parameter descName_
		D3DXHANDLE param = pManagedEffect->pEffect()->GetParameterByName( descName_.c_str(), 0 );

		// Get the annotation, if any
		const char* UIWidget = 0;
		D3DXHANDLE hAnnot = pManagedEffect->pEffect()->GetAnnotationByName( param, "UIWidget" );
		if (hAnnot)
			pManagedEffect->pEffect()->GetString( hAnnot, &UIWidget );
		std::string widgetType;
		if (UIWidget)
			widgetType = std::string(UIWidget);
		else
			widgetType = "";

		
		// Check the type of the new texture
		bool isCubeMap = false;
		std::string             newResID = BWResolver::dissolveFilename( v );
		Moo::BaseTexturePtr		newBaseTex = Moo::TextureManager::instance()->get( newResID ).getObject();

		// Is this texture a cube map
		if ( newBaseTex )
			isCubeMap = newBaseTex->isCubeMap();

		// Check if the types match up
		if ( newBaseTex && widgetType == "CubeMap" && !isCubeMap )
		{
			ME_WARNING_MSGW(	L"Warning - You have attempted to assign a non-cube map texture to a\n"
								L"cube map texture slot!  This is not permitted.")

			return;
		}
		if ( newBaseTex && widgetType != "CubeMap" && isCubeMap )
		{
			ME_WARNING_MSGW(	L"Warning - You have attempted to assign a cube map texture to a\n"
								L"non-cube map texture slot!  This is not permitted.")

			return;
		}
#endif
		if ( !v.empty() )
		{
			std::wstring sfilename = BWResource::resolveFilenameW(v);
			std::replace(sfilename.begin(), sfilename.end(), L'/', L'\\');

			//Create a PIDL from the filename:
			HRESULT hr = S_OK;
			ITEMIDLIST *pidl = NULL;
			DWORD flags = 0;
			hr = ::SHILCreateFromPath(sfilename.c_str(), &pidl, &flags); 

			if (SUCCEEDED(hr))
			{
			   // Convert the PIDL back to a filename (now corrected for case):
			   wchar_t buffer[MAX_PATH];
			   ::SHGetPathFromIDList(pidl, buffer);
			   ::ILFree(pidl); 

				sfilename = buffer;
			}
			
			v = BWResource::dissolveFilename(bw_wtoutf8( sfilename ));
		}

		(*setFn_)( v );

		if (transient) return;

		if ((matterName_ != "") && (tintName_ != "Default"))
		{
			MeApp::instance().mutant()->setTintProperty( matterName_, tintName_, descName_, uiName_, "Texture", v );
		}
		else
		{
			MeApp::instance().mutant()->setMaterialProperty( materialName_, descName_, uiName_, "Texture", v );
		}
		MeApp::instance().mutant()->recalcTextureMemUsage(); // This could have changed
	}

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnTex getFn_;
	MaterialPropertiesUser::SetFnTex setFn_;
	std::string uiName_;
	std::string materialName_;
	std::string matterName_;
	std::string tintName_;
	std::string descName_;
};


class MeMaterialBoolProxy: public BoolProxy
{
public:
	MeMaterialBoolProxy( BaseMaterialProxyPtr proxy, MaterialPropertiesUser::GetFnBool getFn, MaterialPropertiesUser::SetFnBool setFn, const std::string& uiName, const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& descName ):
	  proxyHolder_( proxy ),
	  getFn_(getFn),
	  setFn_(setFn),
	  uiName_(uiName),
	  materialName_(materialName),
	  matterName_(matterName),
	  tintName_(tintName),
	  descName_(descName)
	{}
	
	virtual bool EDCALL get() const
	{
		return (*getFn_)();
	}
	
	virtual void EDCALL set( bool v, bool transient, bool addBarrier = true )
	{
		(*setFn_)( v );

		if (transient) return;

		if ((matterName_ != "") && (tintName_ != "Default"))
		{
			MeApp::instance().mutant()->setTintProperty( matterName_, tintName_, descName_, uiName_, "Bool", v ? "true" : "false" );
		}
		else
		{
			MeApp::instance().mutant()->setMaterialProperty( materialName_, descName_, uiName_, "Bool", v ? "true" : "false" );
		}
	}

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnBool getFn_;
	MaterialPropertiesUser::SetFnBool setFn_;
	std::string uiName_;
	std::string materialName_;
	std::string matterName_;
	std::string tintName_;
	std::string descName_;
};


class MeMaterialIntProxy: public IntProxy
{
public:
	MeMaterialIntProxy( BaseMaterialProxyPtr proxy, MaterialPropertiesUser::GetFnInt getFn, MaterialPropertiesUser::SetFnInt setFn, MaterialPropertiesUser::RangeFnInt rangeFn, const std::string& uiName, const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& descName ):
	  proxyHolder_( proxy ),
	  getFn_(getFn),
	  setFn_(setFn),
	  rangeFn_(rangeFn),
	  uiName_(uiName),
	  materialName_(materialName),
	  matterName_(matterName),
	  tintName_(tintName),
	  descName_(descName)
	{}
	
	virtual int32 EDCALL get() const
	{
		return (*getFn_)();
	}
	
	virtual void EDCALL set( int32 v, bool transient, bool addBarrier = true )
	{
		(*setFn_)( v );

		if (transient) return;

		char buf[16];
		bw_snprintf( buf, sizeof(buf), "%d", v );
		if ((matterName_ != "") && (tintName_ != "Default"))
		{
			MeApp::instance().mutant()->setTintProperty( matterName_, tintName_, descName_, uiName_, "Int", buf );
		}
		else
		{
			MeApp::instance().mutant()->setMaterialProperty( materialName_, descName_, uiName_, "Int", buf );
		}
	}

	bool getRange( int& min, int& max ) const
	{
		int mind, maxd;
		if( rangeFn_ )
		{
			bool result = (*rangeFn_)( mind, maxd );
			min = mind;
			max = maxd;
			return result;
		}
		return false;
	}

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnInt getFn_;
	MaterialPropertiesUser::SetFnInt setFn_;
	MaterialPropertiesUser::RangeFnInt rangeFn_;
	std::string uiName_;
	std::string materialName_;
	std::string matterName_;
	std::string tintName_;
	std::string descName_;
};


class MeMaterialEnumProxy: public IntProxy
{
public:
	MeMaterialEnumProxy( BaseMaterialProxyPtr proxy, MaterialPropertiesUser::GetFnEnum getFn, MaterialPropertiesUser::SetFnEnum setFn, const std::string& uiName, const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& descName ):
	  proxyHolder_( proxy ),
	  getFn_(getFn),
	  setFn_(setFn),
	  uiName_(uiName),
	  materialName_(materialName),
	  matterName_(matterName),
	  tintName_(tintName),
	  descName_(descName)
	{}
	
	virtual int32 EDCALL get() const
	{
		return (*getFn_)();
	}
	
	virtual void EDCALL set( int32 v, bool transient, bool addBarrier = true )
	{
		(*setFn_)( v );

		if (transient) return;

		char buf[16];
		bw_snprintf( buf, sizeof(buf), "%d", v );
		if ((matterName_ != "") && (tintName_ != "Default"))
		{
			MeApp::instance().mutant()->setTintProperty( matterName_, tintName_, descName_, uiName_, "Int", buf );
		}
		else
		{
			MeApp::instance().mutant()->setMaterialProperty( materialName_, descName_, uiName_, "Int", buf );
		}
	}

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnEnum getFn_;
	MaterialPropertiesUser::SetFnEnum setFn_;
	std::string uiName_;
	std::string materialName_;
	std::string matterName_;
	std::string tintName_;
	std::string descName_;
};


class MeMaterialFloatProxy: public FloatProxy
{
public:
	MeMaterialFloatProxy( BaseMaterialProxyPtr proxy, MaterialPropertiesUser::GetFnFloat getFn, MaterialPropertiesUser::SetFnFloat setFn, MaterialPropertiesUser::RangeFnFloat rangeFn, const std::string& uiName, const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& descName ):
	  proxyHolder_( proxy ),
	  getFn_(getFn),
	  setFn_(setFn),
	  rangeFn_(rangeFn),
	  uiName_(uiName),
	  materialName_(materialName),
	  matterName_(matterName),
	  tintName_(tintName),
	  descName_(descName)
	{}
	
	virtual float EDCALL get() const
	{
		return (*getFn_)();
	}
	
	virtual void EDCALL set( float v, bool transient, bool addBarrier = true )
	{
		(*setFn_)( v );

		if (transient) return;
		
		char buf[16];
		bw_snprintf( buf, sizeof(buf), "%f", v );
		if ((matterName_ != "") && (tintName_ != "Default"))
		{
			MeApp::instance().mutant()->setTintProperty( matterName_, tintName_, descName_, uiName_, "Float", buf );
		}
		else
		{
			MeApp::instance().mutant()->setMaterialProperty( materialName_, descName_, uiName_, "Float", buf );
		}
	}

	bool getRange( float& min, float& max, int& digits ) const
	{
		float mind, maxd;
		if( rangeFn_ )
		{
			bool result =  (*rangeFn_)( mind, maxd, digits );
			min = mind;
			max = maxd;
			return result;
		}
		return false;
	}

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnFloat getFn_;
	MaterialPropertiesUser::SetFnFloat setFn_;
	MaterialPropertiesUser::RangeFnFloat rangeFn_;
	std::string uiName_;
	std::string materialName_;
	std::string matterName_;
	std::string tintName_;
	std::string descName_;
};


class MeMaterialVector4Proxy: public Vector4Proxy
{
public:
	MeMaterialVector4Proxy( BaseMaterialProxyPtr proxy, MaterialPropertiesUser::GetFnVec4 getFn, MaterialPropertiesUser::SetFnVec4 setFn, const std::string& uiName, const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& descName ):
	  proxyHolder_( proxy ),
	  getFn_(getFn),
	  setFn_(setFn),
	  uiName_(uiName),
	  materialName_(materialName),
	  matterName_(matterName),
	  tintName_(tintName),
	  descName_(descName)
	{}
	
	virtual Vector4 EDCALL get() const
	{
		return (*getFn_)();
	}
	
	virtual void EDCALL set( Vector4 v, bool transient, bool addBarrier = true )
	{
		(*setFn_)( v );

		if (transient) return;

		char buf[256];
		bw_snprintf( buf, sizeof(buf), "%f %f %f %f", v.x, v.y, v.z, v.w );
		if ((matterName_ != "") && (tintName_ != "Default"))
		{
			MeApp::instance().mutant()->setTintProperty( matterName_, tintName_, descName_, uiName_, "Vector4", buf );
		}
		else
		{
			MeApp::instance().mutant()->setMaterialProperty( materialName_, descName_, uiName_, "Vector4", buf );
		}
	}

private:
	BaseMaterialProxyPtr proxyHolder_;
	MaterialPropertiesUser::GetFnVec4 getFn_;
	MaterialPropertiesUser::SetFnVec4 setFn_;
	std::string uiName_;
	std::string materialName_;
	std::string matterName_;
	std::string tintName_;
	std::string descName_;
};
