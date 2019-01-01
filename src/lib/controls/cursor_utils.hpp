/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CURSOR_UTILS_HPP
#define CURSOR_UTILS_HPP


namespace controls
{

// IMPORTANT: The caller is responsible for deleting the returned HCURSOR.
HCURSOR addPlusSignToCursor( HCURSOR origCursor );

// IMPORTANT: The caller is responsible for deleting the returned HCURSOR.
HCURSOR addOverlayToCursor( HCURSOR origCursor, unsigned char * bitmap, unsigned char * mask );

}; // namespace controls


#endif // CURSOR_UTILS_HPP
