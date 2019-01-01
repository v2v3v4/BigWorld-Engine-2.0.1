/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_WATER_VOLUME_HPP
#define PY_WATER_VOLUME_HPP

#include "water.hpp"
#include "pyscript/script.hpp"

/*~ class BigWorld.PyWaterVolume
 *	@components{ client, tools }
 *	This class implements a python wrapper around a water object.
 *	It allows the script programmer to perform queries on the
 *	volume.
 */

//TODO : change to use SmartPointer to water (requires ChunkWater
//and Waters to use SmartPointers too)


class PyWaterVolume : public PyObjectPlus	
{
	Py_Header( PyWaterVolume, PyObjectPlus )
public:
	///	@name Constructor(s) and Destructor.
	//@{
	PyWaterVolume( Water* pWater, PyTypePlus *pType = &s_type_ );
	//@}    
	
	/// Accessors to PyWaterVolume properties.
	//@{
	float surfaceHeight() const;

	///	@name Python Interface to the PyWaterVolume
	//@{
	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()

	PY_RO_ATTRIBUTE_DECLARE( this->surfaceHeight(), surfaceHeight )
private:
	Water*	pWater_;
};

PY_SCRIPT_CONVERTERS_DECLARE( PyWaterVolume )

#endif