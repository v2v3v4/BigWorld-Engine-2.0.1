/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef HANDLE_POOL_HPP
#define HANDLE_POOL_HPP

#include <stack>

class HandlePool
{
public:
	typedef uint16 Handle;
	static const Handle INVALID_HANDLE = 0xffff;

	HandlePool( uint16 numHandles );
	Handle handleFromId( uint32 id );
	void beginFrame();
	void endFrame();
	void reset();
	uint16 numHandles() const { return numHandles_; }

	struct Info
	{
		Handle handle_;
		bool used_;
	};

	typedef std::map<uint32,Info>	HandleMap;
	HandleMap::iterator begin() { return handleMap_.begin(); }
	HandleMap::iterator end()	{ return handleMap_.end(); }
	
private:
	HandleMap						handleMap_;
	std::stack<Handle>				unusedHandles_;
	uint16		numHandles_;
};
	
#endif // HANDLE_POOL_HPP