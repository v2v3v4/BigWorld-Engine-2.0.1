/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ITEM_FUNCTOR_HPP
#define ITEM_FUNCTOR_HPP

#include "general_properties.hpp"
#include "tool_functor.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "snap_provider.hpp"
#include "property_scaler_helper.hpp"
#include "property_rotater_helper.hpp"

class MatrixProxy;
typedef SmartPointer<MatrixProxy> MatrixProxyPtr;
class FloatProxy;
typedef SmartPointer<FloatProxy> FloatProxyPtr;


/**
 *	This class is a functor that moves around a chunk item, or anything
 *	else that can return a matrix.
 */
class MatrixMover : public ToolFunctor
{
	Py_Header( MatrixMover, ToolFunctor )
public:
	MatrixMover( MatrixProxyPtr pMatrix, PyTypePlus * pType = &s_type_ );
	MatrixMover( MatrixProxyPtr pMatrix, bool snap, bool rotate, PyTypePlus * pType = &s_type_ );
	~MatrixMover();

	virtual void update( float dTime, Tool& tool );
	virtual bool handleKeyEvent( const KeyEvent & event, Tool& tool );
	virtual bool handleMouseEvent( const MouseEvent & event, Tool& tool )
		{ return false; }
	virtual bool applying() const { return true; }
	static bool moving()	{	return !!moving_;	}

	PY_METHOD_DECLARE( py_setUndoName )

	PY_FACTORY_DECLARE()

private:
	SnapProvider::SnapMode snapMode_;
	Vector3 lastLocatorPos_;
	Vector3 totalLocatorOffset_;
	bool gotInitialLocatorPos_;
	bool snap_;
	bool rotate_;
	static int moving_;

	/** What the barrier name will be set to */
	std::string undoName_;
};


/**
 *	This class is a functor that scales a chunk item, or anything
 *	else that can return a matrix.
 */
class MatrixScaler : public ToolFunctor
{
	Py_Header( MatrixScaler, ToolFunctor )
public:
	MatrixScaler( MatrixProxyPtr pMatrix, float scaleSpeedFactor = 1.f,
		FloatProxyPtr scaleX = NULL, FloatProxyPtr scaleY = NULL, FloatProxyPtr scaleZ = NULL,
		PyTypePlus * pType = &s_type_ );

	virtual void update( float dTime, Tool& tool );
	virtual bool handleKeyEvent( const KeyEvent & event, Tool& tool );
	virtual bool handleMouseEvent( const MouseEvent & event, Tool& tool )
		{ return false; }
	virtual bool applying() const { return true; }

	PY_FACTORY_DECLARE()

private:
	MatrixProxyPtr	pMatrix_;
	Matrix			initialMatrix_;
	Matrix			invInitialMatrix_;
	Vector3			initialScale_;
	Vector3			grabOffset_;
	bool			grabOffsetSet_;
	float			scaleSpeedFactor_;
	FloatProxyPtr	scaleX_;
	FloatProxyPtr	scaleY_;
	FloatProxyPtr	scaleZ_;
};


/**
 * This scales all the current scale properties
 */
class PropertyScaler : public ToolFunctor
{
	Py_Header( PropertyScaler, ToolFunctor )
public:
	PropertyScaler( float scaleSpeedFactor = 1.f,
		FloatProxyPtr scaleX = NULL, FloatProxyPtr scaleY = NULL, FloatProxyPtr scaleZ = NULL,
		PyTypePlus * pType = &s_type_ );

	virtual void update( float dTime, Tool& tool );
	virtual bool handleKeyEvent( const KeyEvent & event, Tool& tool );
	virtual bool handleMouseEvent( const MouseEvent & event, Tool& tool )
		{ return false; }
	virtual bool applying() const { return true; }
private:

	float scaleSpeedFactor_;

	FloatProxyPtr scaleX_;
	FloatProxyPtr scaleY_;
	FloatProxyPtr scaleZ_;

