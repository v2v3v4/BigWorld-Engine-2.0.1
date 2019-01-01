/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VIEW_SKIN_HPP
#define VIEW_SKIN_HPP


/**
 *	This class is a centralised repository for drawing settings used in the
 *	post-processing panel.
 */
class ViewSkin
{
public:
	static COLORREF bkColour();
	static COLORREF footerColour();

	static COLORREF keyColour();
	static bool keyColourTransparency();

	static COLORREF effectFontActive();
	static COLORREF effectFontInactive();
	static COLORREF effectActiveGradient1();
	static COLORREF effectActiveGradient2();
	static COLORREF effectInactiveGradient1();
	static COLORREF effectInactiveGradient2();

	static COLORREF phaseFontActive();
	static COLORREF phaseFontInactive();
	static COLORREF phaseActiveGradient1();
	static COLORREF phaseActiveGradient2();
	static COLORREF phaseInactiveGradient1();
	static COLORREF phaseInactiveGradient2();

	static COLORREF nodeNormalEdge();
	static COLORREF nodeSelectedEdge();

	static int nodeEdgeCurve();
	static int nodeNormalEdgeSize();
	static int nodeSelectedEdgeSize();

	static COLORREF effectEdgeColour();
	static COLORREF phaseEdgeColour();

	static int edgeLineSize();
	static int edgeArrowSize();

	static CSize nodeRectTextMargin();

	static CSize effectNodeSize();
	static CSize phaseNodeSize();
	static CSize nodeMargin();
	static CSize effectNodeSeparation();
	static CSize phaseNodeSeparation();

	static void nodeZoom( int nodeZoom );

	static int dragAlpha();

	static float nodeScale();

private:
	static int s_nodeZoom_;
	static float s_nodeZoomFactor_;
};


#endif // VIEW_SKIN_HPP
