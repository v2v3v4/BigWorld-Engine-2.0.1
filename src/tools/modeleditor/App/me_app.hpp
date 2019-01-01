/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ME_APP_HPP
#define ME_APP_HPP

#include "floor.hpp"
#include "mutant.hpp"
#include "lights.hpp"

#include "moo/light_container.hpp"

#include "tools_camera.hpp"

class MeApp
{
public:

	MeApp();
	~MeApp();

	void initCamera();
	
	static MeApp & instance() { ASSERT(s_instance_); return *s_instance_; }
	
	Floor*	floor();
	Mutant*	mutant();
	Lights*	lights();
	ToolsCameraPtr camera();

	Moo::LightContainerPtr blackLight() { return blackLight_; }
	Moo::LightContainerPtr whiteLight() { return whiteLight_; }

	void saveModel();
	void saveModelAs();
	bool canExit( bool quitting );
	void forceClean();
	bool isDirty() const;
private:

	static MeApp *		s_instance_;

	Floor*	floor_;
	Mutant*	mutant_;
	Lights*  lights_;

	Moo::LightContainerPtr blackLight_;
	Moo::LightContainerPtr whiteLight_;

	ToolsCameraPtr camera_;	
};

#endif // ME_APP_HPP