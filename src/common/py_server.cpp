/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "cstdmf/watcher.hpp"
#include "pyscript/script.hpp"
#include "server/bwconfig.hpp"


/*~ module BWConfig
 *	@components{ base, cell, db }
 */

// -----------------------------------------------------------------------------
// Section: Method definitions
// -----------------------------------------------------------------------------

/*~ function BWConfig.readString
 *	@components{ base, cell, db }
 *
 *	This method returns the given value from bw.xml.
 *
 *	@param	configOption	The path of the option to get.
 */
static PyObject * readString( const std::string & configOption,
		const std::string & defaultValue )
{
	return Script::getData( BWConfig::get( configOption.c_str(),
				defaultValue ) );
}
PY_AUTO_MODULE_FUNCTION( RETOWN, readString,
		ARG( std::string, OPTARG( std::string, "", END ) ), BWConfig )


/*~ function BWConfig.readInt
 *	@components{ base, cell, db }
 *
 *	This method returns the given value from bw.xml.
 *
 *	@param	configOption	The path of the option to get.
 */
static PyObject * readInt( const std::string & configOption,
		int defaultValue )
{
	return Script::getData( BWConfig::get( configOption.c_str(),
				defaultValue ) );
}
PY_AUTO_MODULE_FUNCTION( RETOWN, readInt,
		ARG( std::string, OPTARG( int, 0, END ) ), BWConfig )


/*~ function BWConfig.readFloat
 *	@components{ base, cell, db }
 *
 *	This method returns the given value from bw.xml.
 *
 *	@param	configOption	The path of the option to get.
 */
static PyObject * readFloat( const std::string & configOption,
		double defaultValue )
{
	return Script::getData( BWConfig::get( configOption.c_str(),
				defaultValue ) );
}
PY_AUTO_MODULE_FUNCTION( RETOWN, readFloat,
		ARG( std::string, OPTARG( double, 0.0, END ) ), BWConfig )


/*~ function BWConfig.readBool
 *	@components{ base, cell, db }
 *
 *	This method returns the given value from bw.xml.
 *
 *	@param	configOption	The path of the option to get.
 */
static PyObject * readBool( const std::string & configOption,
		bool defaultValue )
{
	return Script::getData( BWConfig::get( configOption.c_str(),
				defaultValue ) );
}
PY_AUTO_MODULE_FUNCTION( RETOWN, readBool,
		ARG( std::string, OPTARG( bool, false, END ) ), BWConfig )


// py_server.cpp
