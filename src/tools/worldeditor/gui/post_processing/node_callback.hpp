/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NODE_CALLBACK_HPP
#define NODE_CALLBACK_HPP


// Forward declarations
class BasePostProcessingNode;
typedef SmartPointer< BasePostProcessingNode > BasePostProcessingNodePtr;


/**
 *	This class works as an base interface class for other class that want to 
 *	receive notifications from an Effect node or a Phase node.
 */
class NodeCallback
{
public:
	virtual void nodeClicked( const BasePostProcessingNodePtr & node ) {}
	virtual void nodeActive( const BasePostProcessingNodePtr & node, bool active ) {}
	virtual void nodeDeleted( const BasePostProcessingNodePtr & node ) {}
	virtual void doDrag( const CPoint & pt, const BasePostProcessingNodePtr & node ) {}
	virtual void endDrag( const CPoint & pt, const BasePostProcessingNodePtr & node, bool canceled = false ) {}
};


#endif // NODE_CALLBACK_HPP
