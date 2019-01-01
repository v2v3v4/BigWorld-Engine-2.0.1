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
#include "pch.hpp"
#include "bslib.hpp"

#include "bzlib.h"

#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stack>
#include <string>
#include <vector>

#ifndef min
#define min std::min
#endif


namespace // (anonymous)
{

// ----------------------------------------------------------------------------
// Section: Error detection declarations
// ----------------------------------------------------------------------------

#ifdef _WIN32
#	define snprintf _snprintf
#endif

#define BSDIFF_ERROR( type, ... ) \
	{ \
		char _errBuf[1024]; \
		snprintf( _errBuf, 1024, __VA_ARGS__ ); \
		s_bsdiff_errDescription = _errBuf; \
		return type; \
	}

// macro for standard memory allocation error
#define BSDIFF_STD_MEM_ERROR \
	BSDIFF_ERROR( BSLIB_MEM_ERROR, "allocation failed" )



// last error description
std::string s_bsdiff_errDescription;

const char * BSLIB_ERROR_STRINGS[BSLIB_NUM_ERRORS] =
{
	"No error",
	"I/O error",
	"Memory error",
	"Parameter error",
	"libbz2 error",
	"Corrupt patch",
	"Unknown error"
};


/**
 *	Auto pointer, but for arrays.
 *
 */
template <typename T>
class ArrayAutoPtr
{
public:

	ArrayAutoPtr( T * pData ):
		_pData( pData ),
		_released( false )
	{}

	~ArrayAutoPtr()
	{
		if (!_released)
		{
			delete [] this->_pData;
		}
	}

	/**
	 *	Release this autopointer, so that it doesn't delete the array on
	 *	destruct.
	 */
	void release( bool released=true )
	{
		_released = released;
	}


private:
	T *		_pData;
	bool 	_released;
};


/**
 *	Auto pointer, for the opening and closing of files.
 */
class FileAutoPtr
{
public:
	FileAutoPtr( const char* path, const char *mode ):
		_fp( fopen( path, mode ) )
	{}

	FileAutoPtr( FILE * fp ):
		_fp( fp )
	{}

	FILE * fp() { return _fp; }

	~FileAutoPtr()
	{	if (_fp != NULL) fclose( this->_fp ); }

private:
	FILE * 	_fp;
};


/**
 *	Structure for holding local variables for a particular iteration of
 *	bsdiff_split().
 */
struct BSDiffSplitFrame
{
	// Input parameters to an iteration of bsdiff_split().
	bs_off_t start;
	bs_off_t len;

	// Local variables for an iteration of bsdiff_split().
	bs_off_t i;
	bs_off_t j;
	bs_off_t k;
	bs_off_t x;
	bs_off_t jj;
	bs_off_t kk;

