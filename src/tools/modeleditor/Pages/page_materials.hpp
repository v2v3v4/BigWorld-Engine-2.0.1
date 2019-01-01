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
#include "resource.h"

#include "common/property_table.hpp"
#include "controls/auto_tooltip.hpp"
#include "guitabs/guitabs_content.hpp"
#include "resmgr/string_provider.hpp"
#include "moo/forward_declarations.hpp"
#include "common/material_properties.hpp"


class UalItemInfo;

typedef std::pair < std::string , std::string > StringPair;

namespace Moo
{
	class Visual;
	typedef SmartPointer< class Visual > VisualPtr;
	class EffectMaterial;
}

// PageMaterials

class PageMaterials: public PropertyTable, public MaterialPropertiesUser, public GUITABS::Content
{
	IMPLEMENT_BASIC_CONTENT( 
		Localise(L"MODELEDITOR/PAGES/PAGE_MATERIALS/SHORT_NAME"), 
		Localise(L"MODELEDITOR/PAGES/PAGE_MATERIALS/LONG_NAME"), 
		285, 800, NULL )

	DECLARE_AUTO_TOOLTIP( PageMaterials, PropertyTable )

public:
	PageMaterials();
	virtual ~PageMaterials();

	static PageMaterials* currPage();
	
	//These are exposed to python as:
	void tintNew();			// newTint()
	void mfmLoad();			// loadMFM()
	void mfmSave();			// saveMFM()
	void tintDelete();		// deleteTint()

	bool canTintDelete();	// canDeleteTint()


// Dialog Data
	enum { IDD = IDD_MATERIALS }; 

protected:
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	
	virtual BOOL OnInitDialog();

	afx_msg LRESULT OnUpdateControls(WPARAM wParam, LPARAM lParam);
	void OnUpdateMaterialPreview();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCbnSelchangeMaterialsTechnique();

private:
	
	SmartPointer< struct PageMaterialsImpl > pImpl_;
	
	void OnGUIManagerCommand(UINT nID);
	void OnGUIManagerCommandUpdate(CCmdUI * cmdUI);
	afx_msg LRESULT OnShowTooltip(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHideTooltip(WPARAM wParam, LPARAM lParam);

	void OnSize( UINT nType, int cx, int cy );
	
	afx_msg LRESULT OnChangePropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDblClkPropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRightClkPropertyItem(WPARAM wParam, LPARAM lParam);

	bool clearCurrMaterial();
		
	void drawMaterialsList();

	void redrawList( CComboBox& list, const std::string& name, bool sel );

public:

	afx_msg void OnTvnItemexpandingMaterialsTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnSelchangedMaterialsTree(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnEnChangeMaterialsMaterial();
	afx_msg void OnEnKillfocusMaterialsMaterial();

	afx_msg void OnEnChangeMaterialsMatter();
	afx_msg void OnEnKillfocusMaterialsMatter();

	afx_msg void OnEnChangeMaterialsTint();
	afx_msg void OnEnKillfocusMaterialsTint();

	bool changeTechnique( int technique );
	void fillTechniqueList();
	
	afx_msg void OnCbnSelchangeMaterialsFxList();
	afx_msg void OnBnClickedMaterialsFxSel();

	bool changeShader( const std::wstring& fxFile );
	bool changeShaderDrop( UalItemInfo* ii );

	bool changeMFM( const std::wstring& mfmFile );
	bool changeMFMDrop( UalItemInfo* ii );

	CRect dropTest( UalItemInfo* ii );
	bool doDrop( UalItemInfo* ii );
	
	afx_msg void OnCbnSelchangeMaterialsPreviewList();
	afx_msg void OnBnClickedMaterialsPreview();

	bool previewMode();
	Moo::VisualPtr previewObject();

	void restoreView();

	Moo::EffectMaterialPtr currMaterial();

	const std::wstring & materialName() const;
	const std::wstring & matterName() const;
	const std::wstring & tintName() const;

	// MaterialPropertiesUser interface
	void proxySetCallback();

	std::string textureFeed( const std::string& descName ) const;

	std::string defaultTextureDir() const;

	std::string exposedToScriptName( const std::string& descName ) const;	

	StringProxyPtr textureProxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnTex getFn, MaterialPropertiesUser::SetFnTex setFn, const std::string& uiName, const std::string& descName ) const;

	BoolProxyPtr boolProxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnBool getFn, MaterialPropertiesUser::SetFnBool setFn, const std::string& uiName, const std::string& descName ) const;
	
	IntProxyPtr enumProxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnEnum getFn, MaterialPropertiesUser::SetFnEnum setFn, const std::string& uiName, const std::string& descName ) const;

	IntProxyPtr intProxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnInt getFn, MaterialPropertiesUser::SetFnInt setFn, MaterialPropertiesUser::RangeFnInt rangeFn, const std::string& uiName,
		const std::string& descName ) const;

	FloatProxyPtr floatProxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnFloat getFn, MaterialPropertiesUser::SetFnFloat setFn, MaterialPropertiesUser::RangeFnFloat rangeFn, const std::string& uiName,
		const std::string& descName ) const;

	Vector4ProxyPtr vector4Proxy( BaseMaterialProxyPtr proxy,
		MaterialPropertiesUser::GetFnVec4 getFn, MaterialPropertiesUser::SetFnVec4 setFn, const std::string& uiName, const std::string& descName ) const;

};

IMPLEMENT_BASIC_CONTENT_FACTORY( PageMaterials )
