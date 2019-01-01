/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCALEFORM_PY_MOVIE_DEF_HPP
#define SCALEFORM_PY_MOVIE_DEF_HPP
#if SCALEFORM_SUPPORT

#include "pyscript/script.hpp"
#include "pyscript/pyobject_plus.hpp"

namespace Scaleform
{
	/*~	class Scaleform.PyMovieDef
	 *
	 *	This class wraps Scaleform's Movie Definition and provides
	 *	lifetime control and python access to its API.
	 *
	 *	A Movie Definition contains the resources for an .swf file,
	 *	and allows creation of instanced views of the movie.
	 *
	 *	Movie Definitions can also be loaded for the sole purpose
	 *	of providing fonts for other movies and for flash text components.
	 */
	class PyMovieDef : public PyObjectPlus
	{
		Py_Header( PyMovieDef, PyObjectPlus )
	public:
		PyMovieDef( GFxMovieDef* impl, PyTypePlus *pType = &s_type_ );
		~PyMovieDef();

		PyObject *pyGetAttribute( const char *attr );
		int pySetAttribute( const char *attr, PyObject *value );

		PyObject * createInstance( bool initFirstFrame = true );
		PY_AUTO_METHOD_DECLARE( RETOWN, createInstance, OPTARG( bool, true, END ) )

		void setAsFontMovie();
		PY_AUTO_METHOD_DECLARE( RETVOID, setAsFontMovie, END )

		void addToFontLibrary( bool pin);
		PY_AUTO_METHOD_DECLARE( RETVOID, addToFontLibrary, OPTARG( bool, true, END ) )

		PY_FACTORY_DECLARE()

		static GFxFontLib* GetFontLib(PyMovieDef* self, PyObject* args);
	private:
		GPtr<GFxMovieDef> pMovieDef_;
	};
} // namespace Scaleform

PY_SCRIPT_CONVERTERS_DECLARE( Scaleform::PyMovieDef )

#endif // #if SCALEFORM_SUPPORT
#endif // SCALEFORM_PY_MOVIE_DEF_HPP