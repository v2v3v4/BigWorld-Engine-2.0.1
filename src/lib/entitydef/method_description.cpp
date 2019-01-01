/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 * 	@file
 *
 * 	This file implements the MethodDescription class.
 *
 *	@ingroup entity
 */

#include "pch.hpp"

#include "method_description.hpp"

#include "data_description.hpp"
#include "method_response.hpp"

#include "cstdmf/binary_stream.hpp"
#include "cstdmf/md5.hpp"
#include "cstdmf/watcher.hpp"

#include "network/basictypes.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include <float.h>

#ifndef CODE_INLINE
#include "method_description.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "DataDescription", 0 )

// -----------------------------------------------------------------------------
// Section: Method Description
// -----------------------------------------------------------------------------

/**
 *	Default constructor.
 */
MethodDescription::MethodDescription() :
	name_(),
	flags_( 0 ),
	args_(),
	returnValues_(),
	internalIndex_( -1 ),
	exposedIndex_( -1 ),
	exposedSubIndex_( -1 ),
	priority_( FLT_MAX )
#if ENABLE_WATCHERS
	,timeSpent_( 0 ),
	timesCalled_( 0 )
#endif
{
}

/**
 *	The copy constructor.
 */
MethodDescription::MethodDescription( const MethodDescription & description )
{
	(*this) = description;
}


/**
 *	Destructor.
 */
MethodDescription::~MethodDescription()
{
}


/**
 *	The assignment operator.
 */
MethodDescription & MethodDescription::operator=(
				const MethodDescription & description )
{
	// The assignment operator used to be necessary when we were storing the
	// function pointer which needed to be reference counted.

	if (this != &description)
	{
		name_			= description.name_;
		flags_			= description.flags_;
		args_			= description.args_;
		returnValues_	= description.returnValues_;
		internalIndex_	= description.internalIndex_;
		exposedIndex_	= description.exposedIndex_;
		exposedSubIndex_= description.exposedSubIndex_;
		priority_		= description.priority_;
#if ENABLE_WATCHERS
		timeSpent_		= description.timeSpent_;
		timesCalled_	= description.timesCalled_;
#endif
	}

	return *this;
}


/**
 *	This method parses a method description.
 *
 *	@param pSection		Datasection from which to parse the description.
 *	@param component	Indicates which component this method belongs to.
 *
 *	@return True if the description was parsed successfully.
 */
bool MethodDescription::parse( DataSectionPtr pSection, Component component )
{
	bool result = true;

	name_ = pSection->sectionName();

	if (name_ == "")
	{
		WARNING_MSG("MethodDescription is missing <Name> tag");
		return false;
	}

	if (pSection->findChild( "Exposed" ))
	{
		this->setExposed();
		IF_NOT_MF_ASSERT_DEV( this->isExposed() )
		{
			MF_EXIT( "just set exposed, but we're not exposed!" );
		}
	}

	this->component( component );

	DataSection::iterator iter = pSection->begin();

	while (iter != pSection->end())
	{
		DataSectionPtr pDS = *iter;
		const std::string & sectionName = pDS->sectionName();
		if (sectionName == "Arg")
		{
			DataTypePtr pDataType = DataType::buildDataType( *iter );

			if (!pDataType)
			{
				ERROR_MSG( "MethodDescription::parse: "
						"Arg %s of %s is invalid\n",
					(*iter)->asString().c_str(), name_.c_str() );
				result = false;
			}

			args_.push_back( pDataType );
		}
		else if (sectionName == "ReturnValues")
		{
			if (!this->parseReturnValues( pDS ))
			{
				return false;
			}
		}

		iter++;
	}

	priority_ = pSection->readFloat( "DetailDistance", FLT_MAX );

	if (priority_ != FLT_MAX)
	{
		priority_ *= priority_;
	}

	return result;
}


