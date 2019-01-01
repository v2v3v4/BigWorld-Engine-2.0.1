/*
  Default header file for malloc-2.8.x, written by Doug Lea
  and released to the public domain, as explained at
  http://creativecommons.org/licenses/publicdomain. 

  last update: Wed May 27 14:25:17 2009  Doug Lea  (dl at gee)

  This header is for ANSI C/C++ only.  You can set any of
  the following #defines before including:

  * If USE_DL_PREFIX is defined, it is assumed that malloc.c 
    was also compiled with this option, so all routines
    have names starting with "dl".

  * If HAVE_USR_INCLUDE_MALLOC_H is defined, it is assumed that this
    file will be #included AFTER <malloc.h>. This is needed only if
    your system defines a struct mallinfo that is incompatible with the
    standard one declared here.  Otherwise, you can include this file
    INSTEAD of your system system <malloc.h>.  At least on ANSI, all
    declarations should be compatible with system versions

  * If MSPACES is defined, declarations for mspace versions are included.
*/

/**
 *	Adapted for BigWorld
 */


#ifndef MALLOC_280_H
#define MALLOC_280_H

// BigWorld conditional compile
#ifdef EDITOR_ENABLED

///////////////////////////////////////////////////////////////////////////////
//  BigWorld-specific Code
///////////////////////////////////////////////////////////////////////////////

// BigWorld preferences
#define USE_DL_PREFIX 1
#define USE_LOCKS 1

#include <crtdefs.h>

// BigWorld declaration of strdup
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	int dlenabled( int enabled );

	char * dlstrdup( const char * str );
	wchar_t * dlwcsdup( const wchar_t * str );
#ifdef __cplusplus
};  /* end of extern "C" */
#endif /* __cplusplus */

///////////////////////////////////////////////////////////////////////////////
//  End BigWorld-specific Code
///////////////////////////////////////////////////////////////////////////////


/* Version identifier to allow people to support multiple versions */
#ifndef DLMALLOC_VERSION
#define DLMALLOC_VERSION 20804
#endif /* DLMALLOC_VERSION */

#ifndef WIN32
#ifdef _WIN32
#define WIN32 1
#endif  /* _WIN32 */
#ifdef _WIN32_WCE
#define LACKS_FCNTL_H
#define WIN32 1
#endif /* _WIN32_WCE */
#endif  /* WIN32 */
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define HAVE_MMAP 1
#define HAVE_MORECORE 0
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
#define LACKS_STRING_H
#define LACKS_STRINGS_H
#define LACKS_SYS_TYPES_H
#define LACKS_ERRNO_H
#ifndef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION
#endif /* MALLOC_FAILURE_ACTION */
#ifdef _WIN32_WCE /* WINCE reportedly does not clear */
#define MMAP_CLEARS 0
#else
#define MMAP_CLEARS 1
#endif /* _WIN32_WCE */
#endif  /* WIN32 */

#if defined(DARWIN) || defined(_DARWIN)
/* Mac OSX docs advise not to use sbrk; it seems better to use mmap */
#ifndef HAVE_MORECORE
#define HAVE_MORECORE 0
#define HAVE_MMAP 1
/* OSX allocators provide 16 byte alignment */
#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT ((size_t)16U)
#endif
#endif  /* HAVE_MORECORE */
#endif  /* DARWIN */

#ifndef LACKS_SYS_TYPES_H
#include <sys/types.h>  /* For size_t */
#endif  /* LACKS_SYS_TYPES_H */

#if (defined(__GNUC__) && ((defined(__i386__) || defined(__x86_64__)))) || (defined(_MSC_VER) && _MSC_VER>=1310)
#define SPIN_LOCKS_AVAILABLE 1
#else
#define SPIN_LOCKS_AVAILABLE 0
#endif

/* The maximum possible size_t value has all bits set */
#define MAX_SIZE_T           (~(size_t)0)

