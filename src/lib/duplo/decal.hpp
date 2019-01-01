/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DECAL_HPP
#define DECAL_HPP

#include "moo/forward_declarations.hpp"
#include "moo/moo_dx.hpp"
#include "math/mathdef.hpp"
#include "math/vector3.hpp"
#include "math/vector2.hpp"
#include "math/matrix.hpp"
#include "cstdmf/main_loop_task.hpp"
#include "moo/device_callback.hpp"
#include "moo/vertex_buffer.hpp"

namespace Moo
{
	class EffectMaterial;
	class BaseTexture;
};

class WorldTriangle;

/**
 *	This class ...
 *
 *	@todo Document this class.
 */
class Decal : public Moo::DeviceCallback
{
public:
	Decal();
	~Decal();

	void add( 
		const Vector3& start, 
		const Vector3& end, 
		float          size, 
		int            type );
	
	void addTriangles(
		const WorldTriangle * triangles, 
		int                   numTriangles, 
		const Matrix        & transform,
		int                   type);

	void clear();
	
	virtual void draw();

	virtual void deleteUnmanagedObjects();
	virtual void createUnmanagedObjects();

private:
#pragma pack(push,1)
	struct DecalVertex
	{
		Vector3	pos_;
		Vector2	uv_;
		static DWORD fvf() { return D3DFVF_XYZ|D3DFVF_TEX1; };
	};
#pragma pack(pop)

	struct ClipVertex
	{
		Vector3 pos_;
		Vector2 uv_;
		Outcode oc_;
		void calcOutcode()
		{
			oc_ = 0;
			if (uv_.x < 0)
				oc_ |= OUTCODE_LEFT;
			else if (uv_.x > 1.f)
				oc_ |= OUTCODE_RIGHT;

			if (uv_.y < 0)
				oc_ |= OUTCODE_TOP;
			else if (uv_.y > 1.f)
				oc_ |= OUTCODE_BOTTOM;
		}
	};

	Moo::EffectMaterialPtr		pMaterial_;

	Moo::VertexBuffer	pVertexBuffer_;

	uint32				nVertices_;
	uint32				decalVertexBase_;

	void clip( std::vector<ClipVertex>& cvs );

	SmartPointer<Moo::BaseTexture> tileTex_;

	Decal( const Decal& );
	Decal& operator=( const Decal& );
};


#endif // DECAL_HPP
