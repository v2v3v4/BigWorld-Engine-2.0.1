/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RESOURCEMEMORYTRACE_HPP
#define RESOURCEMEMORYTRACE_HPP

//#define USE_MEMORY_TRACER

#ifndef USE_MEMORY_TRACER
#define MEM_TRACE_BEGIN( str )
#define MEM_TRACE_END( )
#else
#define MEM_TRACE_BEGIN( str )	ResourceMemoryTrace::instance().begin( str );
#define MEM_TRACE_END( )		ResourceMemoryTrace::instance().end();

#include "cstdmf/stringmap.hpp"
#include <stack>

class ResourceMemoryTrace
{
	public:
		static ResourceMemoryTrace& instance()	{ return s_instance_; }
		void	begin( const std::string& name );
		void	end();

		//public interface for walking the memory trace
		class TreeWalker
		{
		public:
			//return true if you want to traverse this leaf's children.
			virtual bool	atLeaf( const std::string& id, uint32 memUsed, uint32 depth ) = 0;
		};

		void	traverse( TreeWalker& w, uint32 currDepth = 0 );

	private:
		ResourceMemoryTrace();

		class RMap;

		class RMTLeaf
		{
		public:
			RMTLeaf():
				startKB_(0),
				endKB_( 0 ),
				map_( NULL )
			{};

			int32	startKB_;
			int32	endKB_;
			typedef std::pair< std::string, RMTLeaf* > Entry;
			std::vector< Entry > * map_;

			static bool compare( const Entry& one, const Entry& two );
		};

		typedef std::stack<RMTLeaf*>	ResourceTree;
		ResourceTree context_;

		static ResourceMemoryTrace	s_instance_;
};

#endif // USE_MEMORY_TRACER

#endif // RESOURCEMEMORYTRACE_HPP
