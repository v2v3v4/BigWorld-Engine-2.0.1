/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FINALE_APP_HPP
#define FINALE_APP_HPP


#include "cstdmf/main_loop_task.hpp"
#include "input/input.hpp"


class ProgressTask;


/**
 *	Finale task
 */
class FinaleApp : public MainLoopTask, InputHandler
{
public:
	FinaleApp();
	~FinaleApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void draw();

	bool handleKeyEvent(const KeyEvent & event);

public:
	static FinaleApp instance;

private:
	void runPreloads( ProgressTask & preloadTask );
	void findEffects( const std::string& folderName,
						std::vector<std::string>& ret );
	bool cacheEffects( const std::vector<std::string>& effects );

	float				dTime_;
	bool				cancelPreloads_;
	bool				disablePreloads_;
	bool				donePreloads_;

	std::vector< Moo::BaseTexturePtr > preloadedTextures_;
	std::vector< DataSectionPtr > preloadedDataSections_;
	std::vector< PyObject* > preloadedPyObjects_;
	std::vector< Moo::ManagedEffectPtr > preloadedEffects_;
};


#endif // FINALE_APP_HPP


// finale_app.hpp
