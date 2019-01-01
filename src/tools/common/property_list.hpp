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

#include "resmgr/string_provider.hpp"
#include "controls/edit_numeric.hpp"
#include "controls/slider.hpp"
#include "cstdmf/bw_functor.hpp"
#include "gizmo/general_properties.hpp"
#include <stack>

class PropertyList;

// PropertyItem
class PropertyItem
{
public:
	PropertyItem(const CString& name);
	virtual ~PropertyItem() {}

	virtual void create(PropertyList* parent) = 0;
	virtual void select(CRect rect, bool showDropDown = true) = 0;
	virtual void deselect() = 0;
	virtual bool hasSameValue( PropertyItem* item ) { return value() == item->value(); }

	virtual CString name();
	const std::wstring fullName();
	void multipleValue( bool flag ) { multipleValue_ = flag; }
	bool multipleValue() { return multipleValue_; }
	CString value() { return stringValue_; }
	CString displayValue();

	void setSelectable(bool option) { selectable_ = option; }
	bool getSelectable() { return selectable_; }

	void setChangeBuddy(void* buddy) { changeBuddy_ = buddy; }
	void * getChangeBuddy() { return changeBuddy_; }

	virtual controls::EditNumeric* ownEdit() { return NULL; }
		
	virtual void comboChange() {}
	virtual void onBrowse() {}
	virtual void sliderChange( int value, bool transient ) {}
	virtual void editChange() {}
	virtual void onDefault() {}
	virtual void onKeyDown( UINT key ) {}
	virtual void onCustom( UINT nID ) {}
	virtual std::wstring menuOptions() { return L""; }
	virtual std::wstring textureFeed() { return L""; }

	enum ItemType
	{
		Type_Unknown,
		Type_Group,
		Type_Colour,
		Type_ColourScalar,
		Type_Vector,
		Type_Label,
		Type_Label_Highlight,
		Type_String,
		Type_String_ReadOnly,
		Type_ID
	};
	virtual ItemType getType() { return Type_Unknown; }

	bool isGroupType();
	
	virtual const std::wstring& descName() { return descName_; }
	virtual void descName( const std::wstring& desc) { descName_ = desc; }

	virtual std::wstring UIDescL() { return Localise(uiDesc_.c_str()); }
	virtual void UIDesc( const std::wstring& desc) { uiDesc_ = desc; }

	virtual const std::wstring& exposedToScriptName() { return exposedToScriptName_; }
	virtual void exposedToScriptName( const std::wstring& name) { exposedToScriptName_ = name; }

	virtual void canExposeToScript( bool canExposeToScript ) { canExposeToScript_ = canExposeToScript; }
	virtual bool canExposeToScript() { return canExposeToScript_; }

	virtual std::wstring UIDescExtra();

	void setGroup( const std::wstring& group );
	std::wstring getGroup() { return group_; }
	void setGroupDepth( int depth ) { groupDepth_ = depth; }
	int getGroupDepth() { return groupDepth_; }

	void arrayData( int arrayIndex, BWBaseFunctor1<int>* arrayCallback, bool arrayReadOnly )
	{
		arrayIndex_ = arrayIndex;
		arrayCallback_ = arrayCallback;
		arrayReadOnly_ = arrayReadOnly;
	}
	int arrayIndex() {	return arrayIndex_;	}
	BWBaseFunctor1<int>* arrayCallback() {	return arrayCallback_.getObject();	}
	bool arrayReadOnly() const	{	return arrayReadOnly_;	}

protected:
	CString name_;
	CString stringValue_;
	bool multipleValue_;

	std::wstring descName_;
	std::wstring uiDesc_;
	std::wstring exposedToScriptName_;

	bool canExposeToScript_;

	bool selectable_;

	PropertyList* parent_;
	void* changeBuddy_;

	std::wstring group_;
	int groupDepth_;

	int arrayIndex_;
	SmartPointer< BWBaseFunctor1<int> > arrayCallback_;
	bool arrayReadOnly_;
};

typedef std::vector< PropertyItem * > PropertyItemVector;


/**
 *	Small class that detects when a CEdit has been touched (a key was
 *	pressed on it).
 */
