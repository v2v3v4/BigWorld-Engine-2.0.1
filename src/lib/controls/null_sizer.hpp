/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONTROLS_NULL_SIZER_HPP
#define CONTROLS_NULL_SIZER_HPP

#include "sizer.hpp"

namespace controls
{
    /**
     *  This Sizer represents a blank area.
     */
    class NullSizer : public Sizer
    {
    public:
        NullSizer(uint32 minWidth, uint32 minHeight);

        /*virtual*/ void onSize(CRect const &extents);
        /*virtual*/ CSize minimumSize() const;
        /*virtual*/ void draw(CDC *dc);

    private:
        uint32					minWidth_;
        uint32		            minHeight_;
    };
}

#endif // CONTROLS_NULL_SIZER_HPP
