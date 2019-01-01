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

#include "line_helper.hpp"
#include "moo/material.hpp"
#include "math/boundbox.hpp"

// -----------------------------------------------------------------------------
// Section: Construction/Destruction
// -----------------------------------------------------------------------------

LineHelper::LineHelper() :
	immediateLines_( D3DPT_LINELIST ),
	immediateLinesScreenSpace_( D3DPT_LINELIST )
{
}

LineHelper::~LineHelper() 
{
	// deallocate world space meshes
	for ( MeshListWS::iterator it = bufferedLines_.begin(); 
			it != bufferedLines_.end(); it++ )
	{
		delete *it;
	}
}

LineHelper& LineHelper::instance()
{
	static LineHelper s_instance;
	return s_instance;
}

// -----------------------------------------------------------------------------
// Section: Public methods
// -----------------------------------------------------------------------------

/**
 *	This method adds a world space line to the list and potentially draws
 *	it if the buffer has reached its limit.
 *	@param start 	The start of the line
 *	@param end 		The end of the line
 *	@param colour	The colour of the line as a 32 bit ARGB value
 *	@param life		The amount of frames to draw this line.
 */
void LineHelper::drawLine( const Vector3& start, const Vector3& end, 
	Moo::PackedColour colour, uint32 life )
{
	// we may build a mesh anew or render to the immediate mesh
	MeshWS* meshPtr = this->getMeshForLine( life );

	// create start vert
	Moo::VertexXYZL vert;
	vert.pos_		= start;
	vert.colour_	= colour;

	// add start
	meshPtr->push_back( vert );
	
	// add end
	vert.pos_ = end;
	meshPtr->push_back( vert );
}

/**
 *	This method adds a world space line to the list and potentially draws
 *	it if the buffer has reached its limit.
 *	@param start the start of the line
 *	@param end the end of the line
 *	@param colour the colour of the line as a 32 bit ARGB value
 */
void LineHelper::drawLineScreenSpace( const Vector4& start, const Vector4& end, 
	Moo::PackedColour colour )
{
	Moo::VertexTL vert;
	vert.pos_ = start;
	vert.colour_ = colour;
	immediateLinesScreenSpace_.push_back( vert );
	vert.pos_ = end;
	immediateLinesScreenSpace_.push_back( vert );

    if (immediateLinesScreenSpace_.size() > POINT_LIMIT)
		purgeScreenSpace();
}

void LineHelper::drawBoundingBox( const BoundingBox& bb, const Matrix& world,
	Moo::PackedColour colour )
{
	Vector3 diff = bb.maxBounds() - bb.minBounds();

	Vector3 corner = world.applyPoint( bb.minBounds() );
	Vector3 xAxis = world[0] * diff.x;
	Vector3 yAxis = world[1] * diff.y;
	Vector3 zAxis = world[2] * diff.z;

	static Vector3 points[8];
	points[0] = corner;
	points[1] = points[0] + zAxis;
	points[2] = points[1] + xAxis;
	points[3] = points[0] + xAxis;
	points[4] = corner + yAxis;
	points[5] = points[4] + zAxis;
	points[6] = points[5] + xAxis;
	points[7] = points[4] + xAxis;

	for (uint32 i = 0; i < 4; i++)
	{
		drawLine( points[i], points[i + 4], colour );
		uint32 ii = (i + 1) % 4;
		drawLine( points[i], points[ii], colour );
		drawLine( points[i + 4], points[ii + 4], colour );
	}
}

// draw a triangle
void LineHelper::drawTriangle(	const Vector3& v0, const Vector3& v1, 
								const Vector3& v2, 
								Moo::PackedColour	colour,
								uint32				life )
{
	Vector3 points[] = { v0, v1, v2, v0 };

	drawLineStrip( points, ARRAY_SIZE(points), colour, life );
}

// draw a strip of lines 
void LineHelper::drawLineStrip( const Vector3* points, uint32 length, 
								Moo::PackedColour colour,
								uint32 life )
{
	if ( length >= 2 )
	{
		// we may build a mesh anew or render to the immediate mesh
		MeshWS* meshPtr = getMeshForLine( life );

		// create start vert
		Moo::VertexXYZL vert;
		vert.colour_	= colour;

		// add each point
		meshPtr->reserve( length * 2 );
		for ( uint32 i = 1; i < length; i++ )
		{
			// p1
			vert.pos_ = points[i-1];
			meshPtr->push_back( vert );

			// p2
			vert.pos_ = points[i];
			meshPtr->push_back( vert );
		}
	}
}

/**
 *	This method purges the line buffers drawing them to the screen.
 */
void LineHelper::purge() 
{ 
	purgeScreenSpace(); 
	purgeWorldSpace(); 
}

// -----------------------------------------------------------------------------
// Section: Drawing
// -----------------------------------------------------------------------------

/*
 *	This method draws the screen space lines
 */
void LineHelper::purgeScreenSpace()
{
	if (immediateLinesScreenSpace_.size())
	{
		Moo::Material::setVertexColour();
		immediateLinesScreenSpace_.draw();
		immediateLinesScreenSpace_.clear();
	}
}

/*
 *	This method draws the world space lines
 */
void LineHelper::purgeWorldSpace()
{
	// set render states
	Moo::Material::setVertexColour();
	Moo::rc().device()->SetTransform( D3DTS_WORLD, &Matrix::identity );
	Moo::rc().device()->SetTransform( D3DTS_VIEW, &Moo::rc().view() );
	Moo::rc().device()->SetTransform( D3DTS_PROJECTION, &Moo::rc().projection() );
	
	// render immediate line entries
	immediateLines_.draw();
	immediateLines_.clear();

	// render all buffered line entries and decrease lifetime
	MeshListWS::iterator it;
	for ( it = bufferedLines_.begin(); 
		it != bufferedLines_.end(); it++ )
	{
		(*it)->mesh_.draw();
		(*it)->life_--;
	}

	// cull dead line entries
	it = bufferedLines_.begin();
	while ( it != bufferedLines_.end() )
	{
		if ( (*it)->life_ == 0 )
		{
			delete *it;
			it = bufferedLines_.erase( it );
		}
		else
		{
			it++;
		}
	}
}

LineHelper::MeshWS* LineHelper::getMeshForLine( uint32 life )
{
	// we may build a mesh anew or render to the immediate mesh
	MeshWS* meshPtr = NULL;

	if ( life == 0 )
	{
		// set the pointer to our immediate mesh
		meshPtr = &immediateLines_;
	}
	else
	{	
		// create a line entry
		LineEntryWS* newLine = new LineEntryWS( life );

		// add line entry to list - list owns it now.
		bufferedLines_.push_back( newLine );

		// set the pointer to our new mesh
		meshPtr = &newLine->mesh_;
	}

	return meshPtr;
}

// line_helper.cpp
