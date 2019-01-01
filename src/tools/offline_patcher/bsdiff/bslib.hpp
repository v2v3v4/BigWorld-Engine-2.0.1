/*
 * Parts of this library are derived from Colin Percival's bsdiff tool.
 *
 * Copyright 2003-2005 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BSLIB_HPP__
#define __BSLIB_HPP__


/**
 *	Error codes.
 */
enum BSLIB_ERROR
{
	BSLIB_OK 			= 0, 	// Success
	BSLIB_IO_ERROR,				// I/O error
	BSLIB_MEM_ERROR,			// Memory error
	BSLIB_PARAM_ERROR,			// Parameter error
	BSLIB_BZ2_ERROR,			// bzip2 error
	BSLIB_BAD_PATCH_ERROR,		// Bad patch error
	BSLIB_UNKNOWN_ERROR,		// Unknown error

	BSLIB_NUM_ERRORS
};

typedef long long bs_off_t;

const char * bsdiff_errString( int errcode );

const char * bsdiff_errDescription();

int bsdiff_diff( const u_char * src, bs_off_t srcLen,
	const u_char * dst, bs_off_t dstLen,
	u_char ** pControlBlock, bs_off_t * pControlLen,
	u_char ** pDiffBlock, bs_off_t * pDiffLen,
	u_char ** pExtrasBlock, bs_off_t * pExtrasLen );

int bsdiff_blocksToPatchFile( bs_off_t dstLen,
	const u_char * controlBlock, bs_off_t controlLen,
	const u_char * diffBlock, bs_off_t diffLen,
	const u_char * extrasBlock, bs_off_t extrasLen,
	const char * patchPath );

int bsdiff_diffToPatchFile( const char * src, bs_off_t srcLen,
		const char * dst, bs_off_t dstLen,
		const char * patchPath );


int bsdiff_patchFromPatchFile( const char * patchPath,
		FILE * srcFile,
		u_char ** pDstBytes,
		bs_off_t * pDstBytesLen );

#endif // #define __BSLIB_HPP__