#ifndef ONLY_MSPACES
#define ONLY_MSPACES 0     /* define to a value */
#else
#define ONLY_MSPACES 1
#endif  /* ONLY_MSPACES */
#ifndef MSPACES
#if ONLY_MSPACES
#define MSPACES 1
#else   /* ONLY_MSPACES */
#define MSPACES 0
#endif  /* ONLY_MSPACES */
#endif  /* MSPACES */
#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT ((size_t)8U)
#endif  /* MALLOC_ALIGNMENT */
#ifndef FOOTERS
#define FOOTERS 0
#endif  /* FOOTERS */
#ifndef ABORT
#define ABORT  abort()
#endif  /* ABORT */
#ifndef ABORT_ON_ASSERT_FAILURE
#define ABORT_ON_ASSERT_FAILURE 1
#endif  /* ABORT_ON_ASSERT_FAILURE */
#ifndef PROCEED_ON_ERROR
#define PROCEED_ON_ERROR 0
#endif  /* PROCEED_ON_ERROR */
#ifndef USE_LOCKS
#define USE_LOCKS 0
#endif  /* USE_LOCKS */
#ifndef USE_SPIN_LOCKS
#if USE_LOCKS && SPIN_LOCKS_AVAILABLE
#define USE_SPIN_LOCKS 1
#else
#define USE_SPIN_LOCKS 0
#endif /* USE_LOCKS && SPIN_LOCKS_AVAILABLE. */
#endif /* USE_SPIN_LOCKS */
#ifndef INSECURE
#define INSECURE 0
#endif  /* INSECURE */
#ifndef HAVE_MMAP
#define HAVE_MMAP 1
#endif  /* HAVE_MMAP */
#ifndef MMAP_CLEARS
#define MMAP_CLEARS 1
#endif  /* MMAP_CLEARS */
#ifndef HAVE_MREMAP
#ifdef linux
#define HAVE_MREMAP 1
#else   /* linux */
#define HAVE_MREMAP 0
#endif  /* linux */
#endif  /* HAVE_MREMAP */
#ifndef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION  errno = ENOMEM;
#endif  /* MALLOC_FAILURE_ACTION */
#ifndef HAVE_MORECORE
#if ONLY_MSPACES
#define HAVE_MORECORE 0
#else   /* ONLY_MSPACES */
#define HAVE_MORECORE 1
#endif  /* ONLY_MSPACES */
#endif  /* HAVE_MORECORE */
#if !HAVE_MORECORE
#define MORECORE_CONTIGUOUS 0
#else   /* !HAVE_MORECORE */
#define MORECORE_DEFAULT sbrk
#ifndef MORECORE_CONTIGUOUS
#define MORECORE_CONTIGUOUS 1
#endif  /* MORECORE_CONTIGUOUS */
#endif  /* HAVE_MORECORE */
#ifndef DEFAULT_GRANULARITY
#if (MORECORE_CONTIGUOUS || defined(WIN32))
#define DEFAULT_GRANULARITY (0)  /* 0 means to compute in init_mparams */
#else   /* MORECORE_CONTIGUOUS */
#define DEFAULT_GRANULARITY ((size_t)64U * (size_t)1024U)
#endif  /* MORECORE_CONTIGUOUS */
#endif  /* DEFAULT_GRANULARITY */
#ifndef DEFAULT_TRIM_THRESHOLD
#ifndef MORECORE_CANNOT_TRIM
#define DEFAULT_TRIM_THRESHOLD ((size_t)2U * (size_t)1024U * (size_t)1024U)
#else   /* MORECORE_CANNOT_TRIM */
#define DEFAULT_TRIM_THRESHOLD MAX_SIZE_T
#endif  /* MORECORE_CANNOT_TRIM */
#endif  /* DEFAULT_TRIM_THRESHOLD */
#ifndef DEFAULT_MMAP_THRESHOLD
#if HAVE_MMAP
#define DEFAULT_MMAP_THRESHOLD ((size_t)256U * (size_t)1024U)
#else   /* HAVE_MMAP */
#define DEFAULT_MMAP_THRESHOLD MAX_SIZE_T
#endif  /* HAVE_MMAP */
#endif  /* DEFAULT_MMAP_THRESHOLD */
#ifndef MAX_RELEASE_CHECK_RATE
#if HAVE_MMAP
#define MAX_RELEASE_CHECK_RATE 4095
#else
#define MAX_RELEASE_CHECK_RATE MAX_SIZE_T
#endif /* HAVE_MMAP */
#endif /* MAX_RELEASE_CHECK_RATE */
#ifndef USE_BUILTIN_FFS
#define USE_BUILTIN_FFS 0
#endif  /* USE_BUILTIN_FFS */
#ifndef USE_DEV_RANDOM
#define USE_DEV_RANDOM 0
#endif  /* USE_DEV_RANDOM */
#ifndef NO_MALLINFO
#define NO_MALLINFO 0
#endif  /* NO_MALLINFO */
#ifndef MALLINFO_FIELD_TYPE
#define MALLINFO_FIELD_TYPE size_t
#endif  /* MALLINFO_FIELD_TYPE */
#ifndef NO_SEGMENT_TRAVERSAL
#define NO_SEGMENT_TRAVERSAL 0
#endif /* NO_SEGMENT_TRAVERSAL */