	// Serves the same role as the EIP value pushed onto the stack for normal
	// function calls.
	int 		stage;
};


// control block used when constructing the control tuples for a patch file
typedef std::vector<u_char> ControlBlock;

// ----------------------------------------------------------------------------
// Section: Function declarations
// ----------------------------------------------------------------------------

bs_off_t bsdiff_matchlen( const u_char * src, bs_off_t srcLen,
	const u_char * dst, bs_off_t dstLen );

bs_off_t bsdiff_search( bs_off_t * sufTab,
	const u_char * src, bs_off_t srcLen,
	const u_char * search, bs_off_t searchLen,
	bs_off_t start, bs_off_t end, bs_off_t & pos );

void bsdiff_offtout( bs_off_t x, u_char * buf );

bs_off_t bsdiff_offtin( const u_char * buf );

void bsdiff_split( bs_off_t * sufTab, bs_off_t * sufTabInv,
	bs_off_t start, bs_off_t len, bs_off_t h );

void bsdiff_qsufsort( bs_off_t * sufTab, bs_off_t * sufTabInv,
		const u_char * s, bs_off_t len );


/**
 *	Part of the implementation of bsdiff_qsufsort.
 *
 *	Modified by BigWorld from the original implementation to make it
 *	non-recursive.
 *
 *	@todo clean up this code
 */
void bsdiff_split( bs_off_t * sufTab, bs_off_t * sufTabInv,
		bs_off_t start, bs_off_t len, bs_off_t h )
{
	// Our replacement stack. This is used to avoid stack overflows when
	// particularly troublesome files are encountered that required deep
	// recursion into the formerly recursive implementation of bsdiff_split().
	std::stack< BSDiffSplitFrame > stack;

	stack.push( BSDiffSplitFrame() );
	BSDiffSplitFrame & f = stack.top();

	f.start = start;
	f.len = len;
	f.stage = 0;

	while (!stack.empty())
	{
		BSDiffSplitFrame & f = stack.top();
		switch( f.stage )
		{

		case 0:
		{
			if (f.len < 16)
			{
				for (f.k = f.start; f.k < f.start + f.len; f.k += f.j)
				{
					f.j = 1;
					f.x = sufTabInv[ sufTab[f.k] + h ];

					for (f.i = 1; f.k + f.i < f.start + f.len; ++f.i)
					{
						if (sufTabInv[ sufTab[ f.k + f.i ] + h ] < f.x)
						{
							f.x = sufTabInv[ sufTab[ f.k + f.i ] + h ];
							f.j = 0;
						}

						if (sufTabInv[ sufTab[ f.k + f.i ] + h ] == f.x)
						{
							std::swap( sufTab[ f.k + f.j ], sufTab[ f.k + f.i ] );
							++f.j;
						}
					}

					for (f.i = 0; f.i < f.j; ++f.i)
					{
						sufTabInv[ sufTab[ f.k + f.i ] ] = f.k + f.j - 1;
					}

					if (f.j == 1)
					{
						sufTab[f.k] = -1;
					}

				}

				stack.pop();
				continue;
			}

			f.x = sufTabInv[ sufTab[ f.start + f.len / 2 ] + h ];
			f.jj = 0;
			f.kk = 0;
			for (f.i = f.start; f.i < f.start + f.len ; ++f.i)
			{
				if (sufTabInv[ sufTab[f.i] + h ] < f.x)
				{
					++f.jj;
				}

				if (sufTabInv[ sufTab[f.i] + h ] == f.x)
				{
					++f.kk;
				}
			}

			f.jj += f.start;
			f.kk += f.jj;

			f.i = f.start;
			f.j = 0;
			f.k = 0;

			while (f.i < f.jj)
			{
				if (sufTabInv[ sufTab[f.i] + h ] < f.x)
				{
					++f.i;
				}
				else if (sufTabInv[ sufTab[f.i] + h ] == f.x)
				{
					std::swap( sufTab[f.i], sufTab[ f.jj + f.j ] );
					++f.j;
				}
				else
				{
					std::swap( sufTab[f.i], sufTab[ f.kk + f.k ] );
					++f.k;
				}
			}

			while (f.jj + f.j < f.kk)
			{
				if (sufTabInv[ sufTab[ f.jj + f.j ] + h ] == f.x)
				{
					++f.j;
				}
				else
				{
					std::swap( sufTab[f.jj + f.j], sufTab[f.kk + f.k] );
					++f.k;
				}
			}

			if (f.jj > f.start)
			{
				stack.push( BSDiffSplitFrame() );
				stack.top().stage = 0;
				stack.top().start = f.start;
				stack.top().len = f.jj - f.start;
				f.stage = 1;
				continue;
			}
		} // end case 0

		case 1:
		{
			for (f.i = 0; f.i < f.kk - f.jj; ++f.i)
			{
				 sufTabInv[ sufTab[f.jj + f.i] ] = f.kk - 1;
			}

			if (f.jj == f.kk - 1)
			{
				sufTab[f.jj] = -1;
			}

			if (f.start + f.len > f.kk)
			{
				stack.push( BSDiffSplitFrame() );
				stack.top().stage = 0;
				stack.top().start = f.kk;
				stack.top().len = f.start + f.len - f.kk;
				f.stage = 2;
				continue;
			}
		}


		case 2:
		break;
		} // end switch

		stack.pop();
	}
}


/**
 *	Constructs a suffix table and its inverse for the given string 's'
 *
 *	@param sufTab		An array of offsets, must be preallocated of size
 *						(sLen + 1). This is filled with the suffix table for
 *						's'.
 *	@param sufTabInv	An array of offsets, must be preallocated of size
 *						(sLen + 1 ). This is filled with the inverse suffix
 *						table for 's'.
 *	@param s			The string to construct a suffix table for.
 *	@param sLen			The length of 's'.
 */
void bsdiff_qsufsort( bs_off_t * sufTab, bs_off_t * sufTabInv,
		const u_char * s, bs_off_t sLen )
{
	bs_off_t buckets[256];
	bs_off_t i,h,len;

	for(i=0;i<256;i++) buckets[i]=0;
	for(i=0;i<sLen;i++) buckets[s[i]]++;
	for(i=1;i<256;i++) buckets[i]+=buckets[i-1];
	for(i=255;i>0;i--) buckets[i]=buckets[i-1];
	buckets[0]=0;

	for(i=0;i<sLen;i++) sufTab[++buckets[s[i]]]=i;
	sufTab[0]=sLen;
	for(i=0;i<sLen;i++) sufTabInv[i]=buckets[s[i]];
	sufTabInv[sLen]=0;
	for(i=1;i<256;i++) if(buckets[i]==buckets[i-1]+1) sufTab[buckets[i]]=-1;
	sufTab[0]=-1;

	for(h=1;sufTab[0]!=-(sLen+1);h+=h) {
		len=0;
		for(i=0;i<sLen+1;) {
			if(sufTab[i]<0) {
				len-=sufTab[i];
				i-=sufTab[i];
			} else {
				if(len) sufTab[i-len]=-len;
				len=sufTabInv[sufTab[i]]+1-i;
				bsdiff_split(sufTab,sufTabInv,i,len,h);
				i+=len;
				len=0;
			};
		};
		if(len) sufTab[i-len]=-len;
	};

	for(i=0;i<sLen+1;i++) sufTab[sufTabInv[i]]=i;
}


/**
 *	Return the length of the common prefix of two strings, A and B.
 *
 *	@param src		string A
 *	@param srcLen	the length of string A
 *	@param dst		string B
 *	@param dstLen	the length of string B
 *	@return 		the length of the common prefix
 */
bs_off_t bsdiff_matchlen( const u_char * src, bs_off_t srcLen,
		const u_char * dst, bs_off_t dstLen )
{
	bs_off_t i;
	for (i = 0; i < srcLen && i < dstLen; ++i)
	{
		if (src[i] != dst[i])
		{
			break;
		}
	}

	return i;
}


/**
 *	Using a precomputed suffix table as an index into src, (binary) search src
 *	for search and return the position in src in the pos reference parameter
 *	and return the length of the match along 'search' as a return value.
 *
 *	@see bsdiff_qsufsort
 *
 *	@param sufTab		the suffix table generated from bsdiff_qsufsort
 *	@param src			the string to match against
 *	@param srcLen		the length of the string 'src'
 *	@param search		the search string
 *	@param searchLen	the length of the search string
 *	@param start		set to 0 in the initial invocation, used in recursive
 *						calls in the binary search
 *	@param end			set to srcLen in the initial invocation, used in
 *						recursive calls in the binary search.
 *	@param pos			return parameter for the position along src where the
 *						match occurs.
 *	@return				0 if no match, or the length of the match along
 *						'search'
 */
bs_off_t bsdiff_search( bs_off_t * sufTab,
		const u_char * src, bs_off_t srcLen,
		const u_char * search, bs_off_t searchLen,
		bs_off_t start, bs_off_t end, bs_off_t & pos )
{
	if (end - start < 2)
	{
		// end recursion case
		bs_off_t x = bsdiff_matchlen( src + sufTab[start],
			srcLen - sufTab[start], search, searchLen );
		bs_off_t y = bsdiff_matchlen( src + sufTab[end],
			srcLen - sufTab[end], search, searchLen );

		if (x > y)
		{
			pos = sufTab[start];
			return x;
		}
		else
		{
			pos = sufTab[end];
			return y;
		}
	}

	bs_off_t mid = start + (end - start) / 2; // midpoint
	if (memcmp( src + sufTab[mid], search,
			min( srcLen - sufTab[mid], searchLen ) ) < 0)
	{
		// search high
		return bsdiff_search( sufTab, src, srcLen,
			search, searchLen, mid, end, pos );
	}
	else
	{
		// search low
		return bsdiff_search( sufTab, src, srcLen,
			search, searchLen, start, mid, pos );
	}
}


/**
 *	Takes a offset (64-bit) and marshals it to 8-bytes in buf. Taken directly
 *	from implementation in bsdiff.
 *
 *	@param x		the offset to marshal
 *	@param buf		buffer to marshal to
 *
 *	@todo 	Do the marshalling of a 64 byte integer in a more standard way,
 *			though this will break compatibility with the bsdiff command line
 *			utility.
 */
void bsdiff_offtout( bs_off_t x, u_char * buf )
{
	bs_off_t y;

	if (x < 0)
	{
		y = -x;
	}
	else
	{
		y = x;
	}

	// TODO: clean this up, though when testing this, GCC seemed to optimise it
	// quite well..
	buf[0]=y%256;y-=buf[0];
	y=y/256;buf[1]=y%256;y-=buf[1];
	y=y/256;buf[2]=y%256;y-=buf[2];
	y=y/256;buf[3]=y%256;y-=buf[3];
	y=y/256;buf[4]=y%256;y-=buf[4];
	y=y/256;buf[5]=y%256;y-=buf[5];
	y=y/256;buf[6]=y%256;y-=buf[6];
	y=y/256;buf[7]=y%256;

	if (x < 0)
	{
		buf[7] |= 0x80;
	}
}


/**
 *	Read in a 64-bit value from buf.
 *
 *	@param buf		the 8-byte marshalled 64-bit value
 *	@return			the value
 *
 *	@todo 	Do the marshalling of a 64 byte integer in a more standard way,
 *			though this will break compatibility with the bsdiff command line
 *			utility.
 */
bs_off_t bsdiff_offtin( const u_char * buf )
{
	bs_off_t y;

	y=buf[7]&0x7F;
	y=y*256;y+=buf[6];
	y=y*256;y+=buf[5];
	y=y*256;y+=buf[4];
	y=y*256;y+=buf[3];
	y=y*256;y+=buf[2];
	y=y*256;y+=buf[1];
	y=y*256;y+=buf[0];

	if(buf[7]&0x80) y=-y;

	return y;
}

} // end namespace (anonymous)


