/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WAIT_ANIM_CTRL_HPP
#define WAIT_ANIM_CTRL_HPP

namespace controls
{

class WaitAnim : public CAnimateCtrl
{
public:
	static const int USE_DEFAULT_MILLIS = -1;

	WaitAnim();

	bool init( int resID, int width, int height, int maxFPS, int numFrames );

	void update();

	void show();
	void hide( int waitMillis = USE_DEFAULT_MILLIS );

	int width() const { return width_; }
	int height() const { return height_; }

private:
	bool inited_;
	int width_;
	int height_;
	int maxFPS_;
	int numFrames_;
	bool working_;
	bool workingStopped_;
	uint64 workingStoppedTimer_;
	int hideWaitMillis_;
	int frame_;
	uint64 frameTimestamp_;
};

} // namespace controls

#endif WAIT_ANIM_CTRL_HPP
