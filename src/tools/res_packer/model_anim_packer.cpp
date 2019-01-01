/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/



#include "packers.hpp"
#include "packer_helper.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/packed_section.hpp"
#include "model_anim_packer.hpp"
#ifndef MF_SERVER
#include "model/model.hpp"
#include "moo/interpolated_animation_channel.hpp"
#endif // MF_SERVER


IMPLEMENT_PACKER( ModelAnimPacker )

int ModelAnimPacker_token = 0;


bool ModelAnimPacker::prepare( const std::string& src, const std::string& dst )
{
	std::string ext = BWResource::getExtension( src );
	if (bw_stricmp( ext.c_str(), "model" ) == 0)
		type_ = MODEL;
	else if (bw_stricmp( ext.c_str(), "animation" ) == 0)
		type_ = ANIMATION;
	else if (bw_stricmp( ext.c_str(), "anca" ) == 0)
		type_ = ANCA;
	else
		return false;

	src_ = src;
	dst_ = dst;

	return true;
}

bool ModelAnimPacker::print()
{
	if ( src_.empty() )
	{
		printf( "Error: ModelAnimPacker Not initialised properly\n" );
		return false;
	}

	if ( type_ == ANIMATION )
		printf( "AnimationFile: %s\n", src_.c_str() );
	else if ( type_ == ANCA )
		printf( "CompressedAnimationFile: %s\n", src_.c_str() );
	else
		printf( "ModelFile: %s\n", src_.c_str() );
	return true;
}

bool ModelAnimPacker::pack()
{
#ifndef MF_SERVER
	if ( src_.empty() || dst_.empty() )
	{
		printf( "Error: ModelAnimPacker Not initialised properly\n" );
		return false;
	}

	if ( type_ != MODEL )
		return true; // skip .animation and .anca files

	std::string dissolved = BWResolver::dissolveFilename( src_ );
	// load the datasection to find out if the model is nodefull.
	DataSectionPtr ds = BWResource::openSection( dissolved );
	if ( !ds )
	{
		printf( "Could not open model file %s\n", dissolved.c_str() );
		return false;
	}

	// find out if it's nodefull and has animation
	if ( ds->openSection( "nodefullVisual" ) )
	{
		// load the model so it generates it's anca file, if any

		Moo::InterpolatedAnimationChannel::inhibitCompression( false );

		ModelPtr mp = Model::get( dissolved );
		if ( !mp || !mp->valid() )
		{
			printf( "Couldn't load model %s\n", dissolved.c_str() );
			return false;
		}
	}

	// check to see if a new anca file was created
	if ( !PackerHelper::fileExists( BWResource::removeExtension( dst_ ) + ".anca" ) )
	{
		// no new anca file was created. try to find out if an up-to-date file
		// exists in the src_ folder
		if ( PackerHelper::fileExists( BWResource::removeExtension( src_ ) + ".anca" ) )
		{
			// the anca file in the src_ folder was up-to-date, and loading the
			// model didn't create it in the dst_ folder, so copy it to dst_
			if ( !PackerHelper::copyFile(
					BWResource::removeExtension( src_ ) + ".anca",
					BWResource::removeExtension( dst_ ) + ".anca" ) )
				return false;
		}
		else
		{
			// there isn't an anca file in the src_ folder either, so assume
			// the model doesn't have animations (no error returned)
		}
	}

	// copy and pack the model file to the destination
	std::vector< std::string > stripSections;
	stripSections.push_back( "metaData" );
	return PackedSection::convert( src_, dst_, &stripSections );

#else // MF_SERVER

	// just skip model and animation files for the server
	return true;

#endif // MF_SERVER
}
