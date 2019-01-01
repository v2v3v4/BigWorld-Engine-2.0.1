/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUITABS_DATATYPES_HPP
#define GUITABS_DATATYPES_HPP

/**
 *	GUI Tearoff panel framework - Datatypes
 */

namespace GUITABS
{


/**
 *	This enumerated type lists the potential docking positions where a panel
 *	can be insterted.
 */
enum InsertAt
{
	UNDEFINED_INSERTAT,
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
	TAB,
	FLOATING,
	SUBCONTENT // as a content inside a ContentContainer panel
};


/**
 *	This enumerated type aids in specifying the orientation of a splitter
 *	window.
 */
enum Orientation
{
	UNDEFINED_ORIENTATION,
	VERTICAL,
	HORIZONTAL
};


// Forward declarations.
class DragManager;
class Manager;
class ContentFactory;
class Content;
class ContentContainer;
class Tab;
class TabCtrl;
class Panel;
class DockedPanelNode;
class SplitterNode;
class DockNode;
class Floater;
class Dock;


// Typedefs
typedef SmartPointer<Manager> ManagerPtr; // just a pointer to a singleton

typedef SmartPointer<DragManager> DragManagerPtr;

typedef Content* PanelHandle; // This type is used only for weak references.

typedef SmartPointer<ContentFactory> ContentFactoryPtr;
typedef SmartPointer<Content> ContentPtr;
typedef SmartPointer<Tab> TabPtr;
typedef SmartPointer<TabCtrl> TabCtrlPtr;
typedef SmartPointer<Panel> PanelPtr;
typedef SmartPointer<DockedPanelNode> DockedPanelNodePtr;
typedef SmartPointer<SplitterNode> SplitterNodePtr;
typedef SmartPointer<DockNode> DockNodePtr;
typedef SmartPointer<Floater> FloaterPtr;
typedef SmartPointer<Dock> DockPtr;


} // namespace

#endif // GUITABS_DATATYPES_HPP