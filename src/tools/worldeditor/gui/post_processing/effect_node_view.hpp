/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EFFECT_NODE_VIEW_HPP
#define EFFECT_NODE_VIEW_HPP


#include "graph/graph_view.hpp"


// Forward declarations
class EffectNode;
typedef SmartPointer< EffectNode > EffectNodePtr;


/**
 *	This class implements an Effect node view, which is responsible for
 *	displaying an EffectNode on the GraphView canvas.
 */
class EffectNodeView : public Graph::NodeView
{
public:
	EffectNodeView( Graph::GraphView & graphView, const EffectNodePtr node );
	~EffectNodeView();

	// Graph::NodeView interface
	const CRect & rect() const { return rect_; }

	int zOrder() const;

	int alpha() const;

	void position( const CPoint & pos );

	void draw( CDC & dc, uint32 frame, STATE state );

	void leftClick( const CPoint & pt );

	void doDrag( const CPoint & pt );
	void endDrag( const CPoint & pt, bool canceled = false );

private:
	EffectNodePtr node_;
	CRect rect_;
};

#endif // EFFECT_NODE_VIEW_HPP
