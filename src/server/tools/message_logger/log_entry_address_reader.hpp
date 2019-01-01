/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOG_ENTRY_ADDRESS_READER_HPP
#define LOG_ENTRY_ADDRESS_READER_HPP

#include "log_entry_address.hpp"

#include "Python.h"

class BinaryIStream;

#pragma pack( push, 1 )
class LogEntryAddressReader : public LogEntryAddress
{
public:
	bool isValid() const;

	bool fromPyTuple( PyObject *tuple );
};
#pragma pack( pop )

#endif // LOG_ENTRY_ADDRESS_READER_HPP
