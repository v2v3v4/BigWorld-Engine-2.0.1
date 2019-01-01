/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BUFFERED_GHOST_MESSAGE_HPP
#define BUFFERED_GHOST_MESSAGE_HPP


class BufferedGhostMessage
{
public:
	BufferedGhostMessage( bool isSubsequenceStart,
			bool isSubsequenceEnd ) :
		isSubsequenceStart_( isSubsequenceStart ),
		isSubsequenceEnd_( isSubsequenceEnd )
	{
	}

	virtual void play() = 0;
	virtual bool isCreateGhostMessage() const { return false; }

	bool isSubsequenceStart() const { return isSubsequenceStart_; }
	bool isSubsequenceEnd() const { return isSubsequenceEnd_; }

private:
	bool isSubsequenceStart_;
	bool isSubsequenceEnd_;
};

#endif // BUFFERED_GHOST_MESSAGE_HPP
