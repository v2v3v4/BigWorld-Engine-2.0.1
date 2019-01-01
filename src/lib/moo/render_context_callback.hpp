/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RENDER_CONTEXT_CALLBACK_HPP
#define RENDER_CONTEXT_CALLBACK_HPP


namespace Moo
{


/**
 *	This class allows controlled release of resources allocated by derived
 *	classes.
 */
class RenderContextCallback
{
public:
	typedef std::vector< RenderContextCallback * > Callbacks;

	/**
	 *	This method must be implemented in derived classes, and should release
	 *	all dynamic resources it has gathered.
	 */
	virtual void renderContextFini() = 0;


	RenderContextCallback();

	static void fini();

private:
	static bool s_finalised_;
	static Callbacks s_callbacks_;
};


}

#endif // RENDER_CONTEXT_CALLBACK_HPP
