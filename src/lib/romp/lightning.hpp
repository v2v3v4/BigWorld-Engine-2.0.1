/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LIGHTNING_HPP
#define LIGHTNING_HPP

class Lightning
{
public:
	void lightningStrike( const Vector3 & top );
	Vector4	decideLightning( float dTime );	
private:
	float conflict_;
};

#endif