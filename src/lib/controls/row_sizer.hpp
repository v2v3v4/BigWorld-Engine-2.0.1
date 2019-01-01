/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONTROLS_ROW_SIZER_HPP
#define CONTROLS_ROW_SIZER_HPP

#include "sizer.hpp"
#include <vector>

namespace controls
{
    /**
     *  A RowSizer keeps track of a bunch of sizers in either the
     *  vertical or horizontal orientation.
     */
    class RowSizer : public Sizer
    {
    public:
        enum Orientation
        {
            VERTICAL,           // vertically orientated
            HORIZONTAL          // horizontally orientated
        };

        enum SizeUnits
        {
            PIXELS,             // size is pixels
            WEIGHT              // size is a weight
        };

        explicit RowSizer(Orientation orientation);
		RowSizer(Orientation orientation, CWnd *container);
		RowSizer(Orientation orientation, CWnd *parent, uint32 containerID);
        /*virtual*/ ~RowSizer();

        Orientation orientation() const;
        void orientation(Orientation orientation);

        void addChild
        (
            SizerPtr            child,
            uint32				size,
            SizeUnits           sizeUnits
        );
		void addChild(SizerPtr child);

        uint32 edgeGap() const;
        void edgeGap(uint32 gap);

        uint32 childGap() const;
        void childGap(uint32 g);

        /*virtual*/ void onSize(CRect const &extents);
        /*virtual*/ CSize minimumSize() const;
        /*virtual*/ void draw(CDC *dc);

    protected:
        CWnd *window();

    private:
        struct Child
        {
            SizerPtr            sizer_;
            uint32				size_;
            SizeUnits           sizeUnits_;
        };

        CWnd                	*wnd_;
        uint32        			id_;
        Orientation             orientation_;
        uint32					edgeGap_;
        uint32					childGap_;
        std::vector<Child>      children_;
    };
}

#endif // CONTROLS_ROW_SIZER_HPP