class ChangeDetectEdit : public CEdit
{
public:
	ChangeDetectEdit() : touched_( false )
	{
	}

	void touched( bool val )	{ touched_ = val; }
	bool touched() const		{ return touched_; }

protected:
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
	{
		touched_ = true;
		CEdit::OnChar( nChar, nRepCnt, nFlags );
	}
	DECLARE_MESSAGE_MAP()

private:
	bool touched_;
};


class GroupPropertyItem : public PropertyItem
{
public:
	GroupPropertyItem(const CString& name, int depth);
	virtual ~GroupPropertyItem();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	virtual ItemType getType() { return Type_Group; }

	void addChild( PropertyItem * item );

	PropertyItemVector & getChildren() { return children_; }

	void setExpanded( bool option ) { expanded_ = option; }
	bool getExpanded() { return expanded_; }

	void setGroupDepth( int depth ) { groupDepth_ = depth; }

protected:
	PropertyItemVector children_;
	bool expanded_;
};

class ColourPropertyItem : public GroupPropertyItem
{
public:
	ColourPropertyItem(const CString& name, const CString& init, int depth, bool colour = true);
	virtual ~ColourPropertyItem();

	virtual CString name();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	void set(const std::wstring& value);
	std::wstring get();

	virtual ItemType getType() { return colour_ ? Type_Colour : Type_Vector; }

	virtual void onBrowse();
	virtual std::wstring menuOptions();

private:
	static std::map<CWnd*, ChangeDetectEdit*> edit_;
	static std::map<CWnd*, CButton*> button_;
	bool colour_;
};


class ColourScalarPropertyItem : public GroupPropertyItem
{
public:
	ColourScalarPropertyItem(const CString& name, const CString& init, int depth);
	virtual ~ColourScalarPropertyItem();

	virtual CString name();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	void set(const std::string& value);
	std::string get();

	virtual ItemType getType() { return Type_ColourScalar; }

	virtual void onBrowse();
	virtual std::wstring menuOptions();

private:
	static std::map<CWnd*, ChangeDetectEdit*> edit_;
	static std::map<CWnd*, CButton*> button_;	
};


class LabelPropertyItem : public PropertyItem
{
public:
	LabelPropertyItem(const CString& name, bool highlight = false);
	virtual ~LabelPropertyItem();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	virtual ItemType getType();

private:
	bool highlight_;
};


class StringPropertyItem : public PropertyItem
{
public:
	StringPropertyItem(const CString& name, const CString& currentValue, bool readOnly = false);
	virtual ~StringPropertyItem();

	virtual CString name();
	
	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	void set(const std::wstring& value);
	std::wstring get();
	
	void fileFilter( const std::wstring& filter ) { fileFilter_ = filter; }
	std::wstring fileFilter() { return fileFilter_; }

	void defaultDir( const std::wstring& dir ) { defaultDir_ = dir; }
	std::wstring defaultDir() { return defaultDir_; }

	void canTextureFeed( bool val ) { canTextureFeed_ = val; }
	bool canTextureFeed() { return canTextureFeed_; }
	
	void textureFeed( const std::wstring& textureFeed )	{ textureFeed_ = textureFeed; }
	std::wstring textureFeed()	{ return textureFeed_; }

	std::wstring UIDescExtra()
	{
		if ( !canTextureFeed_ ) return L"";
		if ( textureFeed_ == L"" )
		{
			return Localise(L"COMMON/PROPERTY_LIST/ASSIGN_FEED");
		}
		else
		{
			return Localise(L"COMMON/PROPERTY_LIST/ASSIGN_FEED", textureFeed_ );
		}
			
	}

	virtual void onBrowse();
	virtual std::wstring menuOptions();

	virtual ItemType getType();
	bool isHexColor() const	{	return stringValue_.GetLength() == 7 && stringValue_[0] == '#';	}
	bool isVectColor() const { std::wstring val = stringValue_; return std::count( val.begin( ), val.end( ), ',' ) == 2; }
private:
	static std::map<CWnd*, ChangeDetectEdit*> edit_;
	static std::map<CWnd*, CButton*> button_;
	std::wstring fileFilter_;
	std::wstring defaultDir_;
	bool canTextureFeed_;
	std::wstring textureFeed_;
	bool readOnly_;
};


