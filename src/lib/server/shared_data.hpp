/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SHARED_DATA_HPP
#define SHARED_DATA_HPP

#include "pyscript/pyobject_plus.hpp"

#include <string>
#include <map>

typedef uint8 SharedDataType;
class BinaryOStream;
class Pickler;

/*~ class NoModule.SharedData
 *  @components{ base, cell }
 *  An instance of this class emulates a dictionary of data shared between
 *	server components.
 *
 *  Code Example:
 *  @{
 *  sharedData = BigWorld.globalData
 *  print "The main mission entity is", sharedData[ "MainMission" ]
 *  print "There are", len( sharedData ), "global data entries."
 *  @}
 */
/**
 *	This class is used to expose the collection of CellApp data.
 */
class SharedData : public PyObjectPlus
{
Py_Header( SharedData, PyObjectPlus )

public:
	typedef void (*SetFn)( const std::string & key, const std::string & value,
			SharedDataType dataType );
	typedef void (*DelFn)( const std::string & key, SharedDataType dataType );

	typedef void (*OnSetFn)( PyObject * pKey, PyObject * pValue,
			SharedDataType dataType );
	typedef void (*OnDelFn)( PyObject * pKey, SharedDataType dataType );

	SharedData( SharedDataType dataType,
			SharedData::SetFn setFn,
			SharedData::DelFn delFn,
			SharedData::OnSetFn onSetFn,
			SharedData::OnDelFn onDelFn,
			Pickler * pPickler,
			PyTypePlus * pType = &SharedData::s_type_ );
	~SharedData();

	PyObject *			pyGetAttribute( const char * attr );

	PyObject *			subscript( PyObject * key );
	int					ass_subscript( PyObject * key, PyObject * value );
	int					length();

	PY_METHOD_DECLARE( py_has_key )
	PY_METHOD_DECLARE( py_keys )
	PY_METHOD_DECLARE( py_values )
	PY_METHOD_DECLARE( py_items )
	PY_METHOD_DECLARE( py_get )

	static PyObject *	s_subscript( PyObject * self, PyObject * key );
	static int			s_ass_subscript( PyObject * self,
							PyObject * key, PyObject * value );
	static Py_ssize_t	s_length( PyObject * self );

	bool setValue( const std::string & key, const std::string & value );
	bool delValue( const std::string & key );

	bool addToStream( BinaryOStream & stream ) const;

private:
	std::string pickle( PyObject * pObj ) const;
	PyObject * unpickle( const std::string & str ) const;

	PyObject * pMap_;
	SharedDataType dataType_;

	SetFn	setFn_;
	DelFn	delFn_;
	OnSetFn	onSetFn_;
	OnDelFn	onDelFn_;

	Pickler * pPickler_;
};

#endif // SHARED_DATA_HPP
