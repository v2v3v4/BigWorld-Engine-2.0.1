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
#include "editor_chunk_bsp_holder.hpp"
#include "moo/moo_math.hpp"
#include "world/world_manager.hpp"
#include "romp/fog_controller.hpp"


EditorChunkBspHolder::Infos EditorChunkBspHolder::infos_;
volatile LONG EditorChunkBspHolder::count_ = 0;

EditorChunkBspHolder::EditorChunkBspHolder()
	: colour_( (float)rand() / (float)RAND_MAX,
				(float)rand() / (float)RAND_MAX,
				(float)rand() / (float)RAND_MAX,
				1.f )
{
	InterlockedIncrement( &count_ );
}

EditorChunkBspHolder::~EditorChunkBspHolder()
{
	if (InterlockedDecrement( &count_ )  == 1)
	{
		infos_.clear();
	}
}

bool EditorChunkBspHolder::bspCreated( const std::string& name ) const
{
	return infos_.find( name ) != infos_.end();
}


void EditorChunkBspHolder::drawBsp( const std::string& name ) const
{
	Infos::const_iterator iter = infos_.find( name );

	if (iter != infos_.end())
	{
		iter->second.vb_.set( 0, 0, sizeof(Moo::VertexXYZL) );

		if (!WorldManager::instance().drawSelection())
		{
			WorldManager::instance().setColourFog( colour_ );
		}

		Moo::rc().drawPrimitive( D3DPT_TRIANGLELIST, 0, iter->second.primitiveCount_ );

		if (!WorldManager::instance().drawSelection())
		{
			FogController::instance().commitFogToDevice();
		}
	}
}


void EditorChunkBspHolder::addBsp( const std::vector<Moo::VertexXYZL>& verts, const std::string& name )
{
	if (!verts.empty())
	{
		Info info;

		info.primitiveCount_ = verts.size() / 3;
		info.vb_.create( verts.size() * sizeof ( Moo::VertexXYZL ), D3DUSAGE_WRITEONLY,
			Moo::VertexXYZL::fvf(), D3DPOOL_MANAGED );
		Moo::VertexLock<Moo::VertexXYZL>( info.vb_ ).fill( &verts[ 0 ],
			verts.size() * sizeof ( Moo::VertexXYZL ) );

		infos_[ name ] = info;
	}
}


void EditorChunkBspHolder::postClone()
{
	colour_ = Moo::Colour( (float)rand() / (float)RAND_MAX,
		(float)rand() / (float)RAND_MAX,
		(float)rand() / (float)RAND_MAX,
		1.f );
}
