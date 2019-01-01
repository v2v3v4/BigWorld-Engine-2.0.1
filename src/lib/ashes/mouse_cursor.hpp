/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef MOUSE_CURSOR_HPP
#define MOUSE_CURSOR_HPP


#include "input/input_cursor.hpp"
#include "pyscript/script.hpp"
#include "moo/device_callback.hpp"
#include "moo/moo_math.hpp"
#include "moo/texture_manager.hpp"

// -----------------------------------------------------------------------------
// section: MouseCursor
// -----------------------------------------------------------------------------

/*~ class GUI.MouseCursor
 *	@components{ client, tools }
 *
 *	The MouseCursor class encapsulates all information and operations 
 *	related	to the mouse cursor in SimpleGUI. Its is used to activate 
 *	mouse events propagation and to provide access to mouse related 
 *	properties (like position, visibility and shape). It implements 
 *	the InputCursor interface.
 *
 *	Cursor shapes are defined via the mouse cursors definition file.
 *	The name of the cursor definition xml resource file can be set in 
 *	the resources.xml file, in the gui/cursorsDefinitions field. It 
 *	defaults to "gui/mouse_cursors.xml". Bellow, you can see an example 
 *	of a cursor definition file:
 *
 *	@{
 *	&lt;mouse_cursors&gt;
 *		&lt;arrow&gt;	
 *			&lt;hotSpot&gt; 8 8 &lt;/hotSpot&gt;
 *			&lt;texture&gt; gui/maps/cursor_arrow.tga &lt/texture&gt
 *		&lt;/arrow&gt
 *		...
 *	&lt;/mouse_cursors&gt
 *	@{
 *
 *	Where:
 *		arrow:		an arbitrary name identifying the cursor.
 *		hotSpot:	2D position defining this cursor's hot-spot.
 *		texture:	texture resource to be rendered as the mouse cursor.
 *
 *	From the Python scripts, you can change the mouse shape and other
 *	properties and set it as the active cursor, like in the example bellow:
 *
 *	@{
 *		import GUI
 *		import BigWorld
 *
 *		mc = GUI.mcursor()
 *		mc.shape = "arrow"
 *		mc.visible = True
 *		mc.position = (0.5, 0.5)
 *		BigWorld.setCursor( mc )
 *	@{
 *	
 */
class MouseCursor : 
	public InputCursor,
	private Moo::DeviceCallback
{
	Py_Header( MouseCursor, InputCursor )

public:

	explicit MouseCursor( PyTypePlus * pType = &s_type_ );
	~MouseCursor();
	
	Vector2 position() const;
	void position( const Vector2 & pos );
	
	const std::string & shape() const;
	void shape( const std::string & cursorName );
	
	bool visible() const;
	void visible( bool state );

	bool clipped() const;
	void clipped( bool state );

	bool automaticCursorManagement() const;
	void automaticCursorManagement( bool state );

	bool dynamic() const;
	void dynamic( bool dynamic );

	const Vector2 & hotSpot() const;
	void hotSpot( const Vector2 & pos );
	
	virtual bool handleKeyEvent( const KeyEvent & /*event*/ );
	virtual bool handleMouseEvent( const MouseEvent & /*event*/ );
	virtual bool handleAxisEvent( const AxisEvent & /*event*/ );

	void focus( bool focus );
	
	bool isDirty() const;
	void refresh();

	void tick(float dTime);

	virtual void activate();
	virtual void deactivate();
	
	bool isActive() const;

	//-------------------------------------------------
	//Python Interface
	//-------------------------------------------------
	PyObject *	pyGetAttribute( const char * attr );
	int			pySetAttribute( const char * attr, PyObject * value );
	
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector2, position, position )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, shape, shape )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, visible, visible )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, clipped, clipped )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, automaticCursorManagement, automaticCursorManagement )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, dynamic, dynamic )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector2, hotSpot, hotSpot )
	PY_RO_ATTRIBUTE_DECLARE( isActive_, active )
	
	PY_MODULE_STATIC_METHOD_DECLARE( py_mcursor )

	PyObject * pyGet_texture();
	int pySet_texture( PyObject * value );

	//-------------------------------------------------
	//Static methods
	//-------------------------------------------------
	static void updateMouseClipping();
	static void staticSetFocus( bool focus );


private:	
	/**
	* Holds mouse cursor definition data.
	*/
	struct CursorDefinition
	{
		Vector2				hotSpot;
		Moo::BaseTexturePtr	texture;
	};
	typedef std::map< std::string, CursorDefinition > CursorDefMap;
	typedef SmartPointer<class PyTextureProvider> PyTextureProviderPtr;
	typedef ComObjectWrap<DX::BaseTexture> DXBaseTexturePtr;
	
	void loadCursorsDefinitions();
	bool setShape( const CursorDefMap::const_iterator & curIt );
	bool setTexture( PyTextureProviderPtr texture, const Vector2 & hotSpot );
	bool setTexture( DX::BaseTexture * texture, const Vector2 & hotSpot );

	MouseCursor( const MouseCursor & );
	MouseCursor & operator = ( const MouseCursor & );
	
	Vector2	inactivePos_;
	bool	isActive_;
	bool	isVisible_;
	bool	isDirty_;
	bool	isDynamic_;
	bool	isClipped_;
	bool	isAutomaticCursorManagement_;

	CursorDefMap                 cursorDefs_;
	CursorDefMap::const_iterator curCursor_;
	PyTextureProviderPtr         curProvider_;
	DXBaseTexturePtr             curTexture_;
	Vector2                      curHotSpot_;

	/**
	 * Device reset
	 */	
	virtual void deleteUnmanagedObjects();
	virtual void createUnmanagedObjects();
	virtual void createManagedObjects();
};

#endif // MOUSE_CURSOR_HPP
