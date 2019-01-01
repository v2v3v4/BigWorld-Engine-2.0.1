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

#include "ubrowser.h"

#include "GL/glut.h"
#include "glui.h"

uBrowser* theApp;

////////////////////////////////////////////////////////////////////////////////
//
void glutReshape( int widthIn, int heightIn )
{
	if ( theApp )
		theApp->reshape( widthIn, heightIn );
};

////////////////////////////////////////////////////////////////////////////////
//
void glutDisplay()
{
	if ( theApp )
		theApp->display();
};

////////////////////////////////////////////////////////////////////////////////
//
void glutIdle()
{
	if ( theApp )
		theApp->idle();
};

////////////////////////////////////////////////////////////////////////////////
//
void glutKeyboard( unsigned char keyIn, int xIn, int yIn )
{
	if ( theApp )
		theApp->keyboard( keyIn, xIn, yIn );
};

////////////////////////////////////////////////////////////////////////////////
//
void glutSpecialKeyboard( int keyIn, int xIn, int yIn )
{
	// appears that you need this defined even if it's empty
	// passing NULL for the handler func ptr crashes this app
};

////////////////////////////////////////////////////////////////////////////////
//
void glutPassiveMouse( int xIn, int yIn )
{
	if ( theApp )
		theApp->passiveMouse( xIn, yIn );
}

////////////////////////////////////////////////////////////////////////////////
//
void glutMouseMove( int xIn , int yIn )
{
	if ( theApp )
		theApp->mouseMove( xIn, yIn );
}

////////////////////////////////////////////////////////////////////////////////
//
void glutMouseButton( int buttonIn, int stateIn, int xIn, int yIn )
{
	if ( theApp )
		theApp->mouseButton( buttonIn, stateIn, xIn, yIn );
}

////////////////////////////////////////////////////////////////////////////////
//
int main( int argc, char* argv[] )
{
	theApp = new uBrowser;

	if ( theApp )
	{
		glutInit( &argc, argv );
		glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB );

		glutInitWindowPosition( 80, 0 );
		glutInitWindowSize( 1024, 900 );

		int appWindow = glutCreateWindow( theApp->getName().c_str() );

		glutDisplayFunc( glutDisplay );

		GLUI_Master.set_glutReshapeFunc( glutReshape );
		GLUI_Master.set_glutKeyboardFunc( glutKeyboard );
		GLUI_Master.set_glutMouseFunc( glutMouseButton );
		GLUI_Master.set_glutSpecialFunc( glutSpecialKeyboard );

		glutPassiveMotionFunc( glutPassiveMouse );
		glutMotionFunc( glutMouseMove );

		glutSetWindow( appWindow );

		theApp->init( argv[ 0 ], appWindow );

		GLUI_Master.set_glutIdleFunc( glutIdle );

		glutMainLoop();

		delete theApp;
	};

	return 1;
}

