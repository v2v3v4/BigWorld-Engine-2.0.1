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
#include "py_material.hpp"
#include "pyscript/py_data_section.hpp"
#include "romp/py_texture_provider.hpp"

#ifndef CODE_INLINE
#include "py_material.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Romp", 0 )

// Python statics
PY_TYPEOBJECT( PyMaterial )

PY_BEGIN_METHODS( PyMaterial )	
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyMaterial )	
PY_END_ATTRIBUTES()

/*~ function BigWorld.Material
 *	@components{ client,  tools }
 *	Factory function to create and return a PyMaterial object.
 *
 *	Note that the parameter list is complicated, so although
 *	three parameters are listed below, the PyMaterial constructor takes :
 *	Either the name of an .fx file, (and optionally the name of a diffuse map.)
 *	Or a PyDataSection containing a material section.
 *
 *	There are no listed attributes for a PyMaterial, however on creation,
 *	the attribute dictionary will be filled with all the "artist editable"
 *	variables contained in the effect file.
 *
 *	@param	effectName	Name of the effect file (*.fx)
 *	@param	diffuseMap	Name of the diffuse map for the .fx file
 *	@param	DataSection DataSection for a material section.
 *
 *	@return A new PyMaterial object.
 */
PY_FACTORY_NAMED( PyMaterial, "Material", BigWorld )


PyMaterial::PyMaterial( Moo::EffectMaterialPtr pMaterial, PyTypePlus *pType ):
	PyObjectPlus( pType ),
	pMaterial_( pMaterial )
{
	instanceAttributes_.add( this->s_attributes_.di_ );

	Moo::EffectMaterial::Properties::iterator it = pMaterial_->properties().begin();
	Moo::EffectMaterial::Properties::iterator en = pMaterial_->properties().end();
	for( ; it != en; it++ )
	{
		D3DXHANDLE param = it->first;
		D3DXPARAMETER_DESC paramDesc;
		pMaterial_->pEffect()->pEffect()->GetParameterDesc( param, &paramDesc );
		instanceAttributes_.addMember( paramDesc.Name );
	}
}


PyMaterial::~PyMaterial()
{
}


/**
 * This method finds the material property, effect handle and decsription
 * given the string name of the property.
 * @param	name	name of the property to find
 * @param	prop	[out] the found property
 * @param	handle	[out] handle of the found property
 * @param	desc	[out] decsription of the found property
 * @return	bool	whether or not the property was found.
 */
bool PyMaterial::findPropertyAndDesc(
	const char * name, Moo::EffectPropertyPtr & prop,
	D3DXHANDLE & handle,
	D3DXPARAMETER_DESC & desc )
{
	Moo::EffectMaterial::Properties::iterator it = pMaterial_->properties().begin();
	Moo::EffectMaterial::Properties::iterator en = pMaterial_->properties().end();
	for( ; it != en; it++ )
	{
		handle = it->first;		
		pMaterial_->pEffect()->pEffect()->GetParameterDesc( handle, &desc );
		if(!stricmp( name, desc.Name ))
		{
			prop = it->second;
			return true;
		}
	}
	return false;
}


/**
 * This method updates the realtime material properties.
 * @param	dTime	delta frame time, in seconds.
 */
void PyMaterial::tick( float dTime )
{
	Vector4 v4;
	Vector4Providers::iterator vit = v4Providers_.begin();
	Vector4Providers::iterator ven = v4Providers_.end();
	while (vit != ven)
	{
		vit->second->output(v4);
		vit->first->be(v4);
		++vit;
	}

	Matrix m;
	MatrixProviders::iterator mit = matrixProviders_.begin();
	MatrixProviders::iterator men = matrixProviders_.end();
	while (mit != men)
	{
		mit->second->matrix(m);
		mit->first->be(m);
		++mit;
	}
}


/**
 * This method returns a python attribute for PyMaterial.  It overrides the
 * default pyGetAttribute method and allows the script programmer direct
 * access to D3DEffect variables
 * @param	attr	name of the attribute
 * @return	PyObject returned python attribute object, or None
 */
