/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOO_IMAGE_HPP
#define MOO_IMAGE_HPP

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stdmf.hpp"
#include "cstdmf/resource_counters.hpp"
#include "cstdmf/guard.hpp"
#include "resmgr/binary_block.hpp"
#include "math/mathdef.hpp"

#include <string.h>


namespace Moo
{
    /**
     *  This class is a general in-memory image class.  You can also wrap
     *  raw pointers as an image (e.g. DirectX textures).
     */
    template<typename TYPE>
    class Image : public ReferenceCount
    {
    public:
        typedef TYPE        PixelType;

		explicit Image(const std::string& allocator = "unknown image");
        Image(uint32 w, uint32 h, const std::string& allocator = "unknown image");
        Image
		(
			uint32				w, 
			uint32				h, 
			PixelType			const &value, 
			std::string			const &allocator	= "unknown image"
		);
        Image
        (
            uint32				w, 
            uint32				h, 
            PixelType			*pixels, 
            bool				owns        		= true,
            size_t				stride      		= 0,
            bool				flipped     		= false,
			std::string			const &allocator	= "unknown image"
        );
        Image(Image const &other);
        ~Image();

        Image &operator=(Image const &other);

        bool isEmpty() const;
        void clear();
        void fill(PixelType const &value);

        uint32 width() const;
        uint32 height() const;
		bool contains(int x, int y) const;

        PixelType *const getRow(uint32 y) const;
        PixelType *getRow(uint32 y);

        PixelType get(int x, int y) const;
		void set(int x, int y, PixelType const &value);
		PixelType getBilinear(float x, float y) const;
        PixelType getBicubic(float x, float y) const;
        double getBicubic(double x, double y) const;        

        void resize(uint32 w, uint32 h);
        void resize(uint32 w, uint32 h, PixelType const &value);
        void resize
        (
            uint32          w, 
            uint32          h, 
            PixelType       *pixels, 
            bool            owns,
            size_t          stride      = 0,
            bool            flipped     = false
        );

        void 
        blit
        (
            Image           const &other, 
            uint32          srcX         = 0, 
            uint32          srcY         = 0, 
            uint32          srcW         = (uint32)-1, 
            uint32          srcH         = (uint32)-1,
            int32           dstX         = 0,
            int32           dstY         = 0
        );
        void flip(bool flipX);
        void rotate(bool clockwise);

        void toCSV(std::string const &filename) const;

		void copyTo(PixelType* contiguousMemoryBuffer) const;
		void copyFrom(PixelType const* contiguousMemoryBuffer);

        size_t rawRowSize() const;
		size_t rawDataSize() const;

		bool isUpsideDown() const;

        bool operator==(Image const &other) const;
        bool operator!=(Image const &other) const;

        bool operator==(PixelType const &value) const;
        bool operator!=(PixelType const &value) const;

		std::string allocator() const;

    protected:
        virtual bool createBuffer
        (
            uint32      w, 
            uint32      h,
            PixelType   *&buffer,
            bool        &owns,
            size_t      &stride,
            bool        &flipped
        );

        void init
        (
            uint32      w, 
            uint32      h, 
            PixelType   *pixels, 
            bool        owns, 
            size_t      stride, 
            bool        flipped
        );

		void initInternal
		(
			uint32      w, 
			uint32      h, 
			PixelType   *pixels, 
			bool        owns, 
			size_t      stride, 
			bool        flipped
		);

        void createLines
        (
            uint32          w, 
            uint32          h, 
            PixelType       *pixels,
            size_t          stride,
            bool            flipped
        );

        void copy(Image const &other);
        void destroy();

        float positive(float x) const;
        double positive(double x) const;

        float bicubicKernel(float x) const;
        double bicubicKernel(double x) const;

    private:
        PixelType       *pixels_;
        PixelType       **lines_;
        uint32			width_;
        uint32			height_;
        bool            owns_;
        size_t          stride_;

#if ENABLE_RESOURCE_COUNTERS
		std::string		allocator_;
#endif
    };
} // namespace Moo


#include "moo/image.ipp"


#endif // MOO_IMAGE_HPP
