/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "db_interface_utils.hpp"

#include "dbmgr/db_interface.hpp"

#include "cstdmf/blob_or_null.hpp"
#include "cstdmf/debug.hpp"
#include "network/channel_sender.hpp"
#include "network/nub_exception.hpp"

DECLARE_DEBUG_COMPONENT(0)

namespace DBInterfaceUtils
{
	/*~ function BigWorld.executeRawDatabaseCommand
	 *	@components{ base }
	 *	This script function executes the given raw database command on the
	 *	database. There is no attempt to parse the command - it is passed
	 * 	straight through to the underlying database. This feature is not
	 * 	supported for the XML database.
	 *
	 *  Please note that the entity data in the database may not be up to date,
	 * 	especially when secondary databases are enabled. It is highly
	 *  recommended that this function is not used to read or modify entity
	 * 	data.
	 *
	 *	@param command The command string specific to the present database layer.
	 *  For MySQL database it is an SQL query.
	 *	@param callback The object to call back (e.g. a function) with the result
	 *	of the command execution. The callback will be called with 3
	 * 	parameters: result set, number of affected rows and error string.
	 *
	 * 	The result set parameter is a list of rows. Each row is a list of
	 * 	strings containing field values. The result set will be None for commands
	 * 	that do not return a result set e.g. DELETE, or if there was an error in
	 * 	executing the command.
	 *
	 * 	The number of affected rows parameter is a number indicating the
	 * 	number of rows affected by the command. This parameter is only relevant
	 * 	for commands to do not return a result set e.g. DELETE. This parameter
	 * 	is None for commands that do return a result set or if there was and
	 * 	error in executing the command.
	 *
	 * 	The error string parameter is a string describing the error that
	 * 	occurred if there was an error in executing the command. This parameter
	 * 	is None if there was no error in executing the command.
	 */
	/**
	 * 	NOTE: The comment block below is a copy of the comment block above with
	 * 	an additional note for cell entity callbacks.
	 */
	/*~ function BigWorld.executeRawDatabaseCommand
	 *	@components{ cell }
	 *	This script function executes the given raw database command on the
	 *	database. There is no attempt to parse the command - it is passed
	 * 	straight through to the underlying database. This feature is not
	 * 	supported for the XML database.
	 *
	 *	@param command The command string specific to the present database layer.
	 *  For MySQL database it is an SQL query.
	 *	@param callback The object to call back (e.g. a function) with the result
	 *	of the command execution. The callback will be called with 3
	 * 	parameters: result set, number of affected rows and error string.
	 *
	 * 	The result set parameter is a list of rows. Each row is a list of
	 * 	strings containing field values. The result set will be None for commands
	 * 	that do not return a result set e.g. DELETE, or if there was an error in
	 * 	executing the command.
	 *
	 * 	The number of affected rows parameter is a number indicating the
	 * 	number of rows affected by the command. This parameter is only relevant
	 * 	for commands that do not return a result set e.g. DELETE. This parameter
	 * 	is None for commands that return a result set or if there was an error
	  	in executing the command.
	 *
	 * 	The error string parameter is a string describing the error that
	 * 	occurred if there was an error in executing the command. This parameter
	 * 	is None if there was no error.
	 *
	 * 	Please note that due to the itinerant nature of cell entities, the
	 * 	callback function should handle cases where the cell
	 * 	entity has been converted to a ghost or has been destroyed. In general,
	 * 	the callback function can be a simple function that calls an entity
	 * 	method defined in the entity definition file to do the actual work.
	 * 	This way, the call will be forwarded to the real entity if the current
	 * 	entity has been converted to a ghost.
	 */
	/**
	 *	This class handles the response from DbMgr to an executeRawCommand
	 * 	request.
	 */
	class ExecRawDBCmdWaiter : public Mercury::ReplyMessageHandler
	{
	public:
		ExecRawDBCmdWaiter( PyObjectPtr pResultHandler ) :
			pResultHandler_( pResultHandler.getObject() )
		{
			Py_XINCREF( pResultHandler_ );
		}

		// Mercury::ReplyMessageHandler overrides
		virtual void handleMessage(const Mercury::Address& source,
			Mercury::UnpackedMessageHeader& header,
			BinaryIStream& data, void * arg)
		{
			TRACE_MSG( "ExecRawDBCmdWaiter::handleMessage: "
				"DB call response received\n" );

			this->processTabularResult( data );
		}

