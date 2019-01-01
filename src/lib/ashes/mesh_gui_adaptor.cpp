/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "mesh_gui_adaptor.hpp"
#include "cstdmf/debug.hpp"
#include "moo/render_context.hpp"
#include "moo/visual_channels.hpp"

DECLARE_DEBUG_COMPONENT2( "2DComponents", 0 )

#ifndef CODE_INLINE
#include "mesh_gui_adaptor.ipp"
#endif

// -----------------------------------------------------------------------------
// Section: MeshGUIAdaptor
// -----------------------------------------------------------------------------

#undef PY_ATTR_SCOPE
#define PY_ATTR_SCOPE MeshGUIAdaptor::

PY_TYPEOBJECT( MeshGUIAdaptor )

PY_BEGIN_METHODS( MeshGUIAdaptor )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( MeshGUIAdaptor )

	/*~ attribute MeshGUIAdaptor.attachment
	 *	@components{ client, tools }
	 *
	 * @type PyAttachment	This attribute stores the attachment that is to be incorporated into the GUI.
	 */
	PY_ATTRIBUTE( attachment )

	/*~ attribute MeshGUIAdaptor.transform
	 *	@components{ client, tools }
	 *
	 * @type MatrixProvider	This atttribute stores the orientation and a 
	 *							direction for the PyAttachment.  As a 'Provider' 
	 *							object, if the data is updated anywhere, it will 
	 *							be reflected in the GUI...
	 */
	PY_ATTRIBUTE( transform )

PY_END_ATTRIBUTES()

/*~	function GUI.MeshAdaptor
 *	@components{ client, tools }
 *
 *	Creates and returns a new MeshGUIAdaptor object, which is used to display 3D objects in the GUI scene.
 *
 *	@return	The new MeshGUIAdaptor
 */
PY_FACTORY_NAMED( MeshGUIAdaptor, "MeshAdaptor", GUI )

COMPONENT_FACTORY( MeshGUIAdaptor )


/**
 *	Constructor.
 */
MeshGUIAdaptor::MeshGUIAdaptor( PyTypePlus * pType ):
	SimpleGUIComponent( "", pType ),
	attachment_( NULL ),
	transform_( NULL )
{
	BW_GUARD;
	PyMatrix* mat = new PyMatrix;
	transform_ = SmartPointer<PyMatrix>(mat, true);
	Matrix m;
	//flatten every attached mesh, and position in the middle of the z-buffer
	//these defaults fix the scaling from gui meshes exported directly from MAX
	//- but if your art pipeline is different, then you will have to set these
	//- explicity from script.
	m.setScale(1.f/32.f,1.f/24.f,0.001f);	
	m.translation( Vector3(0,0,0.1f) );
	mat->set( m );

	if ( !s_lighting_ )
	{
		//MOo::Colour takes either DWORD(argb) or floats (rgba)
		s_lighting_ = new Moo::LightContainer;
		s_lighting_->ambientColour( Moo::Colour(0xff101820) );
		/*s_lighting_->addDirectional( new Moo::DirectionalLight(
			Moo::Colour(0.75f,0.75f,0.75f,1.f), Vector3(0.5f,0.5f,-0.5f)));
		s_lighting_->addOmni( new Moo::OmniLight(
			Moo::Colour(0xff603000), Vector3(-1.f,-3.f,-3.f), 0.f, 50.f ) );
		s_lighting_->addOmni( new Moo::OmniLight(
			Moo::Colour(0xff181860), Vector3(-0.5f,2.f,5.f), 0.f, 50.f ) );*/
	}
}


/**
 *	Destructor.
 */
MeshGUIAdaptor::~MeshGUIAdaptor()
{
	BW_GUARD;
	transform_ = NULL;
	this->attachment( NULL );
}


/**
 *	This method attaches the given PyAttachment to this gui component.
 */
void MeshGUIAdaptor::attachment( SmartPointer<PyAttachment> att )
{
	BW_GUARD;
	if ( attachment_ )
	{
		attachment_->detach();
		attachment_->leaveWorld();
	}

	attachment_ = att;

	if ( att )
	{
		att->attach( this );
		att->enterWorld();
	}
}


/**
 *	Section - SimpleGUIComponent methods
 */
void MeshGUIAdaptor::update( float dTime, float relParentWidth, float relParentHeight )
{
	BW_GUARD;
	SimpleGUIComponent::update( dTime, relParentWidth, relParentHeight );
	if ( attachment_ )
		attachment_->tick( dTime );
}


