/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Linden Lab Inc. (http://lindenlab.com) code.
 *
 * The Initial Developer of the Original Code is:
 *   Callum Prentice (callum@ubrowser.com)
 *
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Callum Prentice (callum@ubrowser.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef UBROWSER_H
#define UBROWSER_H

#include <string>
#include <vector>
#include <math.h>

#include "GL/glut.h"
#include "glui.h"
#include "llmozlib2.h"

static void gluiCallbackWrapper( int controlIdIn );

////////////////////////////////////////////////////////////////////////////////
//
class uBrowser :
	public LLEmbeddedBrowserWindowObserver
{
	public:
		uBrowser();
		~uBrowser();

		bool init( const char* arg0, int appWindowIn );
		bool reset();
		const std::string& getName() { return mName; };
		void reshape( int widthIn, int heightIn );
		void makeChrome();
		void windowPosToTexturePos( int winXIn, int winYIn, int& texXOut, int& texYOut, int& faceOut );
		void resetView();
		void drawGeometry( int geomTypeIn, int updateTypeIn );

		void display();
		void idle();
		void keyboard( unsigned char keyIn, int xIn, int yIn );
		void passiveMouse( int xIn, int yIn );
		void mouseButton( int button, int state, int xIn, int yIn );
		void mouseMove( int xIn, int yIn );

		void setSize( int indexIn, int widthIn , int heightIn );

		void gluiCallback( int controlIdIn );

		virtual void onPageChanged( const EventType& eventIn );
		virtual void onNavigateBegin( const EventType& eventIn );
		virtual void onNavigateComplete( const EventType& eventIn );
		virtual void onUpdateProgress( const EventType& eventIn );
		virtual void onStatusTextChange( const EventType& eventIn );
		virtual void onLocationChange( const EventType& eventIn );
		virtual void onClickLinkHref( const EventType& eventIn );
		virtual void onClickLinkNoFollow( const EventType& eventIn );

	private:
		enum Constants { MaxBrowsers = 6 };

		void* getNativeWindowHandle();
		void setFocusNativeWindow();
		GLenum getGLTextureFormat(int size);

		const int mVersionMajor;
		const int mVersionMinor;
		const int mVersionPatch;
		std::string mName;
		int mAppWindow;
		int mWindowWidth;
		int mWindowHeight;
		int mTextureWidth;
		int mTextureHeight;
		int mBrowserWindowWidth;
		int mBrowserWindowHeight;
		float mTextureScaleX;
		float mTextureScaleY;
		float mViewportAspect;
		float mViewPos[ 3 ];
		float mViewRotation[ 16 ];
		unsigned char mPixelColorRB[ 3 ];
		unsigned char mPixelColorG[ 3 ];
		int mCurMouseX;
		int mCurMouseY;
		GLuint mRedBlueTexture;
		unsigned char mRedBlueTexturePixels[ 256 * 256 * 3 ];
		GLuint mGreenTexture[ MaxBrowsers ];
		unsigned char mGreenTexturePixels[ MaxBrowsers ][ 16 * 16 * 3 ];
		GLuint mAppTexture[ MaxBrowsers ];
		unsigned char* mAppTexturePixels;
		int mSelBookmark;
		int mCurObjType;
		GLUI* mTopGLUIWindow;
		GLUI_Button* mNavBackButton;
		GLUI_Button* mNavStopButton;
		GLUI_Button* mNavForwardButton;
		GLUI* mTop2GLUIWindow;
		GLUI_EditText* mUrlEdit;
		GLUI_String mNavUrl;
		GLUI* mRightGLUIWindow;
		GLUI_Rotation* mViewRotationCtrl;
		GLUI_Translation* mViewScaleCtrl;
		GLUI_Translation* mViewTranslationCtrl;
		GLUI* mBottomGLUIWindow;
		GLUI_StaticText* mStatusText;
		GLUI_StaticText* mProgressText;
		const int mIdReset;
		const int mIdExit;
		const int mIdClearCookies;
		const int mIdBookmarks;
		const int mIdGeomTypeNull;
		const int mIdGeomTypeFlat;
		const int mIdGeomTypeBall;
		const int mIdGeomTypeCube;
		const int mIdGeomTypeFlag;
		const int mIdUrlEdit;
		const int mIdNavBack;
		const int mIdNavStop;
		const int mIdNavHome;
		const int mIdNavForward;
		const int mIdNavReload;
		const int mIdBrowserSmall;
		const int mIdBrowserMedium;
		const int mIdBrowserLarge;
		const int mIdFace0;
		const int mIdFace1;
		const int mIdFace2;
		const int mIdFace3;
		const int mIdFace4;
		const int mIdFace5;
		const int mIdUpdateTypeRB;
		const int mIdUpdateTypeG;
		const int mIdUpdateTypeApp;
		std::string mHomeUrl[ MaxBrowsers ];
		std::vector< std::pair< std::string, std::string > > mBookmarks;
		bool mNeedsUpdate[ MaxBrowsers ];
		int mWindowId[ MaxBrowsers ];
		int mCurWindowId;
		int mCurFace;
		GLfloat	mRotX;
		GLfloat	mRotY;
		GLfloat	mRotZ;
		const int mNumBrowserWindows;
};

#endif // UBROWSER_H