		void processTabularResult( BinaryIStream& data )
		{
			PyObject* pResultSet;
			PyObject* pAffectedRows;
			PyObject* pErrorMsg;

			std::string errorMsg;
			data >> errorMsg;

			if (errorMsg.empty())
			{
				pErrorMsg = this->newPyNone();

				uint32 numColumns;
				data >> numColumns;

				if (numColumns > 0)
				{	// Command returned tabular data.
					pAffectedRows = this->newPyNone();

					uint32 numRows;
					data >> numRows;
					// Make list of list of strings.
					pResultSet = PyList_New( numRows );
					for ( uint32 i = 0; i < numRows; ++i )
					{
						PyObject* pRow = PyList_New( numColumns );
						for ( uint32 j = 0; j < numColumns; ++j )
						{
							BlobOrNull cell;
							data >> cell;

							PyObject* pCell = (cell.isNull()) ?
									this->newPyNone() :
									PyString_FromStringAndSize( cell.pData(),
												cell.length() );
							PyList_SET_ITEM( pRow, j, pCell );
						}
						PyList_SET_ITEM( pResultSet, i, pRow );
					}
				}
				else
				{	// Empty result set - only affected rows returned.
					uint64	numAffectedRows;
					data >> numAffectedRows;

					pResultSet = this->newPyNone();
					pAffectedRows = Script::getData( numAffectedRows );
					pErrorMsg = this->newPyNone();
				}
			}
			else	// Error has occurred.
			{
				pResultSet = this->newPyNone();
				pAffectedRows = this->newPyNone();
				pErrorMsg = Script::getData( errorMsg );
			}

			this->done( pResultSet, pAffectedRows, pErrorMsg );
		}

		void handleException(const Mercury::NubException& exception, void* arg)
		{
			// This can be called during Channel destruction which can happen
			// after Script has been finalised.
			if (!Script::isFinalised())
			{
				std::stringstream errorStrm;
				errorStrm << "Nub exception " <<
						Mercury::reasonToString( exception.reason() );
				ERROR_MSG( "ExecRawDBCmdWaiter::handleException: %s\n",
						errorStrm.str().c_str() );
				this->done( this->newPyNone(), this->newPyNone(),
						Script::getData( errorStrm.str() ) );
			}
		}

	private:
		void done( PyObject * resultSet, PyObject * affectedRows,
				PyObject * errorMsg )
		{
			if (pResultHandler_)
			{
				Script::call( pResultHandler_,
					Py_BuildValue( "(OOO)", resultSet, affectedRows, errorMsg ),
					"ExecRawDBCmdWaiter callback", /*okIfFnNull:*/false );
				// 'call' does the decref of pResultHandler_ for us
			}

			Py_DECREF( resultSet );
			Py_DECREF( affectedRows );
			Py_DECREF( errorMsg );

			delete this;
		}

		static PyObject* newPyNone()
		{
			PyObject* pNone = Py_None;
			Py_INCREF( pNone );
			return pNone;
		}

		PyObject* 				pResultHandler_;
	};

	/**
	 *	This function sends a message to the DbMgr to an run an
	 * 	executeRawDatabaseCommand request. When the result is sent back from
	 * 	DbMgr, pResultHandler will be called if specified.
	 */
	bool executeRawDatabaseCommand( const std::string & command,
			PyObjectPtr pResultHandler, Mercury::Channel & channel )
	{
		if (pResultHandler && !PyCallable_Check( pResultHandler.get() ) )
		{
			PyErr_SetString( PyExc_TypeError,
				"BigWorld.executeRawDatabaseCommand() "
				"callback must be callable if specified" );
			return false;
		}

		Mercury::Bundle & bundle = channel.bundle();
		bundle.startRequest( DBInterface::executeRawCommand,
			new ExecRawDBCmdWaiter( pResultHandler ) );
		bundle.addBlob( command.data(), command.size() );

		channel.send();

		return true;
	}

	/**
	 *	This function sends a message to the DbMgr to an run an
	 * 	executeRawDatabaseCommand request. When the result is sent back from
	 * 	DbMgr, pResultHandler will be called if specified.
	 */
	bool executeRawDatabaseCommand( const std::string & command,
			PyObjectPtr pResultHandler, Mercury::NetworkInterface & interface,
			const Mercury::Address& dbMgrAddr )
	{
		if (pResultHandler && !PyCallable_Check( pResultHandler.get() ) )
		{
			PyErr_SetString( PyExc_TypeError,
				"BigWorld.executeRawDatabaseCommand() "
				"callback must be callable if specified" );
			return false;
		}

		Mercury::ChannelSender sender(
				interface.findOrCreateChannel( dbMgrAddr ) );
		Mercury::Bundle & bundle = sender.bundle();

		bundle.startRequest( DBInterface::executeRawCommand,
			new ExecRawDBCmdWaiter( pResultHandler ) );
		bundle.addBlob( command.data(), command.size() );

		return true;
	}
} // namespace DBInterfaceUtils

// db_interface_utils.cpp
