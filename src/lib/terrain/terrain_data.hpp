/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_DATA
#define TERRAIN_DATA

namespace Terrain
{
const float     BLOCK_SIZE_METRES		= 100.0f;
const float		SUB_BLOCK_SIZE_METRES	= BLOCK_SIZE_METRES / 2.0f;

enum HeightMapCompression
{
	COMPRESS_RAW  = 0,
	COMPRESS_LAST = 0xffffffff
};

enum NormalMapQuality
{
	NMQ_NICE,	// Best looking, but slower to generate
	NMQ_FAST	// Fast to generate, but not as good quality (good for preview)
};

struct HeightMapHeader
{
	enum HMHVersions
	{
		VERSION_ABS_FLOAT		= 1,	// absolute float values
		VERSION_REL_UINT16		= 2,	// relative uint16 values
		VERSION_REL_UINT16_PNG	= 3,	// as above, compressed via png.
		VERSION_ABS_QFLOAT		= 4		// absolute quantised float values
	};

	uint32					magic_;		// Should be "hmp\0"
	uint32					width_;
	uint32					height_;
	HeightMapCompression	compression_;
	uint32					version_;	// format version
	float					minHeight_;
	float					maxHeight_;
	uint32					pad_;
	// static const uint32 MAGIC = '\0pmh';
	static const uint32 MAGIC = 0x00706d68;
};


struct BlendHeader
{
	enum BHMVersions
	{
		VERSION_RAW_BLENDS	= 1,
		VERSION_PNG_BLENDS	= 2
	};
	
	uint32	magic_;		// Should be "bld\0"
	uint32	width_;
	uint32	height_;
	uint32	bpp_;		// Reserved for future use.
	Vector4	uProjection_;
	Vector4	vProjection_;
	uint32	version_;	// format version
	uint32	pad_[3];
	// static const uint32 MAGIC = '\0dlb';
	static const uint32 MAGIC = 0x00646c62;
};

struct ShadowHeader
{
	uint32	magic_; // Should be "shd\0"
	uint32	width_;
	uint32	height_;
	uint32	bpp_;
	uint32	version_;
	uint32	pad_[3];
	// static const uint32 MAGIC = '\0dhs';
	static const uint32 MAGIC = 0x00646873;
};

struct HolesHeader
{
	uint32 magic_;
	uint32 width_;
	uint32 height_;
	uint32 version_;
	// static const uint32 MAGIC = '\0loh';
	static const uint32 MAGIC = 0x006c6f68;
};

struct VertexLODHeader
{
	// static const uint32 MAGIC = '\0rev';
	static const uint32 MAGIC = 0x00726576;

	enum VLHVersions
	{
		VERSION_RAW_VERTICES = 1,	// regular vertices
		VERSION_ZIP_VERTICES = 2	// zip compressed vertices
	};

	uint32 magic_;
	uint32 version_;
	uint32 gridSize_;
};

struct TerrainBlock1Header
{
	uint32		version_;
	uint32		heightMapWidth_;
	uint32		heightMapHeight_;
	float		spacing_;
	uint32		nTextures_;
	uint32		textureNameSize_;
	uint32		detailWidth_;
	uint32		detailHeight_;
	uint32		padding_[64 - 8];
};

struct TerrainNormalMapHeader
{
	static const uint32 VERSION_16_BIT_PNG	= 1;
	uint32 magic_; // Should be "nrm\0"
	uint32 version_;
	uint32 pad_[2];

	// static const uint32 MAGIC = '\0mrn';
	static const uint32 MAGIC = 0x006d726e;
};

struct DominantTextureMapHeader
{
	static const uint32 VERSION_ZIP	= 1;
	uint32 magic_;
	uint32 version_;
	uint32 numTextures_;
	uint32 textureNameSize_;
	uint32 width_;
	uint32 height_;
	uint32 pad_[2];
	// static const uint32 MAGIC = '\0tam';
	static const uint32 MAGIC = 0x0074616d;
};

};

#endif