PyObject * PyMaterial::pyGetAttribute( const char *attr )
{
	D3DXHANDLE param;
	D3DXPARAMETER_DESC propertyDesc;
	Moo::EffectPropertyPtr prop;

	//Implement dir() support for our additional instance members.
	if (!stricmp( attr, "__members__"))
	{
		return instanceAttributes_.members();
	}

	if (this->findPropertyAndDesc(attr,prop,param,propertyDesc))
	{
		//First check our providers to see if we have one registered
		//for this property.
		Vector4Providers::iterator vit = v4Providers_.find(prop);
		if (vit != v4Providers_.end())
		{
			return Script::getData(vit->second);
		}

		MatrixProviders::iterator mit = matrixProviders_.find(prop);
		if (mit != matrixProviders_.end())
		{
			return Script::getData(mit->second);
		}

		//Otherwise we will get the data out of the property iteself.
		switch( propertyDesc.Type )
		{
		case D3DXPT_BOOL:
			bool b;
			prop->getBool(b);
			return Script::getData(b);
		case D3DXPT_INT:
			int i;
			prop->getInt(i);
			return Script::getData(i);				
		case D3DXPT_FLOAT:
			if (propertyDesc.Class == D3DXPC_VECTOR)
			{
				Vector4 v;
				prop->getVector(v);
				return Script::getData(v);
			}
			else if ((propertyDesc.Class == D3DXPC_MATRIX_ROWS ||
				propertyDesc.Class == D3DXPC_MATRIX_COLUMNS))
			{
				Matrix m;
				prop->getMatrix(m);
				return Script::getData(m);
			}
			else
			{
				float f;
				prop->getFloat(f);
				return Script::getData(f);
			}
		case D3DXPT_TEXTURE:
		case D3DXPT_TEXTURECUBE:
			{
				std::string r;
				prop->getResourceID(r);
				if (!r.empty())
				{
					Moo::BaseTexturePtr pTex = Moo::TextureManager::instance()->get(r);
					return new PyTextureProvider( NULL, pTex );
				}
				else
				{
					Py_Return;
				}
			}
			break;
		default:
			PyErr_SetString( PyExc_TypeError, "BigWorld.PyMaterial: "
				"Could not determine the type of the requested material attribute." );
			Py_Return;
		}
	}

	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute(attr);
}


/**
 * This method sets a python attribute for PyMaterial.  It overrides the
 * default pySetAttribute method and allows the script programmer direct
 * control over D3DEffect variables
 * @param	attr	name of the attribute to set
 * @param	value	python object containing the value to set the attribute to.
 * @return	int		0 for success, -1 for failure.
 */
