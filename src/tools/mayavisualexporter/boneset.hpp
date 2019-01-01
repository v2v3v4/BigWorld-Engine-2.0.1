/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __boneset_hpp__
#define __boneset_hpp__

class BoneSet
{
public:
	void addBone( std::string bone );

	std::map<std::string, uint32> indexes;
	std::vector<std::string> bones;
};

#endif // __boneset_hpp__