/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WATCHER_FORWARDING_TYPES_HPP
#define WATCHER_FORWARDING_TYPES_HPP

#include <utility>


// At present, ComponentID needs to handle both a CellAppID and BaseAppID
typedef int32 ComponentID;
typedef std::vector<ComponentID> ComponentIDList;
typedef std::pair<Mercury::Address,ComponentID> AddressPair;
typedef std::vector<AddressPair> AddressList;

#endif
