/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef POLYCONT_HPP
#define POLYCONT_HPP

#include <vector>

typedef std::vector< class PolyContainer > PCVector;

class PolyContainer
{
public:
	PolyContainer();
	~PolyContainer();
	PolyContainer(const PolyContainer&);
	PolyContainer& operator=(const PolyContainer&);

	void setVIndices( int a, int b, int c );
	void setTCIndices( int a, int b, int c );

	int getV1( void );
	int getV2( void );
	int getV3( void );

	int getTV1( void );
	int getTV2( void );
	int getTV3( void );

private:
	int v1_, v2_, v3_;
	int tv1_, tv2_, tv3_;
};


#endif
/*polyCont.hpp*/