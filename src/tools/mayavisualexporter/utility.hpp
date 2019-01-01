/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include <string>
#include <vector>
#include "resmgr/datasection.hpp"
//~ #include "max.h"
#include <maya/MDistance.h>

#include "matrix3.hpp"

template<class T> void swap( T& t1, T& t2 )
{
	T t = t1;
	t1 = t2;
	t2 = t;
}

matrix4<float> normaliseMatrix( const matrix4<float> &m );
bool isMirrored( const matrix3<float> &m );
std::string toLower( const std::string &s );
std::string trimWhitespaces( const std::string &s );
std::string unifySlashes( const std::string &s );
matrix4<float> rearrangeMatrix( const matrix4<float>& m );
bool trailingLeadingWhitespaces( const std::string&s );
float snapValue( float v, float snapSize );
Point3 snapPoint3( const Point3& pt, float snapSize = 0.001f );

template<class T> 
class ConditionalDeleteOnDestruct
{
public:
	ConditionalDeleteOnDestruct( T* object, bool del = true)
	: object_( object ),
	  delete_( del )
	{
	}
	ConditionalDeleteOnDestruct()
	: object_( NULL ),
	  delete_( true )
	{
	}
	~ConditionalDeleteOnDestruct()
	{
		if (object_&&delete_)
		{
			delete object_;
		}
	}
	void operator = ( T* object )
	{
		object_ = object;
	}
	T& operator *(){ return *object_;}
	T* operator ->(){ return object_;};

	bool del() const { return delete_; };
	void del( bool del ){ delete_ = del; };

private:
	T* object_;
	bool delete_;
};
