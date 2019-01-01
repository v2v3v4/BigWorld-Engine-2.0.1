/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RADIUS_GIZMO_HPP
#define RADIUS_GIZMO_HPP

#include "gizmo_manager.hpp"
#include "solid_shape_mesh.hpp"
#include "formatter.hpp"
#include "input/input.hpp"

typedef SmartPointer<class FloatProxy> FloatProxyPtr;
typedef SmartPointer<class MatrixProxy> MatrixProxyPtr;

class RadiusShapePart;

/**
 *	This class implements a radius gizmo.
 */
class RadiusGizmo : public Gizmo, public Aligned
{
public:
	enum ShowSphere { SHOW_SPHERE_ALWAYS, SHOW_SPHERE_NEVER, SHOW_SPHERE_MODIFIER };

	RadiusGizmo( FloatProxyPtr pRadius,
			MatrixProxyPtr pCenter,
			const std::string& name,
			uint32 colour,
			float editableRadius,
			int enablerModifier = MODIFIER_ALT,
			float adjFactor = 1.75f,
			bool drawZBuffered = true,
			const Matrix * discMatrix = NULL,
			MatrixProxyPtr visualOffsetMatrix = NULL,
			ShowSphere showSphere = SHOW_SPHERE_MODIFIER,
			LabelFormatter<float>* formatter = &DistanceFormatter::s_def);
	~RadiusGizmo();

	void drawZBufferedStuff( bool force );
	bool draw( bool force );
	bool intersects( const Vector3& origin, const Vector3& direction,
														float& t, bool force );
	void click( const Vector3& origin, const Vector3& direction );
	void rollOver( const Vector3& origin, const Vector3& direction );

	void setVisualOffsetMatrixProxy( MatrixProxyPtr matrix );

protected:

	void init();

	void drawRadius( bool force );

	Matrix	objectTransform() const;
	Matrix	gizmoTransform() const;

	bool					active_;
	bool					inited_;
	SolidShapeMesh			selectionMesh_;	
	Moo::VisualPtr			drawMesh_;
	RadiusShapePart *		currentPart_;
	FloatProxyPtr			pFloat_;
	MatrixProxyPtr			pCenter_;
	uint32					gizmoColour_;
	Moo::Colour				lightColour_;
	bool					drawZBuffered_;
	int						enablerModifier_;
	float					editableRadius_;
	std::string				name_;
	const Matrix *			discMatrix_;
	MatrixProxyPtr			visualOffsetMatrix_;
	ShowSphere				showSphere_;
	LabelFormatter<float>*	formatter_;
	float					adjFactor_;

	RadiusGizmo( const RadiusGizmo& );
	RadiusGizmo& operator=( const RadiusGizmo& );
};

#endif // RADIUS_GIZMO_HPP
