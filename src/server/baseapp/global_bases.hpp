/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GLOBAL_BASES_HPP
#define GLOBAL_BASES_HPP

#include "Python.h"

#include "cstdmf/binary_stream.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "network/basictypes.hpp"

#include <string>
#include <map>

class Base;
class BaseAppMgrGateway;
class BaseEntityMailBox;
class BinaryIStream;


/*~ class NoModule.GlobalBases
 *  @components{ base }
 *  An instance of this class emulates a dictionary of bases or base mailboxes.
 *
 *  Code Example:
 *  @{
 *  globalBases = BigWorld.globalBases
 *  print "The main mission entity is", globalBases[ "MainMission" ]
 *  print "There are", len( globalBases ), "global bases."
 *  @}
 */
/**
 *	This class is used to expose the collection of global bases.
 */
class GlobalBases : public PyObjectPlus
{
	Py_Header( GlobalBases, PyObjectPlus )

public:
	GlobalBases( PyTypePlus * pType = &GlobalBases::s_type_ );
	~GlobalBases();

	PyObject * 			pyGetAttribute( const char * attr );

	PyObject * 			subscript( PyObject * entityID );
	int					length();

	PY_METHOD_DECLARE( py_has_key )
	PY_METHOD_DECLARE( py_keys )
	PY_METHOD_DECLARE( py_values )
	PY_METHOD_DECLARE( py_items )
	PY_METHOD_DECLARE( py_get )

	static PyObject * 	s_subscript( PyObject * self, PyObject * entityID );
	static Py_ssize_t	s_length( PyObject * self );

	bool registerRequest( Base * pBase, PyObject * pKey,
		PyObject * pCallback );
	bool deregister( Base * pBase, PyObject * pKey );

	void onBaseDestroyed( Base * pBase );

	void add( BinaryIStream & data );
	void add( Base * pBase, const std::string & pickledKey );

	void remove( BinaryIStream & data );
	void remove( const std::string & pickledKey );

	void addLocalsToStream( BinaryOStream & stream ) const;

	void handleBaseAppDeath( const Mercury::Address & deadAddr );

private:
	void add( BaseEntityMailBox * pMailbox, const std::string & pickledKey );

	PyObject * pMap_;

	typedef std::multimap< EntityID, std::string > RegisteredBases;

	RegisteredBases registeredBases_;
};

#ifdef CODE_INLINE
// #include "global_bases.ipp"
#endif

#endif // GLOBAL_BASES_HPP