class IDPropertyItem : public PropertyItem
{
public:
	IDPropertyItem(const CString& name, const CString& currentValue);
	virtual ~IDPropertyItem();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	void set(const std::wstring& value);
	std::wstring get();

	virtual ItemType getType();

private:
	static std::map<CWnd*, ChangeDetectEdit*> edit_;
};


class ComboPropertyItem : public PropertyItem
{
public:
	ComboPropertyItem(const CString& name, CString currentValue,
				const std::vector<std::wstring>& possibleValues);
	ComboPropertyItem(const CString& name, int currentValueIndex,
				const std::vector<std::wstring>& possibleValues);
	virtual ~ComboPropertyItem();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	void set(const std::wstring& value);
	void set(int index);
	std::wstring get();

	virtual void comboChange();

private:
	static std::map<CWnd*, CComboBox*> comboBox_;
	std::vector<std::wstring> possibleValues_;
};


class BoolPropertyItem : public PropertyItem
{
public:
	BoolPropertyItem(const CString& name, int currentValue);
	virtual ~BoolPropertyItem();

	virtual CString name();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	void set(bool value);
	bool get();

	virtual void comboChange();

	virtual std::wstring menuOptions();

private:
	static std::map<CWnd*, CComboBox*> comboBox_;
	int value_;
};


class FloatPropertyItem : public PropertyItem
{
public:
	FloatPropertyItem(const CString& name, float currentValue);
	virtual ~FloatPropertyItem();

	virtual CString name();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	void setDigits( int digits );
	void setRange( float min, float max, int digits );
	void setDefault( float def );

	void set(float value);
	float get();

	virtual void sliderChange( int value, bool transient );
	virtual void editChange();
	virtual void onDefault();

	virtual std::wstring menuOptions();

	virtual controls::EditNumeric* ownEdit();

private:
	static std::map<CWnd*, controls::EditNumeric*> editNumeric_;
	static std::map<CWnd*, controls::EditNumeric*> editNumericFormatting_;
	static std::map<CWnd*, controls::Slider*> slider_;
	static std::map<CWnd*, CButton*> button_;

	float value_;
	float min_;
	float max_;
	int digits_;
	bool ranged_;
	bool changing_;
	float def_;
	bool hasDef_;
};


class IntPropertyItem : public PropertyItem
{
public:
	IntPropertyItem(const CString& name, int currentValue);
	virtual ~IntPropertyItem();

	virtual CString name();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	void setRange( int min, int max );
	void set(int value);
	int get();

	virtual void sliderChange( int value, bool transient );
	virtual void editChange();

	virtual std::wstring menuOptions();

	virtual controls::EditNumeric* ownEdit();

private:
	static std::map<CWnd*, controls::EditNumeric*> editNumeric_;
	static std::map<CWnd*, controls::EditNumeric*> editNumericFormatting_;
	static std::map<CWnd*, controls::Slider*> slider_;
	int value_;
	int min_;
	int max_;
	bool ranged_;
	bool changing_;
};


class UIntPropertyItem : public PropertyItem
{
public:
	UIntPropertyItem(const CString& name, uint32 currentValue);
	virtual ~UIntPropertyItem();

	virtual CString name();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	void setRange( uint32 min, uint32 max );
	void set(uint32 value);
	uint32 get();

	virtual void sliderChange( int value, bool transient );
	virtual void editChange();

	virtual std::wstring menuOptions();

	virtual controls::EditNumeric* ownEdit();

private:
	static std::map<CWnd*, controls::EditNumeric*> editNumeric_;
	static std::map<CWnd*, controls::EditNumeric*> editNumericFormatting_;
	static std::map<CWnd*, controls::Slider*> slider_;
	uint32 value_;
	uint32 min_;
	uint32 max_;
	bool ranged_;
	bool changing_;
};


class ArrayPropertyItem : public GroupPropertyItem
{
public:
	ArrayPropertyItem(const CString& name, const CString& group, const CString& str, ArrayProxyPtr proxy);
	virtual ~ArrayPropertyItem();

