/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_CELL_DATA_HPP
#define PY_CELL_DATA_HPP

#include "pyscript/pyobject_plus.hpp"
#include "entity_type.hpp"

/**
 *	This class is used to wrap cell data. It keeps the information as a blob and
 *	only creates a dictionary if necessary.
 */
class PyCellData : public PyObjectPlus
{
	Py_Header( PyCellData, PyObjectPlus )

public:
	PyCellData( EntityTypePtr pEntityType, BinaryIStream & data,
		bool persistentDataOnly, PyTypePlus * pType = &PyCellData::s_type_ );

	PyObject * 			pyGetAttribute( const char * attr );

	bool		addToStream( BinaryOStream & stream, bool addPosAndDir,
							bool addPersistentOnly );

	void		migrate( EntityTypePtr pType );

	PyObjectPtr	getDict();
	PY_AUTO_METHOD_DECLARE( RETDATA, getDict, END )

	PyObjectPtr createPyDictOnDemand();

	static bool addToStream( BinaryOStream & stream, EntityTypePtr pType,
			PyObject * pCellData, bool addPosAndDir, bool addPersistentOnly );
	static bool addDictToStream( BinaryOStream & stream,
			EntityTypePtr pEntityType, PyObject* pDict, bool addPosAndDir,
			bool addPersistentOnly );
	static Vector3 getPos( PyObjectPtr pCellData );
	static Vector3 getDir( PyObjectPtr pCellData );
	static SpaceID getSpaceID( PyObjectPtr pCellData );
	static void addSpatialData(  PyObjectPtr pCellData, const Vector3 & pos,
								 const Direction3D & dir, SpaceID spaceID );

private:
	static Vector3 getPosOrDir( PyObjectPtr pCellData,
									const char * pName, int offset );

	EntityTypePtr	pEntityType_;
	PyObjectPtr		pDict_;
	std::string		data_;
	bool			dataHasPersistentOnly_;
};

#endif // PY_CELL_DATA_HPP
