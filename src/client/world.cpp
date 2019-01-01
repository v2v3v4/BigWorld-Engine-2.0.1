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

#pragma warning (disable: 4786)

#include "world.hpp"

#include "physics2/worldtri.hpp"
#include "physics2/worldpoly.hpp"

#include "romp/time_of_day.hpp"

#include "moo/render_context.hpp"



#ifndef CODE_INLINE
#include "world.ipp"
#endif


static int nWorldInstances = 0;

World::World()
{
	++nWorldInstances;
}


World::~World()
{
	if (--nWorldInstances > 0) return;
}




// Debug code: start

static bool g_drawDebugTris = false;

/**
 * @internal
 * Used to register local watchers.
 */
class LocalDebug
{
public:
	LocalDebug()
	{
		MF_WATCH( "debug/collision", g_drawDebugTris );
	}
};

static LocalDebug localDebug;

#include "romp/geometrics.hpp"

VectorNoDestructor< std::pair<WorldTriangle,Moo::Colour> > s_dtris;

void s_dtriadd( const WorldTriangle & wt, uint32 col )
{
	BW_GUARD;
	if (g_drawDebugTris)
		s_dtris.push_back( std::make_pair( wt, Moo::Colour( col ) ) );
}

std::vector< std::pair<WorldPolygon, Moo::Colour> > s_dpolys;

void s_darrowadd( const Vector3 & start, const Vector3 & end, uint32 col )
{
	BW_GUARD;
	if (g_drawDebugTris)
	{
		if ( Vector3(end - start).length() > 0.001f )
		{
			Matrix lookat;
			Vector3 dir(end-start);
			float l = dir.length();
			dir.normalise();

			Vector3 up;
			if (fabsf(dir.y) < 0.95f)
			{
				up = Vector3(0.f,1.f,0.f);
			}
			else
			{
				up = Vector3(0.f,0.f,-1.f);
			}

			Moo::Colour colour(col);

			{
				lookat.lookAt(end, dir, up );				
				lookat.invert();
				float s = 0.03f;
				Vector3 v1( -s, 0.f, -s);
				Vector3 v2(  s, 0.f, -s);
				Vector3 v3(0.f, 0.f,  s);
				s_dtriadd( WorldTriangle( lookat.applyPoint(v1), lookat.applyPoint(v2), lookat.applyPoint(v3) ), colour );
			}

			{
				lookat.lookAt(start, dir, up);
				lookat.invert();
				float s = 0.001f;
				Vector3 v1( -s,  0.f, 0.f);
				Vector3 v2( s,   0.f, 0.f);
				Vector3 v3( 0.f, 0.f,   l);
				s_dtriadd( WorldTriangle( lookat.applyPoint(v1), lookat.applyPoint(v2), lookat.applyPoint(v3) ), colour );
			}
		}
	}
}

void s_dpolyadd( const WorldPolygon & wp, uint32 col )
{
	BW_GUARD;
	if (g_drawDebugTris)
		s_dpolys.push_back( std::make_pair( wp, Moo::Colour( col ) ) );
}

static void s_drawDebugTris()
{
	BW_GUARD;
	if (g_drawDebugTris)
	{
		Moo::rc().push();
		Moo::rc().world( Matrix::identity );

		for (uint i = 0; i < s_dtris.size(); i++)
		{
			WorldTriangle & triangle = s_dtris[i].first;
			Moo::Colour & col = s_dtris[i].second;
			Geometrics::drawLine( triangle.v0(), triangle.v1(), col );
			Geometrics::drawLine( triangle.v1(), triangle.v2(), col );
			Geometrics::drawLine( triangle.v2(), triangle.v0(), col );
		}

		for (uint i = 0; i < s_dpolys.size(); i++)
		{
			WorldPolygon & poly = s_dpolys[i].first;
			Moo::Colour & col = s_dpolys[i].second;

			for (int j = 0; j < int(poly.size()) - 1; j++)
			{
				Geometrics::drawLine( poly[j], poly[j+1], col );
			}

			if (!poly.empty())
			{
				Geometrics::drawLine( poly.back(), poly.front(), col );
			}
		}

		Moo::rc().pop();
	}

	s_dtris.clear();
	s_dpolys.clear();
}
// Debug code: end


/**
 *	This static function draws the saved up debug triangles
 */
void World::drawDebugTriangles()
{
	BW_GUARD;
	s_drawDebugTris();
}



/**
 *	This private static function is used by static functions that would
 *	prefer to work with a world instance.
 */
World & World::instance()
{
	static World	world;
	return world;
}

// world.cpp
