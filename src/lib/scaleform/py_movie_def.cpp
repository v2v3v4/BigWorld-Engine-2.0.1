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
#if SCALEFORM_SUPPORT

#include "py_movie_def.hpp"
#include "manager.hpp"
#include "moo/render_context.hpp"
#include <GFxPlayer.h>
#include <GFxFontLib.h>

namespace Scaleform
{
	PY_TYPEOBJECT( PyMovieDef )

	PY_BEGIN_METHODS( PyMovieDef )
		/*~ function PyMovieDef.createInstance
		 *
		 *	This function creates an instance of the movie definition.
		 *	It returns a PyMovieView object.
		 *
		 *	@param	bool	[optional] Specifies whether the frame1 tags of the
		 *	swf movie are executed or not during the createInstance call.
		 *	Default: true
		 */
		PY_METHOD( createInstance )
		/*~ function PyMovieDef.setAsFontMovie
		 *
		 *	Set this movie as the font source for the DrawTextManager, which is
		 *	used by FlashTextComponents to draw with.  This function is not used
		 *	to provide fonts for other movies, use PyMovieDef.addToFontLibrary
		 *	instead.
		 */
		PY_METHOD( setAsFontMovie )
		/*~ function PyMovieDef.addToFontLibrary
		 *
		 *	Add this movie to the font library used by all Scaleform movies.
		 *
		 *	@param	pin		[optional] If true, the font library is pinned to
		 *					the scaleform subsystem, meaning afterwards you
		 *					no longer have to keep a reference to this movie
		 *					definition instance.
		 *					If false, the movieDef still controls the lifetime
		 *					of its fonts in the font library.
		 *					Defaults to True.
		 */
		PY_METHOD( addToFontLibrary )
	PY_END_METHODS()


	PY_BEGIN_ATTRIBUTES( PyMovieDef )	
	PY_END_ATTRIBUTES()


	/*~ function Scaleform.MovieDef
	 *	Factory function to create and return a Scaleform Movie Definition. 
	 *	@return A new Scaleform PyMovieDef object.
	 */
	PY_FACTORY_NAMED( PyMovieDef, "MovieDef", _Scaleform )


	PyMovieDef::PyMovieDef( GFxMovieDef* impl, PyTypePlus *pType ):
		PyObjectPlus( pType ),
		pMovieDef_( *impl )
	{
		BW_GUARD;
	}


	PyMovieDef::~PyMovieDef()
	{
		BW_GUARD;
		pMovieDef_ = NULL;
	}


	PyObject* PyMovieDef::createInstance( bool initFirstFrame )
	{
		BW_GUARD;
		// Create an instance of the movie definition.
		GFxMovieView* movieView = pMovieDef_->CreateInstance( initFirstFrame );

		//Sets the viewport to the movie Clip irrespective of the resolution.
		D3DVIEWPORT9 pViewport;
		Moo::rc().device()->GetViewport( &pViewport );
		movieView->SetViewport( (SInt)Moo::rc().backBufferDesc().Width,  (SInt)Moo::rc().backBufferDesc().Height , (SInt)pViewport.X, (SInt)pViewport.Y, (SInt)Moo::rc().screenWidth(), (SInt)Moo::rc().screenHeight() );
		
		if (!movieView)
		{
			// File not found exception
			PyErr_SetString( PyExc_Exception, "GFx internally failed." );
			return NULL;
		}

		return new PyMovieView(movieView);
	}


	GFxFontLib* PyMovieDef::GetFontLib( PyMovieDef* self, PyObject* args )
	{
		BW_GUARD;
		GFxFontLib *fontLib = self->pMovieDef_->GetFontLib();
		return (fontLib) ? fontLib : 0;
	}


	void PyMovieDef::setAsFontMovie()
	{
		BW_GUARD;
		GPtr<GFxDrawTextManager> pdm = *new GFxDrawTextManager(this->pMovieDef_);
		Manager::instance().setDrawTextManager(pdm);
	}


	void PyMovieDef::addToFontLibrary( bool pin )
	{
		BW_GUARD;
		Manager::instance().pFontLib_->AddFontsFrom( pMovieDef_, pin );
	}


	PyObject * PyMovieDef::pyGetAttribute( const char *attr )
	{
		PY_GETATTR_STD();
		return PyObjectPlus::pyGetAttribute(attr);
	}


	int PyMovieDef::pySetAttribute( const char *attr, PyObject *value )
	{
		PY_SETATTR_STD();
		return PyObjectPlus::pySetAttribute(attr,value);
	}


	PyObject* PyMovieDef::pyNew( PyObject* args )
	{
		BW_GUARD;
		char * fileName;
		if (!PyArg_ParseTuple( args, "s", &fileName ))
		{
			PyErr_SetString( PyExc_TypeError, "Scaleform.MovieDef: "
				"Argument parsing error: Expected a resource ID" );
			return NULL;
		}

		// Load the movie definition at the path
		GFxMovieDef* movieDef = Manager::instance().pLoader()->CreateMovie(fileName);
		if (!movieDef)
		{
			// File not found exception
			PyErr_SetString(PyExc_IOError, fileName);
			return NULL;
		}

		return new PyMovieDef( movieDef );
	}

}	//namespace Scaleform

#endif //SCALEFORM_SUPPORT