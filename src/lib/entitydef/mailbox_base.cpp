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

#include "Python.h"

#include "mailbox_base.hpp"
#include "method_description.hpp"
#include "remote_entity_method.hpp"

#include "cstdmf/memory_stream.hpp"

DECLARE_DEBUG_COMPONENT2( "entitydef", 0 )

// -----------------------------------------------------------------------------
// Section: PyEntityMailBox
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyEntityMailBox )

PY_BEGIN_METHODS( PyEntityMailBox )
	PY_PICKLING_METHOD()
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyEntityMailBox )
PY_END_ATTRIBUTES()

PY_SCRIPT_CONVERTERS( PyEntityMailBox )

PyEntityMailBox::Population PyEntityMailBox::s_population_;

/**
 *	This method overrides the virtual pyGetAttribute function.
 */
PyObject * PyEntityMailBox::pyGetAttribute( const char * attr )
{
	const MethodDescription * pDescription = this->findMethod( attr );
	if (pDescription != NULL)
	{
		return new RemoteEntityMethod( this, pDescription );
	}

	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}



typedef std::map<
	EntityMailBoxRef::Component,PyEntityMailBox::FactoryFn> Fabricators;
typedef std::vector< std::pair<
	PyEntityMailBox::CheckFn,PyEntityMailBox::ExtractFn> > Interpreters;
typedef std::vector< PyTypeObject * >  MailBoxTypes;


/**
 *	MailBoxRef helper struct
 */
static struct MailBoxRefRegistry
{
	Fabricators		fabs_;
	Interpreters	inps_;
	MailBoxTypes	mailBoxTypes_;
} * s_pRefReg = NULL;


/**
 *	Constructor.
 */
PyEntityMailBox::PyEntityMailBox( 
			PyTypePlus * pType /* = &PyEntityMailBox::s_type_ */ ) :
		PyObjectPlus( pType ),
		populationIter_()
{
	populationIter_ = s_population_.insert( s_population_.end(), this );
}


/**
 *	Destructor.
 */
PyEntityMailBox::~PyEntityMailBox()
{
	s_population_.erase( populationIter_ );
}



/**
 *	Construct a PyEntityMailBox or equivalent from an EntityMailBoxRef.
 *	Returns Py_None on failure.
 */
PyObject * PyEntityMailBox::constructFromRef(
	const EntityMailBoxRef & ref )
{
	if (ref.id == 0) Py_Return;

	if (s_pRefReg == NULL) Py_Return;

	Fabricators::iterator found = s_pRefReg->fabs_.find( ref.component() );
	if (found == s_pRefReg->fabs_.end()) Py_Return;

	PyObject * pResult = (*found->second)( ref );

	if (pResult)
	{
		return pResult;
	}
	else
	{
		WARNING_MSG( "PyEntityMailBox::constructFromRef: "
				"Could not create mailbox from id %d. addr %s. component %d\n",
				ref.id, ref.addr.c_str(), ref.component() );
		Py_Return;
	}
}

/**
 *	Register a PyEntityMailBox factory
 */
void PyEntityMailBox::registerMailBoxComponentFactory(
	EntityMailBoxRef::Component c, FactoryFn fn, PyTypeObject * pType )
{
	if (s_pRefReg == NULL) s_pRefReg = new MailBoxRefRegistry();
	s_pRefReg->fabs_.insert( std::make_pair( c, fn ) );
	s_pRefReg->mailBoxTypes_.push_back( pType );
}


/**
 *	Return whether or not the given python object can be reduced to
 *	an EntityMailBoxRef
 */
bool PyEntityMailBox::reducibleToRef( PyObject * pObject )
{
	if (pObject == Py_None) return true;
	if (s_pRefReg != NULL)
	{
		for (Interpreters::iterator it = s_pRefReg->inps_.begin();
			it != s_pRefReg->inps_.end();
			it++)
		{
			if ((*it->first)( pObject )) return true;
		}
	}
	return false;
}

/**
 *	Reduce the given python object to an EntityMailBoxRef.
 *	Return an empty mailbox if it cannot be reduced.
 */