	virtual void create(PropertyList* parent);
	virtual void select(CRect rect, bool showDropDown = true);
	virtual void deselect();

	virtual void onCustom( UINT nID );

	virtual void clear();

private:
	ArrayProxyPtr proxy_;
	static std::map<CWnd*, CButton*>				addButton_;
	static std::map<CWnd*, CButton*>				delButton_;
};

class BasePropertyTable;

// PropertyList window

class PropertyList : public CListBox
{
public:
	static void mainFrame( CFrameWnd* mainFrame ) { mainFrame_ = mainFrame; }

	PropertyList( BasePropertyTable* propertyTable );
	virtual ~PropertyList();

	void enable( bool enable );

	void isSorted( bool sorted );

	void setArrayProperty( PropertyItem* item );

	int AddPropItem( PropertyItem* item, int forceIndex = -1 );

	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	void clear();

	bool changeSelectItem( int delta );
	bool selectItem( int itemIndex, bool changeFocus = true );
	void deselectCurrentItem();
	void needsDeselectCurrentItem() { needsDeselect_ = true; }
	static void deselectAllItems();
	static void update();
	void selectPrevItem();
	void selectNextItem();

	void setDividerPos( int x );

	PropertyItem * getHighlightedItem();

	void collapseGroup(GroupPropertyItem* gItem, int index, bool reselect = true);
	void expandGroup(GroupPropertyItem* gItem, int index);

	void startArray( BWBaseFunctor1<int>* callback, bool readOnly );
	void endArray();

	CRect dropTest( CPoint point, const std::wstring& fileName );
	bool doDrop( CPoint point, const std::wstring& fileName );

	static WCHAR s_tooltipBuffer_[512];
	
	int OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
	BOOL OnToolTipText( UINT id, NMHDR* pTTTStruct, LRESULT* pResult );

	BasePropertyTable* propertyTable() const	{	return propertyTable_;	}

	PropertyItem * selectedItem() const { return selectedItem_; }
	void selectedItem( PropertyItem * newSelectedItem ) { selectedItem_ = newSelectedItem; }

protected:
	virtual void PreSubclassWindow();

	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnPaint();
	afx_msg void OnSelchange();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnChangePropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg void OnComboChange();
	afx_msg void OnBrowse();
	afx_msg void OnDefault();
	afx_msg void OnCustom(UINT nID);
	afx_msg void OnArrayDelete();
	afx_msg void OnEditChange( ); 
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point );
	afx_msg void OnRButtonUp( UINT, CPoint );
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );

	DECLARE_MESSAGE_MAP()

private:
	static CFrameWnd* mainFrame_;

	typedef std::set< PropertyList * > PropertyLists;
	static PropertyLists s_propertyLists_;

	void DrawDivider(int xpos);
	void selChange( bool showDropDown, bool changeFocus = true );
	void Select(int selected);

	CToolTipCtrl toolTip_;
	CString toolTipMsg_;

	bool enabled_;

	bool isSorted_;
	
	int selected_;
	PropertyItem * selectedItem_;
	bool needsDeselect_;

	int dividerPos_;
	int dividerTop_;
	int dividerBottom_;
	int dividerLastX_;
	bool dividerMove_;
	HCURSOR cursorArrow_;
	HCURSOR cursorSize_;

	bool tooltipsEnabled_;

	std::vector< GroupPropertyItem * > parentGroupStack_;

	bool delayRedraw_;

	std::stack<int> arrayIndex_;
	std::stack<int> arrayPropertItemIndex_;
	std::stack<SmartPointer< BWBaseFunctor1<int> > > arrayCallback_;
	std::stack<bool> arrayReadOnly_;
	static CButton s_arrayDeleteButton_;

	void establishGroup( PropertyItem* item );
	void makeSubGroup( const std::wstring & subGroup, PropertyItem* item );
	void addGroupToStack( const std::wstring & label, PropertyItem* item = NULL );

	int findSortedIndex( PropertyItem * item ) const;

	BasePropertyTable* propertyTable_;
};
