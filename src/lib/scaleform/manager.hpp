/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCALEFORM_MANAGER_HPP
#define SCALEFORM_MANAGER_HPP
#if SCALEFORM_SUPPORT

#include "cstdmf/init_singleton.hpp"
#include "input/input.hpp"
#include "moo/moo_dx.hpp"
#include "moo/device_callback.hpp"

#include "config.hpp"
#include "log.hpp"
#include "loader.hpp"
#include "py_movie_view.hpp"

#include <GFxRenderConfig.h>
#include <GRendererD3D9.h>
#include <GFxDrawText.h>

namespace Scaleform
{
	/*class UserEventHandler : public GFxUserEventHandler
	{
	public:

		UserEventHandler(){}

		virtual void HandleEvent(GFxMovieView* pmovie, const GFxEvent& event);
	};*/


	class Manager : public InitSingleton< Manager >, public Moo::DeviceCallback
	{
	public:
		typedef Manager This;

		//Singleton virtual methods
		virtual bool doInit();
		virtual bool doFini();

		//Moo::DeviceCallback virtual methods
		void deleteUnmanagedObjects();
		void createUnmanagedObjects();
		void deleteManagedObjects();
		void createManagedObjects();

		void tick(float elapsedTime, uint32 frameCatchUpCount = 2);
		void draw();

		//Renderer fns
		void createRenderer(DX::Device* device,
							D3DPRESENT_PARAMETERS* presentParams,
							bool noSceneCalls,
							HWND hwnd);
		void resetDevice(DX::Device* device, D3DPRESENT_PARAMETERS* presentParams);
		void destroyRenderer();
		GPtr<GRendererD3D9> pRenderer()		{ return pRenderer_; }
		Loader* pLoader()					{ return pLoader_; }
		GPtr<Log> pLogger()					{ return pLogger_; }
		GPtr<GFxDrawTextManager> pTextManager() { return pDrawTextManager_; }
		void setDrawTextManager( GPtr<GFxDrawTextManager> dtm );

		//Movie View management fns
		void addMovieView(PyMovieView* obj);
		void removeMovieView(PyMovieView* obj);

		//Input fns
		bool onKeyEvent(const KeyEvent &event);
		//bool onCharEvent( const CharEvent & event );
		bool onMouse(int buttons, int nMouseWheelDelta, int xPos, int yPos);
		void enablePygfxMouseHandle(bool state);

		GRendererD3D9::VMConfigFlags configFlag() const			{ return configFlag_; }
		HWND hwnd() const				{ return hwndModule_; }

		Manager();
		virtual ~Manager();

		//TODO - remove.  build dynamically during mouse move events.
		KeyEvent lastMouseKeyEvent_;

		//TODO - better support for font map/lib.  Break into own class and also
		//add font caching etc.  Don't just expose as public member vars.
		GPtr<GFxFontMap> pFontMap_;
		GPtr<GFxFontLib> pFontLib_;

	private:
		GFxSystem system_;
		Loader* pLoader_;
		GPtr<GRendererD3D9> pRenderer_;
		GPtr<GFxRenderConfig> pRenderConfig_;
		GPtr<GFxRenderStats> pRenderStats_;
		GPtr<GFxDrawTextManager> pDrawTextManager_;
		GPtr<Log> pLogger_;
		GRendererD3D9::VMConfigFlags configFlag_;
		HWND hwndModule_;
		std::list<PyMovieView*> list_;
	};
} //namespace Scaleform

#endif // #if SCALEFORM_SUPPORT
#endif // SCALEFORM_MANAGER_HPP