/*
  mallopt tuning options.  SVID/XPG defines four standard parameter
  numbers for mallopt, normally defined in malloc.h.  None of these
  are used in this malloc, so setting them has no effect. But this
  malloc does support the following options.
*/

#define M_TRIM_THRESHOLD     (-1)
#define M_GRANULARITY        (-2)
#define M_MMAP_THRESHOLD     (-3)

/* ------------------------ Mallinfo declarations ------------------------ */

#if !NO_MALLINFO
/*
  This version of malloc supports the standard SVID/XPG mallinfo
  routine that returns a struct containing usage properties and
  statistics. It should work on any system that has a
  /usr/include/malloc.h defining struct mallinfo.  The main
  declaration needed is the mallinfo struct that is returned (by-copy)
  by mallinfo().  The malloinfo struct contains a bunch of fields that
  are not even meaningful in this version of malloc.  These fields are
  are instead filled by mallinfo() with other numbers that might be of
  interest.

  HAVE_USR_INCLUDE_MALLOC_H should be set if you have a
  /usr/include/malloc.h file that includes a declaration of struct
  mallinfo.  If so, it is included; else a compliant version is
  declared below.  These must be precisely the same for mallinfo() to
  work.  The original SVID version of this struct, defined on most
  systems with mallinfo, declares all fields as ints. But some others
  define as unsigned long. If your system defines the fields using a
  type of different width than listed here, you MUST #include your
  system version and #define HAVE_USR_INCLUDE_MALLOC_H.
*/

/* #define HAVE_USR_INCLUDE_MALLOC_H */

#ifdef HAVE_USR_INCLUDE_MALLOC_H
#include "/usr/include/malloc.h"
#else /* HAVE_USR_INCLUDE_MALLOC_H */
#ifndef STRUCT_MALLINFO_DECLARED
#define STRUCT_MALLINFO_DECLARED 1
struct mallinfo {
  MALLINFO_FIELD_TYPE arena;    /* non-mmapped space allocated from system */
  MALLINFO_FIELD_TYPE ordblks;  /* number of free chunks */
  MALLINFO_FIELD_TYPE smblks;   /* always 0 */
  MALLINFO_FIELD_TYPE hblks;    /* always 0 */
  MALLINFO_FIELD_TYPE hblkhd;   /* space in mmapped regions */
  MALLINFO_FIELD_TYPE usmblks;  /* maximum total allocated space */
  MALLINFO_FIELD_TYPE fsmblks;  /* always 0 */
  MALLINFO_FIELD_TYPE uordblks; /* total allocated space */
  MALLINFO_FIELD_TYPE fordblks; /* total free space */
  MALLINFO_FIELD_TYPE keepcost; /* releasable (via malloc_trim) space */
};
#endif /* STRUCT_MALLINFO_DECLARED */
#endif /* HAVE_USR_INCLUDE_MALLOC_H */
#endif /* NO_MALLINFO */

/*
  Try to persuade compilers to inline. The most critical functions for
  inlining are defined as macros, so these aren't used for them.
*/

#ifndef FORCEINLINE
  #if defined(__GNUC__)
#define FORCEINLINE __inline __attribute__ ((always_inline))
  #elif defined(_MSC_VER)
    #define FORCEINLINE __forceinline
  #endif
#endif
#ifndef NOINLINE
  #if defined(__GNUC__)
    #define NOINLINE __attribute__ ((noinline))
  #elif defined(_MSC_VER)
    #define NOINLINE __declspec(noinline)
  #else
    #define NOINLINE
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#ifndef FORCEINLINE
 #define FORCEINLINE inline
#endif
#endif /* __cplusplus */
#ifndef FORCEINLINE
 #define FORCEINLINE
#endif

#if !ONLY_MSPACES

/* ------------------- Declarations of public routines ------------------- */

