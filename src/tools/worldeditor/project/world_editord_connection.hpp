/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WORLD_EDITORD_CONNECTION_HPP
#define WORLD_EDITORD_CONNECTION_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


#include "worldeditor/project/grid_coord.hpp"
#include "network/endpoint.hpp"
#include <string>
#include <set>


namespace BWLock
{

/*
 *	This enum is used to describe the status of each
 *	grid in the space
 */
enum GridStatus
{
	// this grid is not locked by anyone
	GS_NOT_LOCKED = 0,
	// this grid is locked by me, but not editable
	GS_LOCKED_BY_ME,
	// this grid is locked by sb. else
	GS_LOCKED_BY_OTHERS,
	// this grid is locked by me and editable
	GS_WRITABLE_BY_ME,
	// count of grid status
	GS_MAX
};

struct Rect
{
	short left_;
	short top_;
	short right_;
	short bottom_;

	template<typename T>
	Rect( T left, T top, T right, T bottom )
		: left_( (short)left ), top_( (short)top ), right_( (short)right ), bottom_( (short)bottom )
	{}
	Rect(){}
	bool in( int x, int y ) const
	{
		return x >= left_ && x <= right_ && y >= top_ && y <= bottom_;
	}
	bool intersect( const Rect& that ) const
	{
		if( in( that.left_, that.top_ ) || in( that.right_, that.top_ )
			|| in( that.left_, that.bottom_ ) || in( that.right_, that.bottom_ ) )
			return true;
		return that.in( left_, top_ ) || that.in( right_, top_ )
			|| that.in( left_, bottom_ ) || that.in( right_, bottom_ );
	}
	bool operator<( const Rect& that ) const
	{
		if( left_ < that.left_ )
			return true;
		else if( left_ == that.left_ )
		{
			if( top_ < that.top_ )
				return true;
			else if( top_ == that.top_ )
			{
				if( right_ < that.right_ )
					return true;
				else if( right_ == that.right_ )
				{
					if( bottom_ < that.bottom_ )
						return true;
				}
			}
		}
		return false;
	}
};

inline bool operator ==( const Rect& r1, const Rect& r2 )
{
	return r1.left_ == r2.left_ && r1.top_ == r2.top_ && r1.right_ == r2.right_
		&& r1.bottom_ == r2.bottom_;
}

struct Lock
{
	Rect rect_;
	std::string username_;
	std::string desc_;
	float time_;
};

struct Computer
{
	std::string name_;
	std::vector<Lock> locks_;
};

typedef std::vector<std::pair<std::string,std::string> > GridInfo;

struct Command;

class WorldEditordConnection
{
public:
	class Notification
	{
	public:
		virtual ~Notification(){}
		virtual void changed() = 0;
	};

	WorldEditordConnection();

	void registerNotification( Notification* n );
	void unregisterNotification( Notification* n );
	void notify() const;

	bool enabled() const	{	return enabled_;	}
	bool init( const std::string& hoststr, const std::string& username, int xExtent, int zExtent );
	bool connect();
	bool changeSpace( std::string newSpace );
	void disconnect();
	bool connected() const;

	int xExtent() const	{	return	xExtent_;	}
	int zExtent() const	{	return	zExtent_;	}

	void linkPoint( int16 oldLeft, int16 oldTop, int16 newLeft, int16 newTop );

	bool lock( GridRect rect, const std::string description );
	void unlock( Rect rect, const std::string description );

	bool isWritableByMe( int16 x, int16 z ) const;
	bool isLockedByMe( int16 x, int16 z ) const;
	bool isLockedByOthers( int16 x, int16 z ) const;
	bool isSameLock( int16 x1, int16 z1, int16 x2, int16 z2 ) const;
	bool isAllLocked() const;

	GridInfo getGridInformation( int16 x, int16 z ) const;

	std::set<Rect> getLockRects( int16 x, int16 z ) const;

	bool tick();// return true => the lock rects has been updated

	std::vector<unsigned char> getLockData( int minX, int minY, unsigned int gridWidth, unsigned int gridHeight );

	std::string host() const;

	void addCommentary( const std::wstring& msg, bool isCritical );
	void addCommentary( const std::string& msg, bool isCritical );
private:
	std::set<Notification*> notifications_;
	// private functions
	std::set<Rect> getLockRectsNoLink( int16 x, int16 z ) const;
	void rebuildGridStatus();

	bool waitingForCommandReply_;
	bool enabled_;
	std::string self_;
	std::string host_;
	uint16 port_;
	std::string lockspace_;
	std::string username_;

	Endpoint ep_;
	void sendCommand( const Command* command );
	std::vector<unsigned char> recvCommand();
	std::vector<unsigned char> getReply( unsigned char command, bool processInternalCommand = false );
	void processReply( unsigned char command );
	void processInternalCommand( const std::vector<unsigned char>& command );
	bool available();
	bool connected_;
	std::vector<Computer> computers_;
	typedef std::pair<int16,int16> Point;
	typedef std::pair<Point,Point> LinkPoint;
	std::vector<LinkPoint> linkPoints_;
	int xExtent_;
	int zExtent_;

	int xMin_;
	int zMin_;
	int xMax_;
	int zMax_;

	std::vector<GridStatus> gridStatus_;
};

};

#endif // WORLD_EDITORD_CONNECTION_HPP
