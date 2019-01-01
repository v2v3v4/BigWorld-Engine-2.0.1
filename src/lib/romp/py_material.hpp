/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_MATERIAL_HPP
#define PY_MATERIAL_HPP

#include "moo/effect_material.hpp"
#include "pyscript/script.hpp"
#include "pyscript/script_math.hpp"
#include "pyscript/pyobject_plus.hpp"

class PyMaterial : public PyObjectPlus
{
	Py_Header( PyMaterial, PyObjectPlus )
public:
	PyMaterial( Moo::EffectMaterialPtr pMaterial, PyTypePlus *pType = &s_type_ );
	~PyMaterial();

	void tick( float dTime );
	Moo::EffectMaterialPtr pMaterial() const	{ return pMaterial_; }

	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );

	PY_FACTORY_DECLARE()
private:
	bool PyMaterial::findPropertyAndDesc(
		const char * name, Moo::EffectPropertyPtr & prop,
		D3DXHANDLE & handle,
		D3DXPARAMETER_DESC & desc );
	Moo::EffectMaterialPtr pMaterial_;

	typedef std::map<Moo::EffectPropertyPtr, Vector4ProviderPtr> Vector4Providers;
	typedef std::map<Moo::EffectPropertyPtr, MatrixProviderPtr> MatrixProviders;

	Vector4Providers	v4Providers_;
	MatrixProviders		matrixProviders_;
	PyDirInfo			instanceAttributes_;
};

typedef SmartPointer<PyMaterial> PyMaterialPtr;

PY_SCRIPT_CONVERTERS_DECLARE( PyMaterial )

#ifdef CODE_INLINE
#include "py_material.ipp"
#endif

#endif //#ifndef PY_MATERIAL_HPP