#ifndef USE_DL_PREFIX
#define dlcalloc               calloc
#define dlfree                 free
#define dlmalloc               malloc
#define dlmemalign             memalign
#define dlrealloc              realloc
#define dlvalloc               valloc
#define dlpvalloc              pvalloc
#define dlmallinfo             mallinfo
#define dlmallopt              mallopt
#define dlmalloc_trim          malloc_trim
#define dlmalloc_stats         malloc_stats
#define dlmalloc_usable_size   malloc_usable_size
#define dlmalloc_footprint     malloc_footprint
#define dlmalloc_max_footprint malloc_max_footprint
#define dlindependent_calloc   independent_calloc
#define dlindependent_comalloc independent_comalloc
#endif /* USE_DL_PREFIX */


/*
  malloc(size_t n)
  Returns a pointer to a newly allocated chunk of at least n bytes, or
  null if no space is available, in which case errno is set to ENOMEM
  on ANSI C systems.

  If n is zero, malloc returns a minimum-sized chunk. (The minimum
  size is 16 bytes on most 32bit systems, and 32 bytes on 64bit
  systems.)  Note that size_t is an unsigned type, so calls with
  arguments that would be negative if signed are interpreted as
  requests for huge amounts of space, which will often fail. The
  maximum supported value of n differs across systems, but is in all
  cases less than the maximum representable value of a size_t.
*/
void* dlmalloc(size_t);

/*
  free(void* p)
  Releases the chunk of memory pointed to by p, that had been previously
  allocated using malloc or a related routine such as realloc.
  It has no effect if p is null. If p was not malloced or already
  freed, free(p) will by default cause the current program to abort.
*/
void  dlfree(void*);

/*
  calloc(size_t n_elements, size_t element_size);
  Returns a pointer to n_elements * element_size bytes, with all locations
  set to zero.
*/
void* dlcalloc(size_t, size_t);

/*
  realloc(void* p, size_t n)
  Returns a pointer to a chunk of size n that contains the same data
  as does chunk p up to the minimum of (n, p's size) bytes, or null
  if no space is available.

  The returned pointer may or may not be the same as p. The algorithm
  prefers extending p in most cases when possible, otherwise it
  employs the equivalent of a malloc-copy-free sequence.

  If p is null, realloc is equivalent to malloc.

  If space is not available, realloc returns null, errno is set (if on
  ANSI) and p is NOT freed.

  if n is for fewer bytes than already held by p, the newly unused
  space is lopped off and freed if possible.  realloc with a size
  argument of zero (re)allocates a minimum-sized chunk.

  The old unix realloc convention of allowing the last-free'd chunk
  to be used as an argument to realloc is not supported.
*/

void* dlrealloc(void*, size_t);

/*
  memalign(size_t alignment, size_t n);
  Returns a pointer to a newly allocated chunk of n bytes, aligned
  in accord with the alignment argument.

  The alignment argument should be a power of two. If the argument is
  not a power of two, the nearest greater power is used.
  8-byte alignment is guaranteed by normal malloc calls, so don't
  bother calling memalign with an argument of 8 or less.

  Overreliance on memalign is a sure way to fragment space.
*/
void* dlmemalign(size_t, size_t);

/*
  valloc(size_t n);
  Equivalent to memalign(pagesize, n), where pagesize is the page
  size of the system. If the pagesize is unknown, 4096 is used.
*/
void* dlvalloc(size_t);

/*
  mallopt(int parameter_number, int parameter_value)
  Sets tunable parameters The format is to provide a
  (parameter-number, parameter-value) pair.  mallopt then sets the
  corresponding parameter to the argument value if it can (i.e., so
  long as the value is meaningful), and returns 1 if successful else
  0.  To workaround the fact that mallopt is specified to use int,
  not size_t parameters, the value -1 is specially treated as the
  maximum unsigned size_t value.

  SVID/XPG/ANSI defines four standard param numbers for mallopt,
  normally defined in malloc.h.  None of these are use in this malloc,
  so setting them has no effect. But this malloc also supports other
  options in mallopt. See below for details.  Briefly, supported
  parameters are as follows (listed defaults are for "typical"
  configurations).

  Symbol            param #  default    allowed param values
  M_TRIM_THRESHOLD     -1   2*1024*1024   any   (-1 disables)
  M_GRANULARITY        -2     page size   any power of 2 >= page size
  M_MMAP_THRESHOLD     -3      256*1024   any   (or 0 if no MMAP support)
*/
int dlmallopt(int, int);