/**
 *	This method parses a method description's return values, if it has any.
 *	The presence of return values indicates that the method is two-way. No
 *	return values indicates a one-way call.
 *
 *	@param	pSection	the data section of the method's ReturnValues
 */
bool MethodDescription::parseReturnValues( DataSectionPtr pSection )
{
	DataSection::iterator iter = pSection->begin();

	std::set<std::string> addedReturnValueNames;
	returnValues_.clear();
	while (iter != pSection->end())
	{
		DataSectionPtr pDS = *iter;

		if (addedReturnValueNames.find(pDS->sectionName()) !=
				addedReturnValueNames.end())
		{
			ERROR_MSG( "Return value %s of %s is duplicated\n",
				pDS->sectionName().c_str(), name_.c_str() );
			returnValues_.clear();
			return false;
		}

		DataTypePtr pDataType = DataType::buildDataType( pDS );
		if (!pDataType.hasObject())
		{
			ERROR_MSG( "Return value %s of %s is invalid\n",
				pDS->sectionName().c_str(), name_.c_str() );
			returnValues_.clear();
			return false;
		}


		addedReturnValueNames.insert( pDS->sectionName() );
		returnValues_.push_back(
			MethodDescription::ReturnValue( pDS->sectionName(), pDataType ) );

		++iter;
	}
	return true;
}


/**
 *	This method checks whether the input tuple of arguments are valid
 *	to call this method with (by adding them to a stream)
 *	@see addToStream
 *
 *	@param args		A Python tuple object containing the arguments.
 *	@param exposedExplicit		Flag indicating whether or not the
 *				source for exposed methods is specified explicitly
 *	@param generateException	Flag indicating whether or not to
 *				generate a python exception describing the problem
 *				if the arguments are not valid.
 *
 *	@return	True if the arguments are valid.
 */
bool MethodDescription::areValidArgs( bool exposedExplicit, PyObject * args,
	bool generateException ) const
{
	// check we have a tuple (against programmer error)
	if (!PyTuple_Check( args ))
	{
		ERROR_MSG( "MethodDescription::addToStream: args is not a tuple\n" );

		if (generateException) PyErr_SetString( PyExc_SystemError,
			"MethodDescription::areValidArgs not passed a tuple!" );

		return false;
	}

	// see what the first ordinary argument index should be
	int firstOrdinaryArg = 0;
	if (exposedExplicit && this->isExposed() && this->component() == CELL)
		firstOrdinaryArg = 1;

	int expectedArgs = args_.size() + firstOrdinaryArg;

	// make sure we have the right number (no default arguments yet)
	// if (PyTuple_Size( args ) - firstOrdinaryArg != (int)args_.size())
	if (PyTuple_Size( args ) != expectedArgs)
	{
		if (generateException)
		{
			PyErr_Format( PyExc_TypeError,
				"Method %s requires exactly %d argument%s; %"PRIzd" given",
				name_.c_str(),
				expectedArgs,
				(expectedArgs == 1) ? "" : "s",
				PyTuple_Size( args ) );
		}

		return false;
	}

	// check the exposed parameter if it is explicit
	if (firstOrdinaryArg == 1)
	{
		PyObject * pExposed = PyTuple_GetItem( args, 0 );
		if (!PyInt_Check( pExposed ) && pExposed != Py_None)
		{
			PyObject * peid = PyObject_GetAttrString( pExposed, "id" );
			if (peid == NULL || !PyInt_Check( peid ))
			{
				Py_XDECREF( peid );
				if (generateException)
				{
					PyErr_Format( PyExc_TypeError,
						"Method %s requires None, an id, or an object with an "
						"id as its first argument", name_.c_str() );
				}
				else
				{
					PyErr_Clear();
				}
				return false;
			}
			Py_DECREF( peid );
		}
	}

	// check each argument
	int numArgs = args_.size();
	for (int i = 0; i < numArgs; i++)
	{
		// pArg is a borrowed reference.
		PyObject * pArg = PyTuple_GetItem( args, i + firstOrdinaryArg );

		if (!args_[i]->isSameType( pArg ))
		{
			if (generateException)
			{
				PyObjectPtr pExample = args_[i]->pDefaultValue();

				if (pExample->ob_type == pArg->ob_type)
				{
					PyObjectPtr pStr( PyObject_Str( pArg ),
							PyObjectPtr::STEAL_REFERENCE );
					PyErr_Format( PyExc_TypeError,
						"Method %s, argument %d: Expected %s, got value %s",
						name_.c_str(),
						i+1,
						args_[i]->typeName().c_str(),
						PyString_AsString( pStr.get() ) );
				}
				else
				{
					PyErr_Format( PyExc_TypeError,
						"Method %s, argument %d: Expected %s, %s found",
						name_.c_str(),
						i+1,
						args_[i]->typeName().c_str(),
						pArg->ob_type->tp_name );
				}
			}

			return false;
		}
	}

	return true;
}


