/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOGIN_REPLY_RECORD_HPP
#define LOGIN_REPLY_RECORD_HPP

#include "cstdmf/stdmf.hpp"
#include "cstdmf/memory_stream.hpp"

#include "network/basictypes.hpp"

/**
 * 	This structure contains the reply from a successful login.
 */
struct LoginReplyRecord
{
	Mercury::Address	serverAddr;			// send to here
	uint32				sessionKey;			// use this session key
};

inline BinaryIStream& operator>>(
	BinaryIStream &is, LoginReplyRecord &lrr )
{
	return is >> lrr.serverAddr >> lrr.sessionKey;
}

inline BinaryOStream& operator<<(
	BinaryOStream &os, const LoginReplyRecord &lrr )
{
	return os << lrr.serverAddr << lrr.sessionKey;
}

#endif // LOGIN_REPLY_RECORD_HPP
