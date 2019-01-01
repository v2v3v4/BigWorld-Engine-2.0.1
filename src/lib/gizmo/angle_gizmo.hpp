/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ANGLE_GIZMO_HPP
#define ANGLE_GIZMO_HPP

#include "gizmo_manager.hpp"
#include "solid_shape_mesh.hpp"
#include "input/input.hpp"

typedef SmartPointer<class MatrixProxy> MatrixProxyPtr;
typedef SmartPointer<class FloatProxy> FloatProxyPtr;


class AngleShapePart;

/**
 *	This class implements a gizmo for the world editor
 *	that allows interactive manipulation of an angle property.
 *
 *	The angle property is provided via a FloatProxy passed
 *	into the constructor.
 *
 *	By default, angles are edited using the ALT key while
 *	over an object's gizmo.
 */
class AngleGizmo : public Gizmo, public Aligned
{
public:
	AngleGizmo( MatrixProxyPtr pMatrix,
		FloatProxyPtr pAngle,
		int enablerModifier = MODIFIER_ALT );
	~AngleGizmo();

	void drawZBufferedStuff( bool force );
	bool draw( bool force );
	bool intersects( const Vector3& origin, const Vector3& direction,
														float& t, bool force );
	void click( const Vector3& origin, const Vector3& direction );
	void rollOver( const Vector3& origin, const Vector3& direction );

protected:
	Matrix	objectTransform() const;
	Matrix	gizmoTransform() const;

	void init();

	bool					active_;
	bool					inited_;
	SolidShapeMesh			mesh_;
	class AngleShapePart*	currentPart_;
	MatrixProxyPtr			pMatrix_;
	FloatProxyPtr			pAngle_;
	Moo::Colour				lightColour_;
	int						enablerModifier_;

	AngleGizmo( const AngleGizmo& );
	AngleGizmo& operator=( const AngleGizmo& );
};

#endif // ANGLE_GIZMO_HPP