/**
 *	This method adds a method to the input stream. It assumes that the
 *	arguments are valid, according to 'areValidArgs'. The method header
 *	will already have been put onto the stream by the time this is called,
 *	so we have to complete the message even if we put on garbage.
 *	@see areValidArgs
 *
 *	@param isFromServer Indicates whether or not we are being called in a
 * 					server component.
 *	@param args		A Python tuple object containing the arguments.
 *	@param stream	The stream to which the method call should be added.
 *
 *	@return	True if successful.
 */
bool MethodDescription::addToStream( bool isFromServer,
	PyObject * args, BinaryOStream & stream ) const
{
	int firstOrdinaryArg = 0;

	// add the id of the object if it's not the client talking to the cell
	if (this->isExposed() && isFromServer && this->component() == CELL)
	{
		PyObject * pExposed = PyTuple_GetItem( args, 0 );
		firstOrdinaryArg = 1;

		EntityID eid = 0;
		if (PyInt_Check( pExposed ))
		{
			Script::setData( pExposed, eid );
		}
		else if (pExposed != Py_None)
		{
			// For convenience, can also pass the entity as the first argument
			// instead of the id.
			PyObject * peid = PyObject_GetAttrString( pExposed, "id" );
			if (peid != NULL)
			{
				Script::setData( peid, eid );
				Py_DECREF( peid );
			}
		}
		if (PyErr_Occurred())
		{
			ERROR_MSG( "MethodDescription::addToStream: "
				"Exposed object has no 'id' attribute, passing 0");
			PyErr_Clear();
		}

		stream << eid;
	}

	// Server to server method calls don't expect the sub-index to be on the
	// stream. Only Client-Server method calls use sub-indexing.
	bool isToClient = (this->component() == CLIENT);
	if (isFromServer == isToClient)
	{
		if (this->isExposed() && exposedSubIndex_ >= 0)
		{
			stream << uint8(exposedSubIndex_);
		}
	}

	int numArgs = args_.size();

	for (int i = 0; i < numArgs; i++)
	{
		// PyTuple_GetItem returns a borrowed reference.
		args_[i]->addToStream(
			PyTuple_GetItem( args, firstOrdinaryArg + i ), stream, false );
	}

	return true;
}


/**
 *	This method calls the method described by this object.
 *
 *	@param self the object whose method is called
 *	@param data the incoming data stream
 *	@param sourceID the source object ID if it came from a client
 *	@param replyID the reply ID, -1 if none
 *	@param pReplyAddr the reply address or NULL if none
 *	@param pInterface the nub to use, or NULL if none
 *
 *	@return		True if successful, otherwise false.
 */
