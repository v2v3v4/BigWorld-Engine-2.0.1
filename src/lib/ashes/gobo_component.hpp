/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GOBO_COMPONENT_HPP
#define GOBO_COMPONENT_HPP

#pragma warning( disable:4786 )

#include "ashes/simple_gui_component.hpp"
#include "moo/effect_constant_value.hpp"
#include "romp/py_render_target.hpp"

/*~ class GUI.GoboComponent
 *	@components{ client, tools }
 *
 *	This GoboComponent acts like other gui components, but blends the gui
 *	component texture with another specified PyTextureProvider, using
 *	the alpha channel of the component's texture.
 *
 *	If you associate a render target with the gobo component, you can also
 *	freeze and unfreeze the secondary PyTextureProvider.
 *
 *	A recommended use of the gobo component is to create a post-processing
 *	effect, and use the output of that effect as the second texture stage
 *	in the gobo component.
 *
 *	This is for example perfect for binoculars and sniper scopes.
 *
 *	For example, in the following code:
 *
 *	@{
 *	comp = GUI.Gobo( "gui/maps/gobo_binoculars.dds" )
 *	comp.materialFX="SOLID"
 *	comp.secondTexture=myBlurredRenderTarget.texture
 *	GUI.addRoot( comp )
 *	@}
 *
 *	This example will display a binocular gobo, and where the alpha channel is
 *	relatively opaque in the binocular texture map, a blurred version of the scene
 *	is drawn. 
 */
/**
 *	This class is a GUI component that blends between the given texture and a
 *	secondary texture provider.
 */
class GoboComponent : public SimpleGUIComponent
{
	Py_Header( GoboComponent, SimpleGUIComponent )

public:
	GoboComponent( const std::string& textureName,
		PyTypePlus * pType = &s_type_ );
	virtual ~GoboComponent();

	//-------------------------------------------------
	//Simple GUI component methods
	//-------------------------------------------------
	virtual void		draw( bool reallyDraw, bool overlay = true );
	void				freeze( void );
	void				unfreeze( void );

	//-------------------------------------------------
	//Python Interface
	//-------------------------------------------------
	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	PyObject *			pyGet_freeze();
	void				freeze( PyRenderTargetPtr pRT );

	PY_RW_ATTRIBUTE_DECLARE( secondaryTexture_, secondaryTexture )
	PY_WRITABLE_ACCESSOR_ATTRIBUTE_SET( PyRenderTargetPtr, freeze, freeze )
	PY_FACTORY_DECLARE()

protected:

	///setup the material.
	virtual bool	buildMaterial();
	void			setConstants();

private:
	GoboComponent(const GoboComponent&);
	GoboComponent& operator=(const GoboComponent&);

	PyRenderTargetPtr		freezeRenderTarget_;
	PyTextureProviderPtr	secondaryTexture_;

	COMPONENT_FACTORY_DECLARE( GoboComponent("") )
};


#ifdef CODE_INLINE
#include "gobo_component.ipp"
#endif




#endif // GOBO_COMPONENT_HPP