/**
 *	This method implements the SimpleGUIComponent::draw interface. 
 *	If we have a mesh  attachment, we draw it as well.
 */
void MeshGUIAdaptor::draw( bool reallyDraw, bool overlay )
{
	BW_GUARD;
	if ( visible() )
	{
		Moo::rc().push();
		Moo::rc().preMultiply( runTimeTransform() );
		Moo::rc().device()->SetTransform( D3DTS_WORLD, &Moo::rc().world() );

		SimpleGUIComponent::drawSelf(reallyDraw, overlay);		
		SimpleGUIComponent::drawChildren(reallyDraw, overlay);		

		if ( !momentarilyInvisible_ && reallyDraw )	//don't check the texture_ member ( see SGC.ipp )
		{
			Matrix m( Matrix::identity );
			if ( transform_ )
				transform_->matrix( m );
			Moo::rc().preMultiply(m);
			if ( attachment_ )
			{
				SimpleGUI::instance().setConstants(runTimeColour(), pixelSnap());
				Moo::LightContainerPtr pLC = Moo::rc().lightContainer();
				s_lighting_->ambientColour( Moo::Colour( runTimeColour() ) );
				Moo::rc().lightContainer( s_lighting_ );

				attachment_->draw( Moo::rc().world(), 0.f );

				if ( overlay )
				{
					Moo::SortedChannel::draw();
				}

				Moo::rc().lightContainer( pLC );
			}
		}

		Moo::rc().pop();
		Moo::rc().device()->SetTransform( D3DTS_WORLD, &Moo::rc().world() );
	}

	momentarilyInvisible( false );

	Moo::rc().setRenderState( D3DRS_LIGHTING, FALSE );
	Moo::rc().setVertexShader( NULL );
	Moo::rc().setFVF( GUIVertex::fvf() );
}


void MeshGUIAdaptor::addAsSortedDrawItem( const Matrix& worldTransform )
{
	BW_GUARD;
	if ( visible() )
	{
		//Add children to sorted tree
		SimpleGUIComponent::addAsSortedDrawItem( worldTransform );
		
		if ( attachment_ && !momentarilyInvisible_ ) //don't check the texture_ member ( see SGC.ipp )
		{
			//Draw mesh now.
			//How this works:
			//	Mesh is opaque, in which case drawing order is independent
			//	Mesh is transparent, in which case 'drawing it' will merely
			//	get it to add itself as as sorted draw items
			Moo::rc().push();
			Moo::rc().world( worldTransform );
			Moo::rc().device()->SetTransform( D3DTS_WORLD, &Moo::rc().world() );		
		
			Matrix m( Matrix::identity );
			if ( transform_ ) transform_->matrix( m );
			Moo::rc().preMultiply(m);
			
			SimpleGUI::instance().setConstants(runTimeColour(), pixelSnap());
			Moo::LightContainerPtr pLC = Moo::rc().lightContainer();
			s_lighting_->ambientColour( Moo::Colour( runTimeColour() ) );
			Moo::rc().lightContainer( s_lighting_ );

			attachment_->draw( Moo::rc().world(), 0.f );				

			Moo::rc().lightContainer( pLC );			
			Moo::rc().pop();
			Moo::rc().device()->SetTransform( D3DTS_WORLD, &Moo::rc().world() );
		}
	}

	momentarilyInvisible( false );

	Moo::rc().setRenderState( D3DRS_LIGHTING, FALSE );
	Moo::rc().setVertexShader( NULL );
	Moo::rc().setFVF( GUIVertex::fvf() );
}


/**
 *	Get an attribute for python
 */
PyObject * MeshGUIAdaptor::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();
	return SimpleGUIComponent::pyGetAttribute( attr );
}

/**
 *	Set an attribute for python
 */
int MeshGUIAdaptor::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();
	return SimpleGUIComponent::pySetAttribute( attr, value );
}


/**
 *	Factory method
 */
PyObject * MeshGUIAdaptor::pyNew( PyObject * args )
{
	BW_GUARD;
	return new MeshGUIAdaptor;
}


// -----------------------------------------------------------------------------
// Section: Static Initisation
// -----------------------------------------------------------------------------
Moo::LightContainerPtr MeshGUIAdaptor::s_lighting_ = NULL;