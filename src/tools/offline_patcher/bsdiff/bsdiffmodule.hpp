#ifndef __BSDIFFMODULE_HPP__
#define __BSDIFFMODULE_HPP__

#include "pch.hpp"

#ifdef _WIN32
#	ifdef _MANAGED
#		pragma managed(push, off)
#	endif
#endif

#include "bslib.hpp"

#ifdef _WIN32
#	if defined( BSDIFF_EXPORTS )
#		define BSDIFF_API __declspec(dllexport)
#	elif defined( BSDIFF_IMPORTS )
#		define BSDIFF_API __declspec(dllimport)
#	else
#		pragma message( "BSDIFF_IMPORTS or BSDIFF_EXPORTS should be defined" )
#	endif
#else
#	define BSDIFF_API
#endif

static PyObject * PyExc_BSLibException;

extern "C" BSDIFF_API void init_bsdiff( void );

#endif // __BSDIFFMODULE_HPP__
