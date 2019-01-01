/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TEXT_GUI_COMPONENT_HPP
#define TEXT_GUI_COMPONENT_HPP

#include "simple_gui_component.hpp"
#include "romp/custom_mesh.hpp"
#include "moo/vertex_formats.hpp"
#include "romp/font.hpp"
#include "romp/font_metrics.hpp"
#include "cstdmf/stdmf.hpp"

/*~ class GUI.TextGUIComponent
 *	@components{ client, tools }
 *
 *	The TextGUIComponent is a GUI component used to display text on the screen.
 *	It inherits from SimpleGUIComponent.
 *
 *	It can display text in various fonts, and can have its text assigned 
 *	dynamically.
 *
 *	A new TextGUIComponent is created using the GUI.Text function.
 *
 *	For example:
 *	@{
 *	tx = GUI.Text( "hello there" )
 *	GUI.addRoot( tx )
 *	tx.text = "goodbye"
 *	@}
 *	This example creates and displays a TextGUIComponent with the text
 *	"hello there", and then changes it to say "goodbye" immediately
 *	afterwards.
 *
 *	Note that setting the SimpleGUIComponent.texture and 
 *	SimpleGUIComponent.textureName attributes is not supported on
 *	TextGUIComponent.
 *
 */
/**
 *	This class is a scriptable GUI component that displays a line of text
 */
class TextGUIComponent : public SimpleGUIComponent
{
	Py_Header( TextGUIComponent, SimpleGUIComponent )

public:
	TextGUIComponent( CachedFontPtr font = NULL, PyTypePlus * pType = &s_type_ );
	~TextGUIComponent();

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::wstring, label, text )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, font, font )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, multiline, multiline )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, colourFormatting, colourFormatting )
	PY_RW_ATTRIBUTE_DECLARE( explicitSize_, explicitSize );
	PY_METHOD_DECLARE( py_reset )

	void				label( const std::wstring& l );
	const std::wstring&	label( void );

	void				multiline( bool b );
	bool				multiline();

	void				colourFormatting( bool b );
	bool				colourFormatting();

	///this method only to be used for debugging code from C++ only.
	///(takes an ANSI only string)
	void				slimLabel( const std::string& l );

	virtual void		textureName( const std::string& name );

	void				font( const std::string& fontName );
	const std::string	font() const;

	void				update( float dTime, float relParentWidth, float relParentHeight );

	void				size( const Vector2 & size );
	void				width( float w );
	void				height( float h );

	uint				stringWidth( const std::wstring& theString );
	PY_AUTO_METHOD_DECLARE( RETDATA, stringWidth, ARG( std::wstring, END ) );

	PyObject*			stringDimensions( const std::wstring& theString );
	PY_AUTO_METHOD_DECLARE( RETOWN, stringDimensions, ARG( std::wstring, END ) );

protected:
	TextGUIComponent(const TextGUIComponent&);
	TextGUIComponent& operator=(const TextGUIComponent&);

	void				recalculate();
	void				copyAndMove( float relParentWidth, float relParentHeight );
	void				calculateMeshSize();
	void				drawSelf( bool reallyDraw, bool overlay );
	bool				buildMaterial();

	CachedFontPtr		font_;
	CustomMesh<GUIVertex>* mesh_;
	Vector3				meshSize_;
	std::wstring		label_;
	bool				dirty_;
	bool				explicitSize_;
	bool				multiline_;
	bool				colourFormatting_;
	Vector3				drawOffset_;
	uint32				lastUsedResolution_;
	D3DXHANDLE			technique_;

	virtual bool		load( DataSectionPtr pSect, const std::string& ownerName, LoadBindings & bindings );
	virtual void		save( DataSectionPtr pSect, SaveBindings & bindings );

	virtual void		reset();

	COMPONENT_FACTORY_DECLARE( TextGUIComponent() )
};

#ifdef CODE_INLINE
#include "text_gui_component.ipp"
#endif


#endif // TEXT_GUI_COMPONENT_HPP
