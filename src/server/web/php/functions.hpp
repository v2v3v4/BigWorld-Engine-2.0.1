/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "php.h"
#include "php_bigworld.h"

// ----------------------------------------------------------------------------
// Section: PHP Resource Destruction handlers
// ----------------------------------------------------------------------------

/**
 *	The destructing function for a Python object PHP resource.
 */
void PyObject_ResourceDestructionHandler( zend_rsrc_list_entry *rsrc
	TSRMLS_DC );

// ----------------------------------------------------------------------------
// Section: Globals
// ----------------------------------------------------------------------------
ZEND_EXTERN_MODULE_GLOBALS( bigworld_php )

// ----------------------------------------------------------------------------
// Section: PHP API function declarations
// ----------------------------------------------------------------------------

PHP_FUNCTION( bw_logon );
PHP_FUNCTION( bw_test );
PHP_FUNCTION( bw_look_up_entity_by_name );
PHP_FUNCTION( bw_look_up_entity_by_dbid );
PHP_FUNCTION( bw_exec );
PHP_FUNCTION( bw_reset_network_interface );
PHP_FUNCTION( bw_serialise );
PHP_FUNCTION( bw_deserialise );
PHP_FUNCTION( bw_pystring );
PHP_FUNCTION( bw_set_keep_alive_seconds );
PHP_FUNCTION( bw_get_keep_alive_seconds );
PHP_FUNCTION( bw_set_default_keep_alive_seconds );

#endif // ifndef FUNCTIONS_HPP
