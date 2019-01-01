/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCRIPTER_HPP
#define SCRIPTER_HPP

#include "resmgr/datasection.hpp"

class Scripter
{
public:
	static bool init( DataSectionPtr pDataSection );
	static void fini();
	static bool update();
};

#endif // SCRIPTER_HPP
