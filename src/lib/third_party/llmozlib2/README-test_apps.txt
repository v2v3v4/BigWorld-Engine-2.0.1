LLMozLib2 Test applications
===========================

There are a some simple test applications, built against LLMozLib2 that serve as test mules, examples of how it can be used and what it is/isn't capable of.

They are (or will be eventually) available for all 3 of our supported platforms.

testGL
======
    * Very simple OpenGL application that displays a single Web page (URL is hard coded into the app) on a single orthographic quad.

    * Can be found in tests/testgl directory or viewed at https://svn.secondlife.com/svn/llmozlib/trunk/llmozlib2/tests/testgl/

    * Uses GLUT (http://www.opengl.org/resources/libraries/glut/) for window/input/OpenGL support

    * Mouse/keyboard interaction work as expected

    * Resizes as the application window resizes

    * ESC key exits 


uBrowser
========
    * OpenGL application that display multiple Web pages 

    * Can be found in tests/ubrowser directory or viewed at https://svn.secondlife.com/svn/llmozlib/trunk/llmozlib2/tests/ubrowser/

    * Uses GLUT (http://www.opengl.org/resources/libraries/glut/) for window/input/OpenGL support and GLUI (http://glui.sourceforge.net/) for user interface support. 

    * You can enter a new URL manually by typing into the URL edit field and pressing ENTER (this version of GLUI doesn't support cut/paste unfortunately) 

    * Mouse/keyboard interaction work as expected 

    * Due to a limitation of GLUI (or a limitation of my understanding) you can't return focus automatically to the UI when you click in the URL edit widget. This means you have to click on the "Focus" button first to enter a new URL. 

    * The '<<<' (back), 'HOME' and '>>>' (forward) buttons work as you might expect 

    * There is a long list of bookmarks built in that illustrate the sorts of sites and technologies that are supported (AJAX, SHTML, XUL, SVG etc.) 

    * You can select the type of geometry that is used to display the page.
          * Cube - writes a different page to each side of the cube
          * Flat - is a single quadrilateral - just like having a regular browser (sort of)
          * Ball - My favorite - Google Maps on it looks especially good!
          * Flag - More to demonstrate that it can be done that anything useful. 

    * You can rotate the geometry to a pre-defined position using the controls at the side. 

    * You can change the position, orientation and scale of the geometry by clicking on the 'Rotate', 'Translate' and 'Scale' controls. Hold the mouse down over them and drag. 

    * You can change the resolution of the embedded browser window by pressing the 'small', 'medium' and 'large' buttons. Since, in this application, the browser window is always stretched to fill the geometry, the smaller the browser window, the larger it renders - (if that makes any sense) 

    * You'll see status and progress messages at the bottom - just like a real browser! 

