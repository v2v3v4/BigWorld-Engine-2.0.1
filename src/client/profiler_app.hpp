/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROFILER_APP_HPP
#define PROFILER_APP_HPP

#include "cstdmf/main_loop_task.hpp"

/**
 *	Profiler task
 */
class ProfilerApp : public MainLoopTask
{
public:
	ProfilerApp();
	~ProfilerApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void draw();

	void				updateProfiler( float dTime );
	void				updateCamera( float dTime );

	static ProfilerApp	instance;

private:
	void clampAngles( const Vector3& start, Vector3& end );
	void profilerFinishedWithStats();
	typedef std::pair<std::string, float> PairStringFloat;
	typedef std::list<PairStringFloat > ListStringFloat;
	void profilerFinished(const ListStringFloat& sentList);

	float				cpuStall_;

	float				dTime_;
	float				filteredDTime_;
	float				fps_;
	float				filteredFps_;

public:
};


#endif // PROFILER_APP_HPP


// profiler_app.hpp
