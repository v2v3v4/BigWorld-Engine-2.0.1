/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef IMPORT_CODEC_TERRAGEN_HPP
#define IMPORT_CODEC_TERRAGEN_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/import/import_codec.hpp"


/**
 *  This codec reads/writes terragen data.
 */
class ImportCodecTerragen : public ImportCodec
{
public:
    /*virtual*/ bool canHandleFile(std::string const &filename);

    /*virtual*/ LoadResult load
    (
        std::string         const &filename, 
        ImportImage			&image,
        float               *left           = NULL,
        float               *top            = NULL,
        float               *right          = NULL,
        float               *bottom         = NULL,
		bool				*absolute		= NULL,
        bool                loadConfigDlg   = false
    );

    /*virtual*/ bool save
    (
        std::string         const &filename, 
        ImportImage			const &image,
        float               *left           = NULL,
        float               *top            = NULL,
        float               *right          = NULL,
        float               *bottom         = NULL,
		bool				*absolute		= NULL,
		float				*minVal			= NULL,
		float				*maxVal			= NULL
    );

    /*virtual*/ bool canLoad() const;

    /*virtual*/ bool canSave() const;
};


#endif // _IMPORT_CODEC_TERRAGEN_HPP