// ----------------------------------------------------------------------------
// Section: Implementation of declared functions in bslib.hpp
// ----------------------------------------------------------------------------


/**
 *	Return the error string associated with the error code.
 *
 *	@param errCode 		one of the enum values in BSLIB_ERROR
 *	@return 			a string description of the error
 */
const char * bsdiff_errString( int errCode )
{
	if (errCode < BSLIB_NUM_ERRORS)
	{
		return BSLIB_ERROR_STRINGS[errCode];
	}
	return "Unknown error";
}


/**
 *	Return the last error's description. Use this string immediately as it will
 *	be destructed the next time an error occurs.
 *
 *	@return string description of the last error
 */
const char * bsdiff_errDescription()
{
	return s_bsdiff_errDescription.c_str();
}


/**
 *	Perform a binary diff on a source string and a destination string and
 *	return the control block, diffs block and the extras block.
 *
 *	@param src				the source string
 *	@param srcLen			the length of the source string
 *	@param dst				the destination string
 *	@param dstLen			the length of the destination string
 *	@param pControlBlock	a return value pointer to where the control block
 *							will be allocated and filled.
 *	@param pControlLen		a return value pointer to the length of the control
 *							block
 *	@param pDiffsBlock		a return value pointer to where the diffs block
 *							will be allocated and filled.
 *	@param pDiffsLen		a return value pointer to the length of the diffs
 *							block
 *	@param pExtrasBlock		a return value pointer to where the extras block
 *							will be allocated and filled.
 *	@param pExtrasLen		a return value pointer to the length of the extras
 *							block
 *	@return					a BSLIB_ERROR code
 */