bool MethodDescription::callMethod(PyObject * self,
	BinaryIStream & data,
	EntityID sourceID,
	int replyID,
	const Mercury::Address* pReplyAddr,
	Mercury::NetworkInterface * pInterface ) const
{
//	TRACE_MSG( "MethodDescription::callMethod: %s\n", name_.c_str() );

	bool ret = true;

#if ENABLE_WATCHERS
	this->stats().countReceived( data.remainingLength() );
#endif

	PyObjectPtr spTuple( this->getArgsAsTuple( data, sourceID,
		replyID, pReplyAddr, pInterface ) );

	// If we couldn't stream off the args correctly, abort this method call
	if (spTuple.getObject() == NULL)
	{
		ERROR_MSG( "MethodDescription::callMethod: "
				   "Couldn't stream off args for %s correctly, "
				   "aborting method call!\n", name_.c_str() );
		return false;
	}

	PyObject * pFunction =
		PyObject_GetAttrString( self, const_cast<char*>(name_.c_str()) );

	if (pFunction != NULL)
	{
		Py_INCREF( spTuple.getObject() );
#if ENABLE_WATCHERS
		uint64 curTime = timestamp();
#endif
		std::string errorPrefix =
				"MethodDescription::callMethod: Script call to '" + name_ +
				"' failed. Please check SCRIPT log output for script errors:\n";
		Script::call( pFunction, spTuple.getObject(), errorPrefix.c_str() );
#if ENABLE_WATCHERS
		timeSpent_ += timestamp() - curTime;
		timesCalled_ ++;
#endif
	}
	else
	{
		PyErr_Clear();
		// This warning is disable because it generates loads of spew in 'bots'
		// for all the unimplemented methods.  If you really have an
		// unimplemented method in a non-bots process you will get a warning on
		// startup so you should be able to live without this extra warning
		// anyway.
//  		ERROR_MSG( "MethodDescription::callMethod: "
// 			"No such method %s in object at 0x%08X\n", name_.c_str(), self );
		ret = false;
	}

	return ret;
}

/**
 *	This returns a tuple containing the arguments of the method described by
 *  this class.
 *
 *	@param data	A stream containing the arguments for the call.
 *	@param sourceID	The ID to pass as the source of exposed method calls.
 *	@param	replyID	the reply ID, -1 if none
 *	@param	pReplyAddr	pointer to the reply address, or NULL if none
 *	@param	pInterface	pointer to the interface to use for replies, or NULL if none
 *	@return	A PyObjectPtr containing the arguments as a tuple.
 */
PyObjectPtr MethodDescription::getArgsAsTuple( BinaryIStream & data,
	EntityID sourceID,
	int replyID,
	const Mercury::Address* pReplyAddr,
	Mercury::NetworkInterface * pInterface ) const
{
	int numArgs = args_.size();
	int tupleSize = numArgs
		+ int(this->isExposed() && this->component() == CELL)
		+ int(returnValues_.size() > 0);

	PyObjectPtr spTuple( PyTuple_New( tupleSize ),
						PyObjectPtr::STEAL_REFERENCE );
	// For convenience.
	PyObject * pTuple = spTuple.getObject();

	int argAt = 0;

	if (this->isExposed() && this->component() == CELL)
	{
		/*
		EntityID eid;
		if (exposedExpected)
			data >> eid;
		else
			eid = 0;

		PyTuple_SET_ITEM( pTuple, argAt++, Script::getData( eid ) );
		*/
		PyTuple_SET_ITEM( pTuple, argAt++, Script::getData( sourceID ) );
	}

	// if we have return values, create and add a MethodResponse object to the
	// args
	if (returnValues_.size() > 0)
	{
		if (replyID == -1)
		{
			// treat as one way method call by passing None to the function as
			// the response object
			Py_INCREF( Py_None );
			PyTuple_SET_ITEM( pTuple, argAt++, Py_None );
		}
		else
		{
			PyObject* responseObj = new MethodResponse( replyID,
				*pReplyAddr, *pInterface, *this );

			PyTuple_SET_ITEM( pTuple, argAt++, responseObj );

		}
	}

	for (int i = 0; i < numArgs; i++)
	{
		PyObjectPtr pArg = args_[i]->createFromStream( data, false );

		if (pArg && !data.error())
		{
			Py_INCREF( pArg.get() );
			PyTuple_SET_ITEM( pTuple, argAt++, pArg.get() );
		}

		// If for any reason we couldn't stream the argument off correctly, we
		// want to abort this method call, so just return NULL now
		else
		{
			ERROR_MSG( "MethodDescription::getArgsAsTuple: "
					   "Failed to get arg %d (of type %s) for method %s from "
					   "the stream.\n",
					   i, args_[i]->typeName().c_str(), name_.c_str() );
			PyErr_Print();
			return PyObjectPtr();
		}
	}

	// Check whether the entire stream is consumed
	// NOTE: This means multiple method calls/stream are not supported
	if (data.remainingLength() > 0)
	{
		ERROR_MSG( "MethodDescription::getArgsAsTuple: CHEAT: "
					 "Data still remains on stream after all args have been "
					 "streamed off! (%d bytes remaining)\n",
					 data.remainingLength() );
		return PyObjectPtr();
	}

	return spTuple;
}


