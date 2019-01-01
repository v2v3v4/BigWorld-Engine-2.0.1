/**
 * This file was automatically generated from the PHP tool ext_skel.
 */

#ifndef PHP_BIGWORLD_H
#define PHP_BIGWORLD_H

#include <string>
#include <map>

extern zend_module_entry bigworld_php_module_entry;
#define phpext_bigworld_php_ptr &bigworld_php_module_entry

#ifdef PHP_WIN32
#	define PHP_BIGWORLD_PHP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_BIGWORLD_PHP_API __attribute__ ((visibility("default")))
#else
#	define PHP_BIGWORLD_PHP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION( bigworld_php );
PHP_MSHUTDOWN_FUNCTION( bigworld_php );
PHP_RINIT_FUNCTION( bigworld_php );
PHP_RSHUTDOWN_FUNCTION( bigworld_php );
PHP_MINFO_FUNCTION( bigworld_php );

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:
*/
//*
ZEND_BEGIN_MODULE_GLOBALS( bigworld_php )
	/** Global Python module instance */
	const char * 	addPythonPaths;
	uid_t 			uid;
	int				debugLevel;
	PyObject * 		bwModule;
ZEND_END_MODULE_GLOBALS( bigworld_php )
//*/

/* In every utility function you add that needs to use variables
   in php_bigworld_globals, call TSRMLS_FETCH(); after declaring other
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as BWG(variable).  You are
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define BWG(v) TSRMG(bigworld_php_globals_id, zend_bigworld_php_globals *, v)
#else
#define BWG(v) (bigworld_php_globals.v)
#endif

#endif	/* PHP_BIGWORLD_H */
