/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "resource.h"
#include "gui/vector_generator_custodian.hpp"
#include "resmgr/string_provider.hpp"

using namespace std;

string VecGenGUIStrFromID(string const &str)
{
	BW_GUARD;

    if (str == PointVectorGenerator::nameID_.c_str())
        return LocaliseUTF8(L"`RCS_IDS_POINT");
    else if (str == LineVectorGenerator::nameID_.c_str())
        return LocaliseUTF8(L"`RCS_IDS_LINE");
    else if (str == CylinderVectorGenerator::nameID_.c_str())
        return LocaliseUTF8(L"`RCS_IDS_CYLINDER");
    else if (str == SphereVectorGenerator::nameID_.c_str())
        return LocaliseUTF8(L"`RCS_IDS_SPHERE");
    else if (str == BoxVectorGenerator::nameID_.c_str())
        return LocaliseUTF8(L"`RCS_IDS_BOX");
    else
        return string();
}

string VecGenIDFromGuiStr(string const &str)
{
	BW_GUARD;

    if (str == LocaliseUTF8(L"`RCS_IDS_POINT"))
        return PointVectorGenerator::nameID_.c_str();
    else if (str == LocaliseUTF8(L"`RCS_IDS_LINE"))
        return LineVectorGenerator::nameID_.c_str();
    else if (str == LocaliseUTF8(L"`RCS_IDS_CYLINDER"))
        return CylinderVectorGenerator::nameID_.c_str();
    else if (str == LocaliseUTF8(L"`RCS_IDS_SPHERE"))
        return SphereVectorGenerator::nameID_.c_str();
    else if (str == LocaliseUTF8(L"`RCS_IDS_BOX"))
        return BoxVectorGenerator::nameID_.c_str();
    else
        return string();
}
