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
#include "super_model_progress.hpp"
#include "model/super_model.hpp"
#include "model/super_model_animation.hpp"
#include "moo/visual_channels.hpp"
#include "moo/camera_planes_setter.hpp"

static float lastTime_ = 0.f;

static const float NEAR_PLANE = 0.01f;
static const float FAR_PLANE = 500.0f;


SuperModelProgressDisplay::SuperModelProgressDisplay(
	const std::string& modelName )
{
	lastTime_ = 0.f;

	std::vector<std::string> modelNames;
	modelNames.push_back( modelName );

	superModel_ = new SuperModel( modelNames );

	anim_ = superModel_->getAnimation( "loader2" );
	if ( anim_ )
	{
		anim_->blendRatio = 1.f;
		fashions_.push_back( anim_ );
	}
}

SuperModelProgressDisplay::SuperModelProgressDisplay() :
	superModel_( NULL ),
	anim_( NULL)
{
	
}


SuperModelProgressDisplay::~SuperModelProgressDisplay()
{
	fini();
}

void SuperModelProgressDisplay::fini()
{
	fashions_.clear();
	anim_ = NULL;
	delete superModel_;
	superModel_ = NULL;
}


bool SuperModelProgressDisplay::draw( bool force )
{
	// run the callback
	bool ret = true;
	if (pCallback_)
		ret = (*pCallback_)();
	
	if (!ret)
		return false;

	float percentDone = 0.5f;

	// create a setter for the camera's near and far planes
	Moo::CameraPlanesSetter camPlanes( NEAR_PLANE, FAR_PLANE );

	// now traverse the tree
	int row = 0;
	std::vector<int>	ixStack = roots_;
	
	while( ixStack.size() > 0)
	{
		int i = ixStack.back();
		ixStack.pop_back();

		ProgressNode & node = tasks_[i];

		// draw the bar
		if (node.task != NULL)
		{
			float usedone = node.task->done_;

			if (usedone < 0.f) usedone = 0.f;

			float uselen = node.task->length_;

			if ( uselen == 0.f ) uselen = 1000000.f;
			if ( uselen > 0.f && uselen < usedone ) uselen = usedone;

			percentDone = usedone / uselen;
		}

		// now push on all our children in reverse order
		for (int j = node.children.size()-1; j >= 0; j--)
		{
			ixStack.push_back( node.children[j] );
		}

		row++;
	}

	float aimTime = percentDone * 3.3333f;

	// just draw the scene
	Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1, 0 );
	Moo::rc().beginScene();
	if (Moo::rc().mixedVertexProcessing())
		Moo::rc().device()->SetSoftwareVertexProcessing( TRUE );
	Moo::rc().nextFrame();
	if (Moo::rc().mixedVertexProcessing())
		Moo::rc().device()->SetSoftwareVertexProcessing(TRUE);

	Moo::rc().push();
	Matrix oldView = Moo::rc().view();

	//Matrix transform;
	//transform.setTranslate(Vector3(0,0,20.f));
	Moo::rc().world( Matrix::identity/*transform*/ );
	Moo::rc().view( Matrix::identity );
	Moo::rc().updateViewTransforms();

	if ( anim_ )
		anim_->time = aimTime;

	superModel_->draw( &fashions_ );

	this->drawOther( aimTime - lastTime_ );

	Moo::SortedChannel::draw();

	Moo::rc().pop();
	Moo::rc().view( oldView );

	Moo::rc().endScene();
	Moo::rc().present();

	lastTime_ = aimTime;

	return true;
}