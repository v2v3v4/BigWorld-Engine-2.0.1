/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef POST_PROC_PROPERTY_EDITOR_HPP
#define POST_PROC_PROPERTY_EDITOR_HPP


#include "gizmo/general_editor.hpp"


// Forward declarations.
class BasePostProcessingNode;
typedef SmartPointer< BasePostProcessingNode > BasePostProcessingNodePtr;


/**
 *	This class allows editing a node's properties on a property list, 
 *	query a node's properties, or set a node property's value.
 */
class PostProcPropertyEditor : public GeneralEditor
{
public:
	static const int TEXTURES = 1;
	static const int RENDER_TARGETS = 2;
	static const int SHADERS = 4;

	PostProcPropertyEditor( BasePostProcessingNodePtr node );

	void getProperties( int types, std::vector< std::string > & retProps ) const;

	void setProperty( const std::string & name, const std::string & value );

private:
	BasePostProcessingNodePtr node_;
};

typedef SmartPointer< PostProcPropertyEditor > PostProcPropertyEditorPtr;


#endif // POST_PROC_PROPERTY_EDITOR_HPP
