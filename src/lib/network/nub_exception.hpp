/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NUB_EXCEPTION_HPP
#define NUB_EXCEPTION_HPP

#include "basictypes.hpp"

namespace Mercury
{

/**
 *	This is the base class for all exception types that are thrown by various
 *	Mercury classes.
 *
 *	@ingroup mercury
 */
class NubException
{
public:
	NubException( Reason reason, const Address & addr = Address::NONE );
	virtual ~NubException() {};
	Reason reason() const;
	bool getAddress( Address & addr ) const;

private:
	Reason		reason_;
	Address address_;
};

#include "nub_exception.ipp"

} // namespace Mercury

#endif // NUB_EXCEPTION_HPP
