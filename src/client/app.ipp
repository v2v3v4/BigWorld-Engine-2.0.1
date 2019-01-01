/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// app.ipp

#ifdef CODE_INLINE
    #define INLINE    inline
#else
    #define INLINE
#endif



/**
 *	This method gets the time that the last frame took
 */
INLINE
float App::getTimeDelta( void ) const
{
	return dTime_;
}


/**
 *	This method gets the time that the client has been running for
 */
INLINE
double App::getTime( void ) const
{
	return totalTime_;
}


/**
 * This method returns the compile time string for the application.
 */
INLINE
const char * App::compileTime() const
{
	return compileTime_.c_str();
}

// app.ipp
