/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RELIABLE_ORDER_HPP
#define RELIABLE_ORDER_HPP

#include "cstdmf/stdmf.hpp"

#include <vector>

namespace Mercury
{

/**
 * 	@internal
 * 	This structure is used to describe a reliable message. When
 * 	a message is added to a Bundle, it is streamed onto the end
 * 	of the last packet, and it is not easy to extract it. However
 * 	when a packet containing reliable data is dropped on a
 * 	connection between client and server, only the reliable data
 * 	is resent. The ReliableOrder structure is used to extract
 *	the reliable messages from a bundle that has already been
 *	sent.
 */
class ReliableOrder
{
public:
	uint8	* segBegin;				///< Pointer to the reliable segment
	uint16	segLength;				///< Length of the segment
	uint16 	segPartOfRequest;		///< True if it is part of a request
};

/**
 * 	@internal
 * 	The ReliableVector type is just a vector of ReliableOrders.
 */
typedef std::vector<ReliableOrder> ReliableVector;

} // namespace Mercury

#endif // RELIABLE_ORDER_HPP
