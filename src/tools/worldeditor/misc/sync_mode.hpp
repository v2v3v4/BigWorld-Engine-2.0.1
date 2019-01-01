/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SYNC_MODE_HPP
#define SYNC_MODE_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


/**
 *  This object gives exclusive access to chunks while it is in scope.
 */
class SyncMode
{
public:
    explicit SyncMode();
    ~SyncMode();

    operator bool() const;

private:
    SyncMode(SyncMode const &);
    SyncMode &operator=(SyncMode const &);

private:
    bool            readOnly_;
};


#endif // ELEVATION_MODIFY_HPP
