/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCALEFORM_PY_MOVIE_VIEW_HPP
#define SCALEFORM_PY_MOVIE_VIEW_HPP

#include "pyscript/script.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "py_fs_command_handler.hpp"
#include "py_external_interface.hpp"
#include <GFxPlayer.h>

#include "input/input.hpp"
#include "input/py_input.hpp"


namespace Scaleform
{
	/*~	class Scaleform.PyMovieView
	 *
	 *	This class wraps Scaleform's Movie View and provides
	 *	lifetime control and python access to its API.
	 *
	 *	A Movie View is an instanced view of a Movie Definition,
	 *	and can only be created via the PyMovieDef.createInstance()
	 *	method.
	 */
	class PyMovieView : public PyObjectPlus
	{
		Py_Header( PyMovieView, PyObjectPlus )
	public:
		PyMovieView( GFxMovieView* pMovieView_, PyTypePlus *pType = &s_type_ );
		~PyMovieView();

		GFxMovieView* pMovieView() const	{ return pMovieView_.GetPtr(); }

		PyObject *pyGetAttribute( const char *attr );
		int pySetAttribute( const char *attr, PyObject *value );

		void setFSCommandCallback(PyObjectPtr args);
		PY_AUTO_METHOD_DECLARE( RETVOID, setFSCommandCallback, ARG( PyObjectPtr, END ) )

		void setExternalInterfaceCallback(PyObjectPtr args);
		PY_AUTO_METHOD_DECLARE( RETVOID, setExternalInterfaceCallback, ARG( PyObjectPtr, END ) )

		PyObject* invoke(PyObjectPtr pyArgs);
		PY_AUTO_METHOD_DECLARE( RETOWN, invoke, ARG( PyObjectPtr, END ) )

		void userData(PyObject* args);
		PyObject* userData();
		PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( PyObject*, userData, userData );

		enum eScaleModeType
		{
			SM_NoScale = GFxMovieView::SM_NoScale,
			SM_ShowAll = GFxMovieView::SM_ShowAll,
			SM_ExactFit = GFxMovieView::SM_ExactFit,
			SM_NoBorder = GFxMovieView::SM_NoBorder,
		};

		PY_BEGIN_ENUM_MAP( eScaleModeType, SM_ )
			PY_ENUM_VALUE( SM_NoScale )
			PY_ENUM_VALUE( SM_ShowAll )
			PY_ENUM_VALUE( SM_ExactFit )
			PY_ENUM_VALUE( SM_NoBorder )
		PY_END_ENUM_MAP()

		eScaleModeType scaleMode() const;
		void scaleMode( eScaleModeType s );
		PY_DEFERRED_ATTRIBUTE_DECLARE( scaleMode )

		void gotoFrame( uint32 frameNumber);
		PY_AUTO_METHOD_DECLARE( RETVOID, gotoFrame, ARG( uint32, END ) )

		bool gotoLabeledFrame(const std::string& plabel, int32 offset = 0);
		PY_AUTO_METHOD_DECLARE( RETOK, gotoLabeledFrame, ARG( std::string, OPTARG( int32, 0, END) ) )

		void visible( bool v )	{ this->pMovieView_->SetVisible(v); }
		bool visible() const	{ return this->pMovieView_->GetVisible(); }
		PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, visible, visible )

		float backgroundAlpha() const	{ return pMovieView_->GetBackgroundAlpha(); }
		void backgroundAlpha( float f ) { pMovieView_->SetBackgroundAlpha( (Float)f ); }
		PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( float, backgroundAlpha, backgroundAlpha )

		void restart()	{ pMovieView()->Restart(); }
		PY_AUTO_METHOD_DECLARE( RETVOID, restart, END )

		void setPause( bool state )	{ pMovieView()->SetPause( state ); }
		PY_AUTO_METHOD_DECLARE( RETVOID, setPause, ARG( bool, END ) )

		void setFocussed();
		PY_AUTO_METHOD_DECLARE( RETVOID, setFocussed, END );

		void showAsGlobalMovie( bool state = true );
		PY_AUTO_METHOD_DECLARE( RETVOID, showAsGlobalMovie, OPTARG( bool, true, END ) )

		bool handleKeyEvent( const KeyEvent & /*event*/ );
		PY_AUTO_METHOD_DECLARE( RETDATA, handleKeyEvent, ARG( KeyEvent, END ) );

		bool handleMouseButtonEvent( const KeyEvent & /*event*/ );
		PY_AUTO_METHOD_DECLARE( RETDATA, handleMouseButtonEvent, ARG( KeyEvent, END ) );

		bool handleMouseEvent( const MouseEvent & /*event*/ );
		PY_AUTO_METHOD_DECLARE( RETDATA, handleMouseEvent, ARG( MouseEvent, END ) );


	private:

		GPtr<GFxMovieView> pMovieView_;
		PyObject* userData_;
		GPtr<PyFSCommandHandler> fsCommandHandler_;
		GPtr<PyExternalInterface> externalInterface_;

		KeyEvent			lastMouseKeyEvent_;
	};

	typedef SmartPointer<PyMovieView>	PyMovieViewPtr;

}	//namespace Scaleform

PY_ENUM_CONVERTERS_DECLARE( Scaleform::PyMovieView::eScaleModeType )

#endif	//SCALEFORM_PY_MOVIE_VIEW_HPP