/*
  malloc_footprint();
  Returns the number of bytes obtained from the system.  The total
  number of bytes allocated by malloc, realloc etc., is less than this
  value. Unlike mallinfo, this function returns only a precomputed
  result, so can be called frequently to monitor memory consumption.
  Even if locks are otherwise defined, this function does not use them,
  so results might not be up to date.
*/
size_t dlmalloc_footprint(void);

/*
  malloc_max_footprint();
  Returns the maximum number of bytes obtained from the system. This
  value will be greater than current footprint if deallocated space
  has been reclaimed by the system. The peak number of bytes allocated
  by malloc, realloc etc., is less than this value. Unlike mallinfo,
  this function returns only a precomputed result, so can be called
  frequently to monitor memory consumption.  Even if locks are
  otherwise defined, this function does not use them, so results might
  not be up to date.
*/
size_t dlmalloc_max_footprint(void);

#if !NO_MALLINFO
/*
  mallinfo()
  Returns (by copy) a struct containing various summary statistics:

  arena:     current total non-mmapped bytes allocated from system
  ordblks:   the number of free chunks
  smblks:    always zero.
  hblks:     current number of mmapped regions
  hblkhd:    total bytes held in mmapped regions
  usmblks:   the maximum total allocated space. This will be greater
                than current total if trimming has occurred.
  fsmblks:   always zero
  uordblks:  current total allocated space (normal or mmapped)
  fordblks:  total free space
  keepcost:  the maximum number of bytes that could ideally be released
               back to system via malloc_trim. ("ideally" means that
               it ignores page restrictions etc.)

  Because these fields are ints, but internal bookkeeping may
  be kept as longs, the reported values may wrap around zero and
  thus be inaccurate.
*/
struct mallinfo dlmallinfo(void);
#endif /* NO_MALLINFO */

/*
  independent_calloc(size_t n_elements, size_t element_size, void* chunks[]);

  independent_calloc is similar to calloc, but instead of returning a
  single cleared space, it returns an array of pointers to n_elements
  independent elements that can hold contents of size elem_size, each
  of which starts out cleared, and can be independently freed,
  realloc'ed etc. The elements are guaranteed to be adjacently
  allocated (this is not guaranteed to occur with multiple callocs or
  mallocs), which may also improve cache locality in some
  applications.

  The "chunks" argument is optional (i.e., may be null, which is
  probably the most typical usage). If it is null, the returned array
  is itself dynamically allocated and should also be freed when it is
  no longer needed. Otherwise, the chunks array must be of at least
  n_elements in length. It is filled in with the pointers to the
  chunks.

  In either case, independent_calloc returns this pointer array, or
  null if the allocation failed.  If n_elements is zero and "chunks"
  is null, it returns a chunk representing an array with zero elements
  (which should be freed if not wanted).

  Each element must be individually freed when it is no longer
  needed. If you'd like to instead be able to free all at once, you
  should instead use regular calloc and assign pointers into this
  space to represent elements.  (In this case though, you cannot
  independently free elements.)

  independent_calloc simplifies and speeds up implementations of many
  kinds of pools.  It may also be useful when constructing large data
  structures that initially have a fixed number of fixed-sized nodes,
  but the number is not known at compile time, and some of the nodes
  may later need to be freed. For example:

  struct Node { int item; struct Node* next; };

  struct Node* build_list() {
    struct Node** pool;
    int n = read_number_of_nodes_needed();
    if (n <= 0) return 0;
    pool = (struct Node**)(independent_calloc(n, sizeof(struct Node), 0);
    if (pool == 0) die();
    // organize into a linked list...
    struct Node* first = pool[0];
    for (i = 0; i < n-1; ++i)
      pool[i]->next = pool[i+1];
    free(pool);     // Can now free the array (or not, if it is needed later)
    return first;
  }
*/
void** dlindependent_calloc(size_t, size_t, void**);

