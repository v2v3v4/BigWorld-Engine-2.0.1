/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FLASH_TEXT_GUI_COMPONENT_HPP
#define FLASH_TEXT_GUI_COMPONENT_HPP
#if SCALEFORM_SUPPORT

#include "cstdmf/stdmf.hpp"
#include "ashes/simple_gui_component.hpp"
#include <GFxDrawText.h>

namespace Scaleform
{

/*~ class GUI.FlashTextGUIComponent
 *
 *	The FlashTextGUIComponent is a GUI component used to display text on the screen.
 *	It inherits from SimpleGUIComponent.
 *
 *	It uses the Scaleform DrawTextAPI to display the text.
 *	It can display text in various fonts, and can have its text assigned 
 *	dynamically.
 *
 *	A new FlashTextGUIComponent is created using the GUI.FlashText function.
 *
 *	For example:
 *	@{
 *	tx = GUI.FlashText( "hello there" )
 *	GUI.addRoot( tx )
 *	tx.text = "goodbye"
 *	@}
 *	This example creates and displays a FlashTextGUIComponent with the text
 *	"hello there", and then changes it to say "goodbye" immediately
 *	afterwards.
 */
/**
 *	This class is a scriptable GUI component that displays a line of text
 *	using the Scaleform DrawText API
 */
class FlashTextGUIComponent : public SimpleGUIComponent
{
	Py_Header( FlashTextGUIComponent, SimpleGUIComponent )

public:
	FlashTextGUIComponent( const std::string& font = "", PyTypePlus * pType = &s_type_ );
	~FlashTextGUIComponent();

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::wstring, label, text )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, font, font )
	PY_RW_ATTRIBUTE_DECLARE( explicitSize_, explicitSize );
	PY_METHOD_DECLARE( py_reset )

	void				label( const std::wstring& l );
	const std::wstring&	label( void );

	void				font( const std::string& fontName );
	const std::string	font() const;

	void				update( float dTime, float relParentWidth, float relParentHeight );
	void				drawSelf( bool reallyDraw, bool overlay );

	void				size( const Vector2 & size );
	void				width( float w );
	void				height( float h );

	float				fontSize() const	{ return textParams_.FontSize; }
	void				fontSize( float s )	{ textParams_.FontSize = s; dirty_ = true; }
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, fontSize, fontSize );

	bool				underline() const	{ return textParams_.Underline; }
	void				underline( bool u ) { textParams_.Underline = u; dirty_ = true; }
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, underline, underline );

	bool				wordWrap() const	{ return textParams_.WordWrap; }
	void				wordWrap( bool w ) { textParams_.WordWrap = w; dirty_ = true; }
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, wordWrap, wordWrap );

	bool				multiline() const	{ return textParams_.Multiline; }
	void				multiline( bool m ) { textParams_.Multiline = m; dirty_ = true; }
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, multiline, multiline );

	uint				stringWidth( const std::wstring& theString );
	PY_AUTO_METHOD_DECLARE( RETDATA, stringWidth, ARG( std::wstring, END ) );

	PyObject*			stringDimensions( const std::wstring& theString );
	PY_AUTO_METHOD_DECLARE( RETOWN, stringDimensions, ARG( std::wstring, END ) );

protected:
	FlashTextGUIComponent(const FlashTextGUIComponent&);
	FlashTextGUIComponent& operator=(const FlashTextGUIComponent&);

	std::wstring		label_;
	bool				dirty_;
	bool				explicitSize_;
	GPtr<GFxDrawText>	gfxText_;
	GFxDrawTextManager::TextParams textParams_;
	Vector2				corners_[4];

	GRectF				stringRect_;

	virtual bool		load( DataSectionPtr pSect, const std::string& ownerName, LoadBindings & bindings );
	virtual void		save( DataSectionPtr pSect, SaveBindings & bindings );
	virtual void		reset();
private:
	void	recalculate();
	void	textureName( const std::string& name )	{};	//hide textureName, this is invalid for text components.

	COMPONENT_FACTORY_DECLARE( FlashTextGUIComponent( "Arial" ) )
};

};	//namespace Scaleform

#endif // #if SCALEFORM_SUPPORT
#endif // FLASH_TEXT_GUI_COMPONENT_HPP