	bool					grabOffsetSet_;
	Matrix					invGrabOffset_;

	PropertyScalerHelper	scalerHelper_;

};

/**
 *	This class is a functor that rotates a chunk item, or anything
 *	else that can return a matrix.
 */
class MatrixRotator : public ToolFunctor
{
	Py_Header( MatrixRotator, ToolFunctor )
public:
	MatrixRotator( MatrixProxyPtr pMatrix, PyTypePlus * pType = &s_type_ );

	virtual void update( float dTime, Tool& tool );
	virtual bool handleKeyEvent( const KeyEvent & event, Tool& tool );
	virtual bool handleMouseEvent( const MouseEvent & event, Tool& tool )
		{ return false; }
	virtual bool applying() const { return true; }

	PY_FACTORY_DECLARE()

private:
	bool					grabOffsetSet_;
	Matrix					invGrabOffset_;
	Vector3					initialToolLocation_;
	PropertyRotaterHelper	rotaterHelper_;
};

/**
 *	This class is a functor that alters any float property.
 */
class DynamicFloatDevice : public ToolFunctor
{
	Py_Header( DynamicFloatDevice, ToolFunctor )
public:
	DynamicFloatDevice( MatrixProxyPtr pCenter,
		FloatProxyPtr pFloat,
		float adjFactor,
		PyTypePlus * pType = &s_type_ );

	virtual void update( float dTime, Tool& tool );
	virtual bool handleKeyEvent( const KeyEvent & event, Tool& tool );
	virtual bool handleMouseEvent( const MouseEvent & event, Tool& tool )
		{ return false; }
	virtual bool applying() const { return true; }

	PY_FACTORY_DECLARE()

private:
	MatrixProxyPtr	pCenter_;
	FloatProxyPtr	pFloat_;
	Matrix			initialCenter_;
	float			initialFloat_;
	Vector3			grabOffset_;
	bool			grabOffsetSet_;
	float			adjFactor_;
};

/**
 * Rotates the current GenRotationProperty via the mousewheel
 */
class WheelRotator : public ToolFunctor
{
	Py_Header( WheelRotator, ToolFunctor )
public:
	WheelRotator( PyTypePlus * pType = &s_type_ );

	virtual void update( float dTime, Tool& tool );
	virtual bool handleKeyEvent( const KeyEvent & event, Tool& tool );
	virtual bool handleMouseEvent( const MouseEvent & event, Tool& tool );
	virtual bool applying() const { return true; }

	PY_FACTORY_DECLARE()

private:
	float					timeSinceInput_;
	float					rotAmount_;
	bool					needsInit_;
	std::vector< GenRotationProperty * > curProperties_;

	PropertyRotaterHelper	rotaterHelper_;

	bool arePropertiesValid() const;

	/** Programmatically add rotation */
	void rotateBy( float degs, bool useLocalYaxis );

	void commitChanges();
};



/**
 *	This class is a functor that actually moves around a matrix.
 *  (as opposed to a property, as is done above..)
 */
class MatrixPositioner : public ToolFunctor
{
	Py_Header( MatrixPositioner, ToolFunctor )
public:
	MatrixPositioner( MatrixProxyPtr pMatrix, PyTypePlus * pType = &s_type_ );

	virtual void update( float dTime, Tool& tool );
	virtual bool handleKeyEvent( const KeyEvent & event, Tool& tool );
	virtual bool handleMouseEvent( const MouseEvent & event, Tool& tool )
		{ return false; }
	virtual bool applying() const { return true; }

	PY_METHOD_DECLARE( py_setUndoName )

	PY_FACTORY_DECLARE()

private:
	Vector3 lastLocatorPos_;
	Vector3 totalLocatorOffset_;
	bool gotInitialLocatorPos_;

	/** What the barrier name will be set to */
	std::string undoName_;

	MatrixProxyPtr matrix_;
};


#endif // ITEM_FUNCTOR_HPP