/*
  independent_comalloc(size_t n_elements, size_t sizes[], void* chunks[]);

  independent_comalloc allocates, all at once, a set of n_elements
  chunks with sizes indicated in the "sizes" array.    It returns
  an array of pointers to these elements, each of which can be
  independently freed, realloc'ed etc. The elements are guaranteed to
  be adjacently allocated (this is not guaranteed to occur with
  multiple callocs or mallocs), which may also improve cache locality
  in some applications.

  The "chunks" argument is optional (i.e., may be null). If it is null
  the returned array is itself dynamically allocated and should also
  be freed when it is no longer needed. Otherwise, the chunks array
  must be of at least n_elements in length. It is filled in with the
  pointers to the chunks.

  In either case, independent_comalloc returns this pointer array, or
  null if the allocation failed.  If n_elements is zero and chunks is
  null, it returns a chunk representing an array with zero elements
  (which should be freed if not wanted).

  Each element must be individually freed when it is no longer
  needed. If you'd like to instead be able to free all at once, you
  should instead use a single regular malloc, and assign pointers at
  particular offsets in the aggregate space. (In this case though, you
  cannot independently free elements.)

  independent_comallac differs from independent_calloc in that each
  element may have a different size, and also that it does not
  automatically clear elements.

  independent_comalloc can be used to speed up allocation in cases
  where several structs or objects must always be allocated at the
  same time.  For example:

  struct Head { ... }
  struct Foot { ... }

  void send_message(char* msg) {
    int msglen = strlen(msg);
    size_t sizes[3] = { sizeof(struct Head), msglen, sizeof(struct Foot) };
    void* chunks[3];
    if (independent_comalloc(3, sizes, chunks) == 0)
      die();
    struct Head* head = (struct Head*)(chunks[0]);
    char*        body = (char*)(chunks[1]);
    struct Foot* foot = (struct Foot*)(chunks[2]);
    // ...
  }

  In general though, independent_comalloc is worth using only for
  larger values of n_elements. For small values, you probably won't
  detect enough difference from series of malloc calls to bother.

  Overuse of independent_comalloc can increase overall memory usage,
  since it cannot reuse existing noncontiguous small chunks that
  might be available for some of the elements.
*/
void** dlindependent_comalloc(size_t, size_t*, void**);


/*
  pvalloc(size_t n);
  Equivalent to valloc(minimum-page-that-holds(n)), that is,
  round up n to nearest pagesize.
 */
void*  dlpvalloc(size_t);

/*
  malloc_trim(size_t pad);

  If possible, gives memory back to the system (via negative arguments
  to sbrk) if there is unused memory at the `high' end of the malloc
  pool or in unused MMAP segments. You can call this after freeing
  large blocks of memory to potentially reduce the system-level memory
  requirements of a program. However, it cannot guarantee to reduce
  memory. Under some allocation patterns, some large free blocks of
  memory will be locked between two used chunks, so they cannot be
  given back to the system.

  The `pad' argument to malloc_trim represents the amount of free
  trailing space to leave untrimmed. If this argument is zero, only
  the minimum amount of memory to maintain internal data structures
  will be left. Non-zero arguments can be supplied to maintain enough
  trailing space to service future expected allocations without having
  to re-obtain memory from the system.

  Malloc_trim returns 1 if it actually released any memory, else 0.
*/
int  dlmalloc_trim(size_t);

/*
  malloc_stats();
  Prints on stderr the amount of space obtained from the system (both
  via sbrk and mmap), the maximum amount (which may be more than
  current if malloc_trim and/or munmap got called), and the current
  number of bytes allocated via malloc (or realloc, etc) but not yet
  freed. Note that this is the number of bytes allocated, not the
  number requested. It will be larger than the number requested
  because of alignment and bookkeeping overhead. Because it includes
  alignment wastage as being in use, this figure may be greater than
  zero even when no user-level chunks are allocated.

  The reported current and maximum system memory can be inaccurate if
  a program makes other calls to system memory allocation functions
  (normally sbrk) outside of malloc.

  malloc_stats prints only the most commonly interesting statistics.
  More information can be obtained by calling mallinfo.
*/
void  dlmalloc_stats(void);

#endif /* ONLY_MSPACES */

/*
  malloc_usable_size(void* p);

  Returns the number of bytes you can actually use in
  an allocated chunk, which may be more than you requested (although
  often not) due to alignment and minimum size constraints.
  You can use this many bytes without worrying about
  overwriting other allocated objects. This is not a particularly great
  programming practice. malloc_usable_size can be more useful in
  debugging and assertions, for example:

  p = malloc(n);
  assert(malloc_usable_size(p) >= 256);
*/
size_t dlmalloc_usable_size(void*);


#if MSPACES

