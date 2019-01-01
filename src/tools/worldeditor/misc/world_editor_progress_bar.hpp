/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WORLD_EDITOR_PROGRESS_BAR_HPP
#define WORLD_EDITOR_PROGRESS_BAR_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "ashes/gui_attachment.hpp"
#include "ashes/text_gui_component.hpp"
#include "romp/super_model_progress.hpp"
#include "moo/node.hpp"


class WorldEditorProgressBar : public SuperModelProgressDisplay
{
public:
	WorldEditorProgressBar();
	~WorldEditorProgressBar();

	virtual void fini();

	void drawOther(float dTime);
	void escapable( bool escape );
	void setLabel( const std::string& label );

private:
	SmartPointer<Moo::Node>				taskNode_;
	SmartPointer<GuiAttachment>			taskAttachment_;
	SmartPointer<TextGUIComponent>		taskText_;
	SmartPointer<TextGUIComponent>		escapeText_;
	SmartPointer<GUIShader>				textPosition_;
	SmartPointer<PyMatrix>				textTransform_;

	bool escapable_;
	bool inited_;
};


#endif // WORLD_EDITOR_PROGRESS_BAR_HPP