int bsdiff_diff( const u_char * src, bs_off_t srcLen,
		const u_char * dst, bs_off_t dstLen,
		u_char ** pControlBlock, bs_off_t * pControlLen,
		u_char ** pDiffsBlock, bs_off_t * pDiffLen,
		u_char ** pExtrasBlock, bs_off_t * pExtrasLen )
{
	bs_off_t *sufTab,*sufTabInv;
	bs_off_t scan,pos,len;
	bs_off_t lastscan,lastpos,lastoffset;
	bs_off_t oldscore,scsc;
	bs_off_t s,Sf,lenf,Sb,lenb;
	bs_off_t overlap,Ss,lens;
	bs_off_t i;
	u_char *db,*eb;
	u_char buf[8];

	if (!src || !dst || srcLen < 0 || dstLen < 0 ||
			!pControlBlock || !pDiffsBlock || !pExtrasBlock)
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR,
			"At least one of the parameters is NULL" );
	}

	ControlBlock controlBlock;

	// Allocate srcLen+1 bytes instead of srcLen bytes to ensure
	//	that we never try to malloc(0) and get a NULL pointer
	sufTab = new bs_off_t[srcLen + 1];
	if (sufTab == NULL)		BSDIFF_STD_MEM_ERROR;
	ArrayAutoPtr<bs_off_t> sufTabAuto( sufTab );

	sufTabInv = new bs_off_t[srcLen + 1];
	if (sufTabInv == NULL)	BSDIFF_STD_MEM_ERROR;

	{
		ArrayAutoPtr<bs_off_t> sufTabInvAuto( sufTabInv );
		bsdiff_qsufsort( sufTab, sufTabInv, src, srcLen );
	} // sufTabInv is now gone

	// Allocate dstLen+1 bytes instead of dstLen bytes to ensure
	//	that we never try to malloc(0) and get a NULL pointer
	*pDiffsBlock = new u_char[dstLen + 1];
	if (!*pDiffsBlock) 		BSDIFF_STD_MEM_ERROR;
	db = *pDiffsBlock;
	ArrayAutoPtr<u_char> diffsBlockAuto( db );

	*pExtrasBlock = new u_char[dstLen + 1];
	if (!*pExtrasBlock)		BSDIFF_STD_MEM_ERROR;
	eb = *pExtrasBlock;
	ArrayAutoPtr<u_char> extrasBlockAuto( eb );

	bs_off_t & dblen = *pDiffLen;
	bs_off_t & eblen = *pExtrasLen;
	dblen = 0;
	eblen = 0;

	// Compute the differences, writing ctrl as we go

	scan = 0;
	len = 0;
	lastscan = 0;
	lastpos = 0;
	lastoffset = 0;
	while (scan < dstLen)
	{
		oldscore = 0;

		for (scsc = scan += len; scan < dstLen; scan++)
		{
			len = bsdiff_search( sufTab, src, srcLen, dst + scan, dstLen - scan,
					0, srcLen, pos );

			for (;scsc < scan + len; ++scsc)
			{
				if(scsc + lastoffset < srcLen &&
						src[scsc + lastoffset] == dst[scsc])
				{
					++oldscore;
				}
			}

			if((len == oldscore && len != 0) ||
					len > oldscore + 8)
			{
				break;
			}

			if(scan + lastoffset < srcLen &&
					src[scan + lastoffset] == dst[scan])
			{
				--oldscore;
			}
		} // for (..; scan < dstLen; ..)

		if (len != oldscore || scan == dstLen)
		{
			s = 0;
			Sf = 0;
			lenf = 0;

			for (i = 0; lastscan + i < scan && lastpos + i < srcLen; )
			{
				if (src[lastpos + i] == dst[lastscan + i])
				{
					++s;
				}
				++i;
				if (s * 2 - i > Sf * 2 - lenf)
				{
					Sf = s;
					lenf = i;
				}
			}

			lenb = 0;
			if (scan < dstLen)
			{
				s = 0;
				Sb = 0;
				for (i = 1; scan >= lastscan + i && pos >= i; ++i)
				{
					if (src[pos - i] == dst[scan - i])
					{
						++s;
					}
					if (s * 2 - i > Sb * 2 - lenb)
					{
						Sb = s;
						lenb = i;
					}
				}
			}

			if (lastscan + lenf > scan - lenb)
			{
				overlap = (lastscan + lenf) - (scan - lenb);
				s = 0;
				Ss = 0;
				lens = 0;
				for (i = 0; i < overlap; ++i)
				{
					if (dst[lastscan + lenf - overlap + i] ==
						   src[lastpos + lenf - overlap + i])
					{
						++s;
					}
					if(dst[scan - lenb + i] ==
					  		src[pos - lenb + i])
					{
						--s;
					}
					if (s > Ss)
					{
						Ss = s;
						lens = i + 1;
					}
				}

				lenf += lens - overlap;
				lenb -= lens;
			} // if (lastscan + lenf > scan - lenb)

			for (i = 0; i < lenf; ++i)
			{
				db[dblen + i] = dst[lastscan + i] - src[lastpos + i];
			}
			for (i = 0; i < (scan - lenb) - (lastscan + lenf); ++i)
			{
				eb[eblen + i] = dst[lastscan + lenf + i];
			}
			dblen += lenf;
			eblen += (scan - lenb) - (lastscan + lenf);

			bsdiff_offtout( lenf, buf );
			controlBlock.insert( controlBlock.end(), buf, buf + 8 );

			bsdiff_offtout( (scan - lenb) - (lastscan + lenf), buf );
			controlBlock.insert( controlBlock.end(), buf, buf + 8 );

			bsdiff_offtout( (pos - lenb) - (lastpos + lenf), buf );
			controlBlock.insert( controlBlock.end(), buf, buf + 8 );

			lastscan = scan - lenb;
			lastpos = pos - lenb;
			lastoffset = pos - scan;
		}// if (len != oldscore || scan == dstLen)
	} // while (scan < dstLen)

	*pControlLen = controlBlock.size();
	*pControlBlock = new u_char[*pControlLen];
	for (i = 0; i < *pControlLen; ++i)
	{
		(*pControlBlock)[i] = controlBlock[i];
	}

	diffsBlockAuto.release();
	extrasBlockAuto.release();

	return BSLIB_OK;
}


