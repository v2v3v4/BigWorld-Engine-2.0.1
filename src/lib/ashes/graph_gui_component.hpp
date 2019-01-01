/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GRAPH_GUI_COMPONENT_HPP
#define GRAPH_GUI_COMPONENT_HPP

#include "pyscript/script_math.hpp"
#include "pyscript/stl_to_py.hpp"
#include "simple_gui_component.hpp"
#include "romp/custom_mesh.hpp"
#include "moo/vertex_formats.hpp"
#include "romp/font.hpp"

/*~ class GUI.GraphGUIComponent
 *	@components{ client, tools }
 *
 *	The GraphGUIComponent is a GUI component used to display 
 *	a line graph of a Vector4Provider on the screen.
 *
 *	A new GraphGUIComponent is created using GUI.Graph function.
 */
/**
 * TODO: to be documented.
 */
class GraphGUIComponent : public SimpleGUIComponent
{
	Py_Header( GraphGUIComponent, SimpleGUIComponent )

public:
	GraphGUIComponent( PyTypePlus * pType = &s_type_ );
	~GraphGUIComponent();

	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	int nPoints() const		{ return nPoints_; }
	void nPoints( int n );

	PY_FACTORY_DECLARE()

	PY_RW_ATTRIBUTE_DECLARE( input_, input )
	PY_RW_ATTRIBUTE_DECLARE( minY_, minY )
	PY_RW_ATTRIBUTE_DECLARE( maxY_, maxY )
	PY_RW_ATTRIBUTE_DECLARE( frequency_, frequency )
	PY_RW_ATTRIBUTE_DECLARE( lineColoursHolder_, lineColours )
	PY_RW_ATTRIBUTE_DECLARE( showGrid_, showGrid )
	PY_RW_ATTRIBUTE_DECLARE( gridSpacing_, gridSpacing )
	PY_RW_ATTRIBUTE_DECLARE( gridColour_, gridColour )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( uint32, nPoints, nPoints );

	void				update( float dTime, float relParentWidth, float relParentHeight );
	void				draw( bool reallyDraw, bool overlay = true );

	bool load( DataSectionPtr pSect, const std::string& ownerName, LoadBindings & bindings );
	void save( DataSectionPtr pSect, SaveBindings & bindings );

protected:
	GraphGUIComponent(const GraphGUIComponent&);
	GraphGUIComponent& operator=(const GraphGUIComponent&);

	Vector4ProviderPtr	input_;
	CustomMesh< Moo::VertexXYZDUV >* mesh_[4];
	Vector4*			yValues_;
	uint32				nPoints_;
	float				minY_;
	float				maxY_;
	float				frequency_;
	float				accumTime_;
	float				dTime_;
	int					head_;

	typedef std::vector<Vector4> Vector4Array;
	Vector4Array	lineColours_;
	PySTLSequenceHolder<Vector4Array>	lineColoursHolder_;

	bool				showGrid_;
	int					gridSpacing_;
	Vector4				gridColour_;

	COMPONENT_FACTORY_DECLARE( GraphGUIComponent() )
};


#endif // GRAPH_GUI_COMPONENT_HPP
