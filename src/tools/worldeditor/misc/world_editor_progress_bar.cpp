/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "worldeditor/misc/world_editor_progress_bar.hpp"
#include "ashes/matrix_gui_shader.hpp"
#include "ashes/simple_gui.hpp"
#include "resmgr/string_provider.hpp"
#include "romp/font_manager.hpp"


/**
 *	This class handles the fancy-pants progress bar in WorldEditor.
 *	Its currently specialised to work only with the 'loader2.model' progress
 *	bar. 
 *	i.e. Do not use with other progress bar models.
 */ 
WorldEditorProgressBar::WorldEditorProgressBar():
	SuperModelProgressDisplay( localiseFileName( "resources/maps/gui/loader2.model" ) ),
	escapable_( false ),
	inited_( true )
{
	BW_GUARD;

	taskText_ = new TextGUIComponent();
	taskText_->filterType( SimpleGUIComponent::FT_LINEAR );
	taskText_->materialFX( SimpleGUIComponent::FX_ADD );
	taskText_->colour( 0xffC76535 );//199,101,53

	taskNode_ = superModel_->findNode( "Rectangle18" );

	MatrixGUIShader* mgs = new MatrixGUIShader();
	textTransform_ = new PyMatrix();
	textTransform_->setScale(64,32,1);
	textTransform_->postRotateX(DEG_TO_RAD(90));
	textTransform_->translation(Vector3(4,-0.5,0));
	mgs->target( textTransform_ );

	textPosition_ = mgs;
	taskText_->addShader( "transform", &*textPosition_ );

	taskAttachment_ = new GuiAttachment();
	taskAttachment_->component( taskText_ );
}


WorldEditorProgressBar::~WorldEditorProgressBar()
{
	BW_GUARD;

	if ( inited_ )
		fini();
}


void WorldEditorProgressBar::fini()
{
	BW_GUARD;

	if ( !inited_ )
		return;
		
	Py_DECREF(textPosition_.getObject());
	Py_DECREF(textTransform_.getObject());
	Py_DECREF(taskAttachment_.getObject());
	Py_DECREF(taskText_.getObject());

	taskAttachment_->component( NULL );
	taskAttachment_ = NULL;	
	taskText_ = NULL;	
	taskNode_ = NULL;
	
	escapable( false );
	
	inited_ = false;

	SuperModelProgressDisplay::fini();
}


void WorldEditorProgressBar::drawOther(float dTime)
{
	BW_GUARD;

	if ( !inited_ )
		return;

	ProgressDisplay::ProgressNode& pn = tasks_[tasks_.size()-1];
	taskText_->label( bw_utf8tow(pn.name) );
	taskText_->update( dTime, Moo::rc().screenWidth(), Moo::rc().screenHeight() );
	taskText_->applyShaders( dTime );
	taskAttachment_->draw( taskNode_->worldTransform(), 0 );

	if ( escapable_ )
	{
		escapeText_->update( dTime, Moo::rc().screenWidth(), Moo::rc().screenHeight() );
		escapeText_->draw(true);
	}
}


void WorldEditorProgressBar::escapable( bool escape )
{
	BW_GUARD;

	if ( !inited_ )
		return;

	escapable_ = escape;
	if ( escapable_ && !escapeText_.hasObject() )
	{
		escapeText_ = new TextGUIComponent(  FontManager::instance().getCachedFont("default_medium.font")  );
		escapeText_->filterType( SimpleGUIComponent::FT_LINEAR );
		escapeText_->label( Localise(L"WORLDEDITOR/WORLDEDITOR/BIGBANG/BIG_BANG_PROGRESS_BAR/ESCAPE_SAVE") );
		escapeText_->colour( 0xffC76535 );//199,101,53
		escapeText_->position( Vector3( 0.f, 0.5f, 1.f ) );
		SimpleGUI::instance().addSimpleComponent( *escapeText_ );
	}
	else if ( !escapable_ && escapeText_.hasObject() )
	{
		SimpleGUI::instance().removeSimpleComponent( *escapeText_ );
		Py_DECREF(escapeText_.getObject());
		escapeText_ = NULL;
	}
}


void WorldEditorProgressBar::setLabel( const std::string& label )
{
	BW_GUARD;

	escapable( true );
	escapeText_->slimLabel( label );
}