/**
 *	This takes the control, diffs and extras blocks returned from bsdiff_diff
 *	and writes them out to a patch file path compatible with the bsdiff tool.
 *
 *	Header is:
 *		0	8	 "BSDIFF40"
 *		8	8	length of bzip2ed ctrl block
 *		16	8	length of bzip2ed diff block
 *		24	8	length of dst file
 *	 File is
 *		0	32	Header
 *		32	??	Bzip2ed ctrl block
 *		??	??	Bzip2ed diff block
 *		??	??	Bzip2ed extra block
 *
 *	@return				a BSLIB_ERROR code
 */
int bsdiff_blocksToPatchFile( bs_off_t dstLen,
		const u_char * control, bs_off_t controlLen,
		const u_char * diffs, bs_off_t diffsLen,
		const u_char * extras, bs_off_t extrasLen,
		const char * patchPath )
{
	if (dstLen < 0)
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR, "destination file size is negative" );
	}

	if (control == NULL || controlLen < 0)
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR, "invalid control block" );
	}

	if (diffs == NULL  || diffsLen < 0)
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR, "invalid diffs block" );
	}

	if (extras == NULL || extrasLen < 0)
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR, "invalid extras block" );
	}

	if (patchPath == NULL)
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR, "invalid patch path" );
	}

	FileAutoPtr patchFileAuto( patchPath, "wb" );
	FILE * patchFile = patchFileAuto.fp();
	if (!patchFile)
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not open patch file for writing '%s': %s",
			patchPath, strerror( errno ) );
	}

	// Compress control block.
	// Why the funny value? See the bzip2 docos.. they recommend allocating
	// 1% more than the uncompressed size plus 600 bytes as the guaranteed
	// non-overflow memory allocation size.
	static const unsigned BZ2_EXTRA_LENGTH = 600;
	unsigned controlCompressLen = int( ceil( 1.01 * controlLen ) ) +
		BZ2_EXTRA_LENGTH;

	char * controlBZ2 = new char[controlCompressLen];
	if (!controlBZ2)
	{
		BSDIFF_STD_MEM_ERROR;
	}

	ArrayAutoPtr<char> controlBZ2Auto( controlBZ2 );

	int res = BZ2_bzBuffToBuffCompress( controlBZ2, &controlCompressLen,
		(char*)control, controlLen, 9, 0, 0 );

	if (res != BZ_OK)
	{
		BSDIFF_ERROR( BSLIB_BZ2_ERROR,
			"bz2 compress failed on control block: res=%d",
			res );
	}

	// Compress diff block - see above notes for the control block as to why
	// diffsCompressLen is what it is.
	unsigned diffsCompressLen = int( ceil( 1.01 * diffsLen ) ) +
		BZ2_EXTRA_LENGTH;
	char * diffsBZ2 = new char[diffsCompressLen];
	if (!diffsBZ2)
	{
		BSDIFF_STD_MEM_ERROR;
	}

	ArrayAutoPtr<char> diffsBZ2Auto( diffsBZ2 );

	res = BZ2_bzBuffToBuffCompress( diffsBZ2, &diffsCompressLen,
		(char*)diffs, diffsLen, 9, 0, 0 );

	if (res != BZ_OK)
	{
		BSDIFF_ERROR( BSLIB_BZ2_ERROR,
			"bz2 compress failed on diffs block: res=%d",
			res );
	}

	// Compress extras block - see above notes for the control block as to why
	// extrasCompressLen is what it is.
	unsigned extrasCompressLen = int( ceil( 1.01 * extrasLen ) ) +
		BZ2_EXTRA_LENGTH;
	char * extrasBZ2 = new char[extrasCompressLen];
	if (!extrasBZ2)
	{
		BSDIFF_ERROR( BSLIB_MEM_ERROR,
			"Could not allocate %u bytes for compressing extras block",
			extrasCompressLen );
	}

	ArrayAutoPtr<char> extrasBZ2Auto( extrasBZ2 );

	res = BZ2_bzBuffToBuffCompress( extrasBZ2, &extrasCompressLen,
		(char*)extras, extrasLen, 9, 0, 0 );

	if (res != BZ_OK)
	{
		BSDIFF_ERROR( BSLIB_BZ2_ERROR,
			"bz2 compress failed on extras block: res=%d",
			res );
	}

	// write out the patch file
	if (0 == fwrite( "BSDIFF40", 8, 1, patchFile ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"Could not write header to patch file" );
	}

	u_char buf[8];
	// write X
	bsdiff_offtout( controlCompressLen, buf );
	if (0 == fwrite( buf, sizeof( buf ), 1, patchFile ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"Could not write control block length to patch file" );
	}

	// write Y
	bsdiff_offtout( diffsCompressLen, buf );
	if (0 == fwrite( buf, sizeof( buf ), 1, patchFile ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"Could not write diffs block length to patch file" );
	}

	// write new file size
	bsdiff_offtout( dstLen, buf );
	if (0 == fwrite( buf, sizeof( buf ), 1, patchFile ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not write out new file size to patch file" );
	}

	// write compressed control, diffs, and extras blocks out
	if (0 == fwrite( controlBZ2, controlCompressLen, 1, patchFile ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not write out control block to patch file" );
	}

	if (0 == fwrite( diffsBZ2, diffsCompressLen, 1, patchFile ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not write out diffs block to patch file" );
	}

	if (0 == fwrite( extrasBZ2, extrasCompressLen, 1, patchFile ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not write out extras block to patch file" );
	}

	return BSLIB_OK;
}


/**
 *	Compute the deltas for buf1 and buf2 and write the patch file to the given
 *	patchPath.
 *
 *	@param src			source byte string as a character buffer
 *	@param srcLen		source byte string length
 *	@param dst			destination byte string as a character buffer
 *	@param dstLen		destination byte string length
 *	@param patchPath	the patch path to write out to
 *	@return				a BSLIB_ERROR code
 *
 */
int bsdiff_diffToPatchFile( const char * src, bs_off_t srcLen,
		const char * dst, bs_off_t dstLen,
		const char * patchPath )
{
	u_char * controlBlock;
	bs_off_t controlBlockLen;
	u_char * diffsBlock;
	bs_off_t diffsBlockLen;
	u_char * extrasBlock;
	bs_off_t extrasBlockLen;


	int res = bsdiff_diff( (u_char*)src, srcLen, (u_char*)dst, dstLen,
		&controlBlock, &controlBlockLen,
		&diffsBlock, &diffsBlockLen,
		&extrasBlock, &extrasBlockLen );

	if (res != BSLIB_OK)
	{
		return res;
	}

	res = bsdiff_blocksToPatchFile( dstLen, controlBlock, controlBlockLen,
			diffsBlock, diffsBlockLen, extrasBlock, extrasBlockLen,
			patchPath );

	delete [] controlBlock;
	delete [] diffsBlock;
	delete [] extrasBlock;

	return res;
}


/**
 *	Performs a patch of a source file to a character buffer from a patch file.
 *
 *	The pointer to the character buffer is returned in pDstBytes, and is
 *	allocated using the C++ new operator, and should be destructed using the
 *	delete [] by the calling code.
 *
 *	If an error occurs, then no new buffers are allocated.
 *
 *	@param patchPath	the path to the patch file
 *	@param srcFile		an open file handle for reading from the source file
 *	@param pDstBytes	a pointer which is filled with a pointer to a newly
 *						allocated buffer containing the patched file contents
 *	@param pDstBytesLen	a pointer which is filled with the length of the
 *						patched file contents
 *	@return				a BSLIB_ERROR code
 *
 */
int bsdiff_patchFromPatchFile( const char * patchPath,
		FILE * srcFile,
		u_char ** pDstBytes,
		bs_off_t * pDstBytesLen )
{
	FILE * patchFile;
	FILE * controlFile;
	FILE * diffFile;
	FILE * extrasFile;
	BZFILE * controlFileBZ2;
	BZFILE * diffFileBZ2;
	BZFILE * extrasFileBZ2;

	int bz2Err;

	bs_off_t srcSize,dstSize;
	u_char buf[24];
	u_char *src, *dst;
	bs_off_t srcPos, dstPos;
	bs_off_t addLen, copyLen, skipLen;
	bs_off_t i;

	if (!patchPath)
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR, "patchPath is NULL" );
	}
	else if (!srcFile)
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR, "srcFile is NULL" );
	}
	else if (!pDstBytes)
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR, "pDstBytes is NULL" );
	}
	else if (!pDstBytesLen)
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR, "pDstBytesLen is NULL" );
	}
	else if (-1 == fileno( srcFile ))
	{
		BSDIFF_ERROR( BSLIB_PARAM_ERROR,
			"srcFile is not a valid file pointer" );
	}

	// Open patch file
	else if ((patchFile = fopen( patchPath, "rb" )) == NULL)
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not open patch file '%s'", patchPath );
	}

	// File format:
	// 	0	8	"BSDIFF40"
	// 	8	8	X
	// 	16	8	Y
	// 	24	8	sizeof(newfile)
	// 	32	X	bzip2(control block)
	// 	32+X	Y	bzip2(diff block)
	// 	32+X+Y	???	bzip2(extra block)
	// with control block a set of triples (x,y,z) meaning "add x bytes
	// from oldfile to x bytes from the diff block; copy y bytes from the
	// extra block; seek forwards in oldfile by z bytes".

	// Read header
	u_char header[32];
	if (fread( header, 1, 32, patchFile ) < 32)
	{
		if (feof( patchFile ))
		{
			BSDIFF_ERROR( BSLIB_BAD_PATCH_ERROR,
				"patch file contains no patch header '%s'", patchPath );
		}
		if (ferror( patchFile ))
		{
			BSDIFF_ERROR( BSLIB_IO_ERROR,
				"read error while reading from '%s'", patchPath );
		}
		BSDIFF_ERROR( BSLIB_UNKNOWN_ERROR,
			"while reading from '%s'", patchPath );
	}

	// Check for appropriate magic
	if (memcmp( header, "BSDIFF40", 8 ) != 0)
	{
		BSDIFF_ERROR( BSLIB_BAD_PATCH_ERROR,
			"bad magic: '%s'", patchPath );
	}

	// Read lengths from header
	bs_off_t bzControlLen = bsdiff_offtin( header + 8 );
	bs_off_t bzDiffLen = bsdiff_offtin( header + 16 );
	dstSize = bsdiff_offtin( header + 24 );

	if (bzControlLen < 0 || bzDiffLen < 0 || dstSize < 0)
	{
		BSDIFF_ERROR( BSLIB_BAD_PATCH_ERROR,
			"invalid patch header: '%s'", patchPath );
	}

	// Close patch file and re-open it via libbzip2 at the right places
	fclose( patchFile );

	FileAutoPtr controlFileAuto( patchPath, "rb" );
	if (!controlFileAuto.fp())
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not open control file in '%s': %s",
			patchPath, strerror( errno ) );
	}
	controlFile = controlFileAuto.fp();

	if (0 != fseek( controlFile, 32, SEEK_SET ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not seek to control block in patch '%s': %s",
			patchPath, strerror( errno ) );
	}
	controlFileBZ2 = BZ2_bzReadOpen( &bz2Err, controlFile,
		/*verbosity*/0, /*small*/0, /*unused*/NULL, /*nUnused*/0 );
	if (controlFileBZ2 == NULL)
	{
		BSDIFF_ERROR( BSLIB_BZ2_ERROR,
			"could not bz2-open control block in patch '%s': err=%d",
			patchPath, bz2Err );
	}

	FileAutoPtr diffFileAuto( patchPath, "rb" );
	if (!diffFileAuto.fp())
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not open diffs file in '%s': %s",
			patchPath, strerror( errno ) );
	}
	diffFile = diffFileAuto.fp();
	if (fseek( diffFile, 32 + bzControlLen, SEEK_SET ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not seek to diffs block in patch '%s': %s",
			patchPath, strerror( errno ) );
	}
	diffFileBZ2 = BZ2_bzReadOpen( &bz2Err, diffFile, 0, 0, NULL, 0 );
	if (diffFileBZ2 == NULL)
	{
		BSDIFF_ERROR( BSLIB_BZ2_ERROR,
			"could not bz2-open diffs block in patch '%s': res=%d",
			patchPath, bz2Err );
	}

	FileAutoPtr extrasFileAuto( patchPath, "rb" );
	if (!extrasFileAuto.fp())
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not open extras file in '%s': %s",
			patchPath, strerror( errno ) );
	}
	extrasFile = extrasFileAuto.fp();
	if (fseek( extrasFile, 32 + bzControlLen + bzDiffLen, SEEK_SET ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not seek to extras block in patch '%s': %s",
			patchPath, strerror( errno ) );
	}
	extrasFileBZ2 = BZ2_bzReadOpen( &bz2Err, extrasFile,
		0, 0, NULL, 0);
	if (extrasFileBZ2 == NULL)
	{
		BSDIFF_ERROR( BSLIB_BZ2_ERROR,
			"could not bz2-open extras block in patch '%s': res=%d",
			patchPath, bz2Err );
	}

	struct stat srcStat;
	if (0 != fstat( fileno( srcFile ), &srcStat ))
	{
		BSDIFF_ERROR( BSLIB_IO_ERROR,
			"could not stat source file: %s",
			strerror( errno ) );
	}
	srcSize = srcStat.st_size;
	src = new u_char[srcSize + 1];
	if (!src)	BSDIFF_STD_MEM_ERROR;
	ArrayAutoPtr<u_char> srcAuto( src );
	dst = new u_char[dstSize + 1];
	if (!dst)	BSDIFF_STD_MEM_ERROR;
	ArrayAutoPtr<u_char> dstAuto( dst );


	// read 1 x object of size srcStat.st_size, should return 0 or 1
	int res = fread( src, srcStat.st_size, 1, srcFile );
	if (res == 0)
	{
		if (ferror( srcFile ))
		{
			BSDIFF_ERROR( BSLIB_IO_ERROR,
				"read error reading source file" );
		}
		if (feof( srcFile ))
		{
			BSDIFF_ERROR( BSLIB_IO_ERROR,
				"unexpected EOF reading source file" );
		}
		BSDIFF_ERROR( BSLIB_UNKNOWN_ERROR,
			"reading source file" );
	}

	srcPos = 0;
	dstPos = 0;
	while (dstPos < dstSize)
	{

		bs_off_t readLen = BZ2_bzRead( &bz2Err, controlFileBZ2, buf, 24 );

		if (readLen < 24)
		{
			BSDIFF_ERROR( BSLIB_BAD_PATCH_ERROR,
				"prematurely consumed all control triples" );
		}
		else if (bz2Err != BZ_OK && bz2Err != BZ_STREAM_END)
		{
			BSDIFF_ERROR( BSLIB_BZ2_ERROR,
				"bz2 error while reading control file (res=%d)",
				bz2Err );
		}

		addLen = bsdiff_offtin( buf );
		copyLen = bsdiff_offtin( buf + 8 );
		skipLen = bsdiff_offtin( buf + 16 );

		// Sanity-check

		if (dstPos + addLen > dstSize)
		{
			BSDIFF_ERROR( BSLIB_BAD_PATCH_ERROR,
				"control add is out of bounds: %lld",
				dstPos + addLen );
		}

		// Read diff string
		readLen = BZ2_bzRead( &bz2Err, diffFileBZ2, dst + dstPos, addLen );
		if (readLen < addLen ||
		 	   (bz2Err != BZ_OK) && (bz2Err != BZ_STREAM_END))
		{
			BSDIFF_ERROR( BSLIB_BAD_PATCH_ERROR,
				"prematurely consumed all diff data (read %lld bytes)",
				readLen );
		}

		// Add old data to diff string
		for (i = 0; i < addLen; ++i)
		{
			if(srcPos + i >= 0 && srcPos + i < srcSize)
			{
				dst[dstPos + i] += src[srcPos + i];
			}
		}

		// Adjust pointers
		dstPos += addLen;
		srcPos += addLen;

		// Sanity-check
		if (dstPos + copyLen > dstSize)
		{
			BSDIFF_ERROR( BSLIB_BAD_PATCH_ERROR,
				"control copy out of bounds: %lld",
				dstPos + copyLen );
		}

		// Read extra string
		readLen = BZ2_bzRead( &bz2Err, extrasFileBZ2,
			dst + dstPos, copyLen );
		if (readLen < copyLen ||
		 	   (bz2Err != BZ_OK) && (bz2Err != BZ_STREAM_END))
		{
			BSDIFF_ERROR( BSLIB_BAD_PATCH_ERROR,
				"could not read extras block" );
		}

		// Adjust pointers
		dstPos += copyLen;
		srcPos += skipLen;
	} // while (dstPos < dstSize)

	// Clean up the bzip2 reads
	BZ2_bzReadClose( &bz2Err, controlFileBZ2 );
	BZ2_bzReadClose( &bz2Err, diffFileBZ2 );
	BZ2_bzReadClose( &bz2Err, extrasFileBZ2 );

	// set output params
	*pDstBytesLen = dstSize;
	*pDstBytes = dst;
	dstAuto.release(); // pass responsibility for it to the caller

	return BSLIB_OK;
}


// bslib.cpp
