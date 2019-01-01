/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef VERTCONT_HPP
#define VERTCONT_HPP

#include <vector>
#include <algorithm>

typedef std::vector< class VertexContainer > VCVector;

class VertexContainer
{
public:
	VertexContainer();
	VertexContainer(int v, int tv);
	~VertexContainer();
	VertexContainer(const VertexContainer&);
	VertexContainer& operator = (const VertexContainer&);
	bool operator == (const VertexContainer&) const;

	int getV(void);
	int getTV(void);

	void setV( int i);
	void setTV( int i);
private:
	int v_;
	int tv_;

};

class UniqueVertices
{
private:
	VCVector vcs_;
public:
	void addVertex( const VertexContainer &vc )
	{
		VCVector::iterator it = std::find( vcs_.begin(), vcs_.end(), vc );
		if( it == vcs_.end() )
			vcs_.push_back( vc );
	}

	int getIndex( const VertexContainer &vc )
	{
		unsigned int ind = 0;
		while( ind < vcs_.size() )
		{
			if( vcs_[ ind ] == vc )
			{
				return int(ind);
			}
			ind++;
		}
		return -1;
	}

	int findVertexIndexFromPositionIndex( int positionIndex, int vertexListIndex = 0 )
	{
		int i;
		for( i = vertexListIndex; unsigned int(i) < vcs_.size(); i++ )
		{
			if( vcs_[ i ].getV() == positionIndex )
			{
				return i;
			}
		}
		return i;
	}

	VCVector& v( void )
	{
		return vcs_;
	}

};

#endif
/*vertcont.hpp*/