/*
  mspace is an opaque type representing an independent
  region of space that supports mspace_malloc, etc.
*/
typedef void* mspace;

/*
  create_mspace creates and returns a new independent space with the
  given initial capacity, or, if 0, the default granularity size.  It
  returns null if there is no system memory available to create the
  space.  If argument locked is non-zero, the space uses a separate
  lock to control access. The capacity of the space will grow
  dynamically as needed to service mspace_malloc requests.  You can
  control the sizes of incremental increases of this space by
  compiling with a different DEFAULT_GRANULARITY or dynamically
  setting with mallopt(M_GRANULARITY, value).
*/
mspace create_mspace(size_t capacity, int locked);

/*
  destroy_mspace destroys the given space, and attempts to return all
  of its memory back to the system, returning the total number of
  bytes freed. After destruction, the results of access to all memory
  used by the space become undefined.
*/
size_t destroy_mspace(mspace msp);

/*
  create_mspace_with_base uses the memory supplied as the initial base
  of a new mspace. Part (less than 128*sizeof(size_t) bytes) of this
  space is used for bookkeeping, so the capacity must be at least this
  large. (Otherwise 0 is returned.) When this initial space is
  exhausted, additional memory will be obtained from the system.
  Destroying this space will deallocate all additionally allocated
  space (if possible) but not the initial base.
*/
mspace create_mspace_with_base(void* base, size_t capacity, int locked);

/*
  mspace_track_large_chunks controls whether requests for large chunks
  are allocated in their own untracked mmapped regions, separate from
  others in this mspace. By default large chunks are not tracked,
  which reduces fragmentation. However, such chunks are not
  necessarily released to the system upon destroy_mspace.  Enabling
  tracking by setting to true may increase fragmentation, but avoids
  leakage when relying on destroy_mspace to release all memory
  allocated using this space.  The function returns the previous
  setting.
*/
int mspace_track_large_chunks(mspace msp, int enable);


/*
  mspace_malloc behaves as malloc, but operates within
  the given space.
*/
void* mspace_malloc(mspace msp, size_t bytes);

/*
  mspace_free behaves as free, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_free is not actually needed.
  free may be called instead of mspace_free because freed chunks from
  any space are handled by their originating spaces.
*/
void mspace_free(mspace msp, void* mem);

/*
  mspace_realloc behaves as realloc, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_realloc is not actually
  needed.  realloc may be called instead of mspace_realloc because
  realloced chunks from any space are handled by their originating
  spaces.
*/
void* mspace_realloc(mspace msp, void* mem, size_t newsize);

/*
  mspace_calloc behaves as calloc, but operates within
  the given space.
*/
void* mspace_calloc(mspace msp, size_t n_elements, size_t elem_size);

/*
  mspace_memalign behaves as memalign, but operates within
  the given space.
*/
void* mspace_memalign(mspace msp, size_t alignment, size_t bytes);

/*
  mspace_independent_calloc behaves as independent_calloc, but
  operates within the given space.
*/
void** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_size, void* chunks[]);

/*
  mspace_independent_comalloc behaves as independent_comalloc, but
  operates within the given space.
*/
void** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], void* chunks[]);

/*
  mspace_footprint() returns the number of bytes obtained from the
  system for this space.
*/
size_t mspace_footprint(mspace msp);

/*
  mspace_max_footprint() returns the peak number of bytes obtained from the
  system for this space.
*/
size_t mspace_max_footprint(mspace msp);


#if !NO_MALLINFO
/*
  mspace_mallinfo behaves as mallinfo, but reports properties of
  the given space.
*/
struct mallinfo mspace_mallinfo(mspace msp);
#endif /* NO_MALLINFO */

/*
  malloc_usable_size(void* p) behaves the same as malloc_usable_size;
*/
  size_t mspace_usable_size(void* mem);

/*
  mspace_malloc_stats behaves as malloc_stats, but reports
  properties of the given space.
*/
void mspace_malloc_stats(mspace msp);

/*
  mspace_trim behaves as malloc_trim, but
  operates within the given space.
*/
int mspace_trim(mspace msp, size_t pad);

/*
  An alias for mallopt.
*/
int mspace_mallopt(int, int);

#endif /* MSPACES */

#ifdef __cplusplus
};  /* end of extern "C" */
#endif /* __cplusplus */


// End BigWorld conditional compile
#endif // EDITOR_ENABLED


#endif /* MALLOC_280_H */
