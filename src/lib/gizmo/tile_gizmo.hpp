/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TILE_GIZMO_HPP
#define TILE_GIZMO_HPP


#include "gizmo/gizmo_manager.hpp"
#include "gizmo/scale_gizmo.hpp"
#include "gizmo/rotation_gizmo.hpp"
#include "input/input.hpp"
#include "moo/managed_texture.hpp"


class TileGizmo : public Gizmo, public Aligned
{
public:
	enum DrawOptions
	{
		DRAW_SCALE		=  1,
		DRAW_ROTATION	=  2,
		DRAW_GRID		=  4,
		DRAW_TEXTURE	=  8,
		DRAW_FORCED		= 16,

		DRAW_ALL		= DRAW_SCALE | DRAW_ROTATION | DRAW_GRID | DRAW_TEXTURE 
	};

	explicit TileGizmo
	(
		MatrixProxyPtr		matrixProxy,
		uint32				enableModifier	= MODIFIER_SHIFT,
		uint32				disableModifier	= MODIFIER_CTRL | MODIFIER_ALT
	);
	~TileGizmo();

	/*virtual*/ bool draw(bool force);

	/*virtual*/ bool intersects( Vector3 const &origin,
							Vector3 const &direction, float &t, bool force );

	/*virtual*/ void click(Vector3 const &origin, Vector3 const &direction);
	/*virtual*/ void rollOver(Vector3 const &origin, Vector3 const &direction);
	/*virtual*/ Matrix objectTransform() const;

	Moo::BaseTexturePtr texture() const;
	void texture(Moo::BaseTexturePtr texture);

	uint32 drawOptions() const;
	void drawOptions(uint32 options);

	uint8 opacity() const;
	void opacity(uint8 o);

protected:
	uint32 numTiles( float scale ) const;
	float drawTiles( float scale ) const;

	void extractScale(Matrix &m, float &sx, float &sy, float &sz) const;

	void drawGrid(Matrix const &m);
	void drawTexture(Matrix const &m);

private:
	TileGizmo(TileGizmo const &);				// not allowed
	TileGizmo &operator=(TileGizmo const &);	// not allowed

private:
	MatrixProxyPtr				matrixProxy_;
	uint32						enableModifier_;
	uint32						disableModifier_;
	RotationGizmoPtr			rotationGizmo_;
	ScaleGizmoPtr				scaleGizmo_;
	FloatProxyPtr				uniformScaleProxy_;
	GizmoPtr					uniformScaleGizmo_;
	Moo::BaseTexturePtr			texture_;
	uint32						drawOptions_;
	uint8						opacity_;
	float						scaledMinGridSize_;
};

typedef SmartPointer<TileGizmo>	TileGizmoPtr;

#endif // TILE_GIZMO_HPP
