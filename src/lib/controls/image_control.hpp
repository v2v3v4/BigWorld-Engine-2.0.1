/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef IMAGE_CONTROL_HPP
#define IMAGE_CONTROL_HPP


#include "controls/dib_section8.hpp"
#include "controls/dib_section32.hpp"


namespace controls
{
    /**
    *  This control displays an image inside itself.
    */
	template<typename IMAGETYPE>
    class ImageControl : public CWnd
    {
    public:
		typedef IMAGETYPE		ImageType;

		enum AspectRatio
		{
			FIT_TO_WINDOW,
			PRESERVE,
			PRESERVE_FUZZY
		};

        ImageControl();

        BOOL Create(DWORD style, RECT const &extents, CWnd *parent, unsigned int id = 0);

        BOOL subclass(unsigned int resourceID, CWnd *parent);

        void image(ImageType const &dibSection);
        ImageType const &image() const;
        ImageType &image();

        enum BorderStyle
        {
            BS_NONE,
            BS_BLACKRECT,
            BS_SUNKEN,
            BS_RAISED
        };

        BorderStyle border() const;
        void border(BorderStyle border);

        AspectRatio preserveAspectRatio() const;
        void preserveAspectRatio(AspectRatio preserve);

        COLORREF backgroundColour() const;
        void backgroundColour(COLORREF colour);

        unsigned int borderPadding() const;
        void borderPadding(unsigned int padding);

        void beginSet();
        void endSet();

		void text(std::string const &str);
		std::string text() const;

    protected:
		const static uint32 FUZZY_ASPECT_PIXELS = 4; // fuzziness pixels for fixed aspect ratio

        void OnPaint();

        BOOL OnEraseBkgnd(CDC *dc);

    private:
        ImageType			image_;
        BorderStyle         border_;
        AspectRatio         aspect_;   
        COLORREF            background_;
        unsigned int        padding_;
        size_t              setCount_;
		std::wstring		textNoImage_;
    };


	class ImageControl8 : public ImageControl<DibSection8>
	{
	public:
		typedef ImageControl<DibSection8>		Base;

	protected:
        afx_msg void OnPaint();

        afx_msg BOOL OnEraseBkgnd(CDC *dc);

        DECLARE_MESSAGE_MAP()
	};


	class ImageControl32 : public ImageControl<DibSection32>
	{
	public:
		typedef ImageControl<DibSection32>		Base;

	protected:
        afx_msg void OnPaint();

        afx_msg BOOL OnEraseBkgnd(CDC *dc);

        DECLARE_MESSAGE_MAP()
	};
}


#include "controls/image_control.ipp"


#endif // IMAGE_CONTROL_HPP
