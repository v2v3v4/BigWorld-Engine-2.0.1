/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PHASE_NODE_VIEW_HPP
#define PHASE_NODE_VIEW_HPP


#include "graph/graph_view.hpp"
#include "pp_preview.hpp"


// Forward declarations
class PhaseNode;
typedef SmartPointer< PhaseNode > PhaseNodePtr;


/**
 *	This class implements an Phase node view, which is responsible for
 *	displaying an PhaseNode on the GraphView canvas.
 */
class PhaseNodeView : public Graph::NodeView
{
public:
	PhaseNodeView( Graph::GraphView & graphView, const PhaseNodePtr node, PPPreviewPtr preview );
	~PhaseNodeView();

	// Graph::NodeView interface
	const CRect & rect() const { return rect_; }

	int zOrder() const;

	int alpha() const;

	void position( const CPoint & pos );

	void draw( CDC & dc, uint32 frame, STATE state );

	void drawRenderTargetName( CDC & dc, const CPoint & pos );

	void leftBtnDown( const CPoint & pt );

	void leftClick( const CPoint & pt );

	void doDrag( const CPoint & pt );
	void endDrag( const CPoint & pt, bool canceled = false );

	PostProcessing::Phase* phase() const;

	bool isRenderTargetDragging() const { return isRenderTargetDragging_; }

private:
	PhaseNodePtr node_;
	CRect rect_;
	CRect renderTargetDragRect_;
	bool isRenderTargetDragging_;
	char * pPreviewImg_;
	int previewInterleave_;
	std::wstring outputStr_;
	std::string lastOutput_;
	int lastOutputW_;
	int lastOutputH_;
	PPPreviewPtr preview_;

	bool calcPreviewImg( ComObjectWrap<DX::Texture> pPreview, const CRect& srcRect, int width, int height, char * retPixels ) const;

	void updateOutputString();
};

#endif // PHASE_NODE_VIEW_HPP