EntityMailBoxRef PyEntityMailBox::reduceToRef( PyObject * pObject )
{
	EntityMailBoxRef mbr;
	if (pObject != Py_None && s_pRefReg != NULL)
	{
		for (Interpreters::iterator it = s_pRefReg->inps_.begin();
			it != s_pRefReg->inps_.end();
			it++)
		{
			if ((*it->first)( pObject ))
			{
				mbr = (*it->second)( pObject );
				return mbr;
			}
		}
	}
	mbr.init();
	return mbr;
}

/**
 *	Register a Python object reducible to an EntityMailBoxRef
 */
void PyEntityMailBox::registerMailBoxRefEquivalent( CheckFn cf, ExtractFn ef )
{
	if (s_pRefReg == NULL) s_pRefReg = new MailBoxRefRegistry();
	s_pRefReg->inps_.push_back( std::make_pair( cf, ef ) );
}


/**
 *	Visit every mail box in the population.
 *
 *	@param visitor 	The PyEntityMailBoxVisitor instance.
 */
void PyEntityMailBox::visit( PyEntityMailBoxVisitor & visitor )
{
	Population::iterator iPopulation = s_population_.begin();

	while (iPopulation != s_population_.end())
	{
		PyEntityMailBox * pMailBox = *iPopulation;
		++iPopulation;
		visitor.onMailBox( pMailBox );
	}
}


// -----------------------------------------------------------------------------
// Section: Pickling and unpickling routines
// -----------------------------------------------------------------------------

/**
 *  This method reduces this mailbox to something that can be pickled
 */
PyObject * PyEntityMailBox::pyPickleReduce()
{
	EntityMailBoxRef embr = PyEntityMailBox::reduceToRef( this );

	PyObject * pConsArgs = PyTuple_New( 1 );
	PyTuple_SET_ITEM( pConsArgs, 0,
		PyString_FromStringAndSize( (char*)&embr, sizeof(embr) ) );

	return pConsArgs;
}

/**
 *	This static function unpickles a previously pickled mailbox
 *	(possibly from a different component)
 */
static PyObject * PyEntityMailBox_pyPickleResolve( const std::string & str )
{
	if (str.size() != sizeof(EntityMailBoxRef))
	{
		PyErr_SetString( PyExc_ValueError, "PyEntityMailBox_pyPickleResolve: "
			"wrong length string to unpickle" );
		return NULL;
	}

	return PyEntityMailBox::constructFromRef( *(EntityMailBoxRef*)str.data() );
}
PY_AUTO_UNPICKLING_FUNCTION( RETOWN, PyEntityMailBox_pyPickleResolve,
	ARG( std::string, END ), MailBox )


/**
 *	This method is used to implement the str and repr methods called on an
 *	entity mailbox.
 */
PyObject * PyEntityMailBox::pyRepr()
{
	EntityMailBoxRef embr = PyEntityMailBox::reduceToRef( this );
	const char * location =
		(embr.component() == EntityMailBoxRef::CELL)   ? "Cell" :
		(embr.component() == EntityMailBoxRef::BASE)   ? "Base" :
		(embr.component() == EntityMailBoxRef::CLIENT) ? "Client" :
		(embr.component() == EntityMailBoxRef::BASE_VIA_CELL)   ? "BaseViaCell" :
		(embr.component() == EntityMailBoxRef::CLIENT_VIA_CELL) ? "ClientViaCell" :
		(embr.component() == EntityMailBoxRef::CELL_VIA_BASE)   ? "CellViaBase" :
		(embr.component() == EntityMailBoxRef::CLIENT_VIA_BASE) ? "ClientViaBase" : "???";

	return PyString_FromFormat( "%s mailbox id: %d type: %d addr: %s",
			location, embr.id, embr.type(), embr.addr.c_str() );
}


// -----------------------------------------------------------------------------
// Section: Script converters for EntityMailBoxRef
// -----------------------------------------------------------------------------

/**
 *	setData function
 */
int Script::setData( PyObject * pObj, EntityMailBoxRef & mbr,
		const char * varName )
{
	if (!PyEntityMailBox::reducibleToRef( pObj ))
	{
		PyErr_Format( PyExc_TypeError, "%s must be set to "
			"a type reducible to an EntityMailBox", varName );
		return -1;
	}

	mbr = PyEntityMailBox::reduceToRef( pObj );
	return 0;
}

/**
 *	getData function
 */
PyObject * Script::getData( const EntityMailBoxRef & mbr )
{
	return PyEntityMailBox::constructFromRef( mbr );
}

// mailbox_base.cpp