int PyMaterial::pySetAttribute( const char *attr, PyObject *value )
{
	BW_GUARD;

	D3DXHANDLE param;
	D3DXPARAMETER_DESC propertyDesc;
	Moo::EffectPropertyPtr prop;

	if (this->findPropertyAndDesc(attr,prop,param,propertyDesc))
	{
		//First check our providers to see if we have one registered
		//for this property, and remove it.
		Vector4Providers::iterator vit = v4Providers_.find(prop);
		if (vit != v4Providers_.end())
		{
			v4Providers_.erase(vit);
		}

		MatrixProviders::iterator mit = matrixProviders_.find(prop);
		if (mit != matrixProviders_.end())
		{
			matrixProviders_.erase(mit);
		}

		switch( propertyDesc.Type )
		{
		case D3DXPT_BOOL:
			bool b;
			Script::setData(value,b);
			prop->be(b);
			return 0;
		case D3DXPT_INT:
			int i;
			Script::setData(value,i);
			prop->be(i);
			return 0;
		case D3DXPT_FLOAT:
			if (propertyDesc.Class == D3DXPC_VECTOR)
			{
				Vector4 v;
				if (Vector4Provider::Check(value))
				{
					//INFO_MSG( "material property %s is now a Vector4Provider", propertyDesc.Name );
					Vector4Provider* v4Prov = static_cast<Vector4Provider*>(value);
					v4Providers_[prop] = v4Prov;					
					v4Prov->output(v);
				}
				else
				{
					//INFO_MSG( "material property %s is not a Vector4Provider", propertyDesc.Name );
					Script::setData(value,v);
				}
				prop->be(v);
				return 0;
			}
			else if ((propertyDesc.Class == D3DXPC_MATRIX_ROWS ||
				propertyDesc.Class == D3DXPC_MATRIX_COLUMNS))
			{
				Matrix m;
				if (MatrixProvider::Check(value))
				{
					//INFO_MSG( "material property %s is now a MatrixProvider", propertyDesc.Name );
					MatrixProvider* mProv = static_cast<MatrixProvider*>(value);
					matrixProviders_[prop] = mProv;
					mProv->matrix(m);
				}
				else
				{
					INFO_MSG( "material property %s is not a MatrixProvider", propertyDesc.Name );
					Script::setData(value,m);
				}				
				prop->be(m);
				return 0;
			}
			else
			{
				if (Vector4Provider::Check(value))
				{
					//INFO_MSG( "material property %s is now a Vector4Provider", propertyDesc.Name );
					Vector4 v;
					Vector4Provider* v4Prov = static_cast<Vector4Provider*>(value);
					v4Providers_[prop] = v4Prov;
					v4Prov->output(v);
					prop->be(v);
				}
				else if (PyVector<Vector4>::Check(value))
				{
					//INFO_MSG( "material property %s is not a Vector4Provider", propertyDesc.Name );
					Vector4 v;
					Script::setData(value,v);
					prop->be(v.w);
				}
				else
				{
					//INFO_MSG( "material property %s is not a Vector4Provider", propertyDesc.Name );
					float f;
					Script::setData(value,f);
					prop->be(f);
				}				
				return 0;
			}
		case D3DXPT_TEXTURE:
		case D3DXPT_TEXTURECUBE:
			{
				if (PyTextureProvider::Check(value))
				{
					PyTextureProvider * pTex = static_cast<PyTextureProvider*>(value);
					prop->be(pTex->texture());
					return 0;
				}
				else
				{
					std::string s;
					Script::setData(value, s);
					prop->be(s);
					return 0;
				}
			}
			break;
		default:
			PyErr_SetString( PyExc_TypeError, "BigWorld.PyMaterial: "
				"Could not determine the type of the requested material attribute." );
			return -1;
		}
	}

	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute(attr,value);
}


PyObject* PyMaterial::pyNew(PyObject* args)
{
	PyObject * pObj = NULL;
	char * effectName = NULL;
	char * diffuseMap = NULL;
	Moo::EffectMaterial * m = NULL;

	if (PyArg_ParseTuple( args, "s|s", &effectName, &diffuseMap ))
	{
		if (effectName != NULL)
		{
			m = new Moo::EffectMaterial();
			bool ok = false;
			if (diffuseMap != NULL)
			{
				ok = m->initFromEffect(effectName, diffuseMap);
			}
			else
			{
				ok = m->initFromEffect(effectName);
			}

			if (ok)
			{
				//effect materials by default share their properties, unless
				//created via a material section, in which the listed
				//properties in the material section become instanced.  In
				//this case, we have created a material directly based on an
				//.fx file, so we need to replace all default properties
				//with instanced ones.
				m->replaceDefaults();
			}
			else
			{
				delete m;
				m = NULL;
				PyErr_SetString( PyExc_TypeError, "BigWorld.PyMaterial: "
					"Error creating material from effect filename." );
				return NULL;
			}
		}
	}
	else if (PyArg_ParseTuple(args, "O", &pObj ))
	{
		//Failure of ParseTuple above sets the python error string,
		//but its ok to fail that test.
		PyErr_Clear();

		if ( pObj != NULL && PyDataSection::Check(pObj) )
		{
			PyDataSection * pSect = static_cast<PyDataSection*>(pObj);		
			m = new Moo::EffectMaterial();
			if (!m->load( pSect->pSection() ))
			{
				delete m;
				m = NULL;
				PyErr_SetString( PyExc_TypeError, "BigWorld.PyMaterial: "
					"Error loading material from data section." );
				return NULL;
			}
		}
	}

	if (!m)
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.PyMaterial: "
			"Argument parsing error: Expected a d3dEffect filename or a material data section. "
			"If using d3dEffect filename, you may also specify a diffuse map name." );
		return NULL;
	}
	
	return new PyMaterial( m );
}


PY_SCRIPT_CONVERTERS( PyMaterial )