/**
 *	Returns the number of return values for this method.
 *
 *	@return the number of return values for this method.
 */
uint MethodDescription::returnValues() const
{
	return returnValues_.size();
}


/**
 *	Returns the return value key name for the given index.
 *
 *	@param index the return value index
 *	@return the name of the return value
 */
const std::string& MethodDescription::returnValueName( uint index ) const
{
	return returnValues_[index].first;
}


/**
 *	Returns the return value data type for the given index.
 *
 *	@param index the return value index
 *	@return a pointer to the data type, or NULL if it does not exist.
 */
DataTypePtr MethodDescription::returnValueType( uint index ) const
{
	return returnValues_[index].second;
}


/**
 *	This method adds this object to the input MD5 object.
 */
void MethodDescription::addToMD5( MD5 & md5, int legacyExposedIndex ) const
{
	md5.append( name_.c_str(), name_.size() );
	md5.append( &flags_, sizeof( flags_ ) );

	Args::const_iterator argsIter = args_.begin();

	while (argsIter != args_.end())
	{
		if (*argsIter)
		{
			(*argsIter)->addToMD5( md5 );
		}

		argsIter++;
	}

	md5.append( &legacyExposedIndex, sizeof( legacyExposedIndex ) );
}

bool MethodDescription::hasPythonArg() const
{
	static const std::string PYTHON_TYPENAME = std::string( "PYTHON" );

	Args::const_iterator iArg = args_.begin();
	while (iArg != args_.end())
	{
		if ((*iArg)->typeName() == PYTHON_TYPENAME)
		{
			return true;
		}
		++iArg;
	}
	return false;
}

#if ENABLE_WATCHERS
/**
 * 	This method returns the generic watcher for Method Descriptions.
 */
WatcherPtr MethodDescription::pWatcher()
{
	static WatcherPtr watchMe = NULL;

	if (watchMe == NULL)
	{
		MethodDescription *pNull = NULL;
		watchMe = new DirectoryWatcher();

		watchMe->addChild( "name", makeWatcher( pNull->name_ ) );
		watchMe->addChild( "priority", makeWatcher( pNull->priority_ ) );
		watchMe->addChild( "internalIndex", 
						   makeWatcher( pNull->internalIndex_ ) );
		watchMe->addChild( "exposedIndex", 
						   makeWatcher( pNull->exposedIndex_ ) );
		watchMe->addChild( "timeSpent", makeWatcher( pNull->timeSpent_ ) );	
		watchMe->addChild( "timesCalled", makeWatcher( pNull->timesCalled_ ) );	
		watchMe->addChild( "stats", EntityMemberStats::pWatcher(), 
						   &pNull->stats_ );
	}
	return watchMe;
}

#endif
// method_description.cpp
