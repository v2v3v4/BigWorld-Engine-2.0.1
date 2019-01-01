/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "engine_statistics.hpp"

#include "resource_manager_stats.hpp"
#include "cstdmf/memory_tracker.hpp"
#include "cstdmf/profile.hpp"
#include "cstdmf/memory_counter.hpp"
#include "moo/render_context.hpp"
#include "moo/texture_manager.hpp"
#include "moo/vertex_formats.hpp"
#include "moo/visual.hpp"
#include "chunk/chunk.hpp"

#include <strstream>
#include <fstream>

#ifndef CODE_INLINE
#include "engine_statistics.ipp"
#endif

EngineStatistics EngineStatistics::instance_;

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )

// -----------------------------------------------------------------------------
// Section: Construction/Destruction
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
EngineStatistics::EngineStatistics() : lastFrameTime_( 1.f ),
	timeToNextUpdate_( 0.f )
{}


/**
 *	Destructor.
 */
EngineStatistics::~EngineStatistics()
{}


// -----------------------------------------------------------------------------
// Section: General
// -----------------------------------------------------------------------------

/**
 *	Utility struct for keeping track of where we are in the DogWatch tree
 */
struct TreeTrack
{
	/// Constructor.
	TreeTrack(
		DogWatchManager::iterator & iA,
		DogWatchManager::iterator & endA,
		uint64 totA ) : i( iA ), end( endA ), tot( totA ), acc( 0 )
	{ }

	DogWatchManager::iterator	i;		///< @todo Comment
	DogWatchManager::iterator	end;	///< @todo Comment
	uint64						tot;	///< @todo Comment
	uint64						acc;	///< @todo Comment
};


/**
 *	Utility class for keeping track of a graph, and drawing it
 */
class GraphTrack
{
public:
	/**
	 *	Constructor.
	 *
	 *	@todo Comment.
	 */
	GraphTrack( DogWatchManager::iterator & stat,
			DogWatchManager::iterator & frame,
			int index ) :
		stat_( stat ), frame_( frame ), index_( index ) {}

	void draw();

	/**
	 *	This method returns the colour that the corresponding graph will be draw
	 *	in.
	 */
	uint32 colour() const
	{
		return s_colours[ index_ ];
	}

private:
	DogWatchManager::iterator	stat_;
	DogWatchManager::iterator	frame_;

	int		index_;

	const static struct Colours : public std::vector<uint32>
	{
		Colours()
		{
			static uint32 ls_colours[] =
			{
				0xffff0000,
				0xff00ff00,
				0xff0000ff,
				0xff00ffff,
				0xffff00ff,
				0xff888800,
				0xffaaaaaa
			};

			this->assign( ls_colours,
				&ls_colours[sizeof(ls_colours)/sizeof(*ls_colours)] );
		}

		const uint32 & operator[]( int i ) const
		{
			return (*(std::vector<uint32>*)this)[ i % this->size() ];
		}
	} s_colours;

};

const GraphTrack::Colours	GraphTrack::s_colours;

/**
 *	This method draws the graph of the given statistic.
 */
void GraphTrack::draw()
{
	BW_GUARD;
	static std::vector< Moo::VertexTL > tlvv;
	//static std::vector< uint16 > indices;

	tlvv.erase( tlvv.begin(), tlvv.end() );
	//indices.erase( indices.begin(), indices.end() );


	double period = stampsPerSecondD() * 2.0;
	double range = stampsPerSecondD() / 20.0;

	Moo::VertexTL tlv;
	tlv.pos_.z = 0;
	tlv.pos_.w = 1;
	tlv.colour_ = this->colour();

	uint64	doneTime = 0;
	uint64	uperiod = uint64(period);
	for (int i = 1; i < 120 && doneTime < uperiod; i++)
	{
		tlv.pos_.x = float( double(int64(doneTime)) / period );
		tlv.pos_.x *= Moo::rc().screenWidth();
		tlv.pos_.x = Moo::rc().screenWidth() - tlv.pos_.x;

		tlv.pos_.y = float( double(int64(stat_.value( i ))) / range );
		tlv.pos_.y *= -Moo::rc().screenHeight();
		tlv.pos_.y += Moo::rc().screenHeight();

		tlvv.push_back( tlv );

		doneTime += frame_.value( i );
	}

	if( tlvv.size() > 1 )
	{

		Moo::Material::setVertexColour();

		Moo::rc().setRenderState( D3DRS_ZENABLE, TRUE );
		Moo::rc().setRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );

		Moo::rc().setVertexShader( NULL );
		Moo::rc().setFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

		Moo::rc().drawPrimitiveUP( D3DPT_LINESTRIP, tlvv.size()-1,
			&tlvv.front(), sizeof( Moo::VertexTL ) );
	}

}

/**
 *	This is a static helper function for App::displayStatistics.
 */
static void displayOneStatistic( XConsole & console, const std::string & label,
	int indent, uint64 value, uint64 overall )
{
	BW_GUARD;
	static char * spaceString = "                         ";

	std::strstream	stream;

	stream << label;
	int extraSpace = strlen(spaceString) - (label.length() + indent);
	if (extraSpace > 0)
	{
		stream << &spaceString[strlen(spaceString) - extraSpace];
	}

	char	percent[16];
	bw_snprintf( percent, sizeof(percent), "%4.1f%%", value * 100.f / overall );
	stream << percent << " (" << NiceTime( value ) << ")";
	stream << std::endl << std::ends;
	console.setCursor( indent, console.cursorY() );
	console.print( stream.str() );

	stream.freeze( 0 );
}


/**
 *	This method displays the statistics associated with the frame on the input
 *	console.
 *
 *	@param console	The console on which the statistics should be displayed.
 */
void EngineStatistics::displayStatistics( XConsole & console )
{	
	BW_GUARD;
	static const size_t STR_SIZE = 64;
	char statString[STR_SIZE];
	statString[STR_SIZE - 1] = 0;

	console.setCursor( 0, 0 );
	console.print( "Realtime Profiling Console\n" );

	float framesPerSecond = lastFrameTime_ > 0.f ? 1.f/lastFrameTime_ : 0.f;

	// Frames Per Second
	_snprintf( statString, STR_SIZE - 1, "Fps      : %2.2f \n", framesPerSecond );
	console.print( statString );

	// Triangles
	
	const Moo::DrawcallProfilingData& pd = Moo::rc().lastFrameProfilingData();
	uint32 primsPerDrawcall = 0;
	if (pd.nDrawcalls_ != 0)
		primsPerDrawcall = pd.nPrimitives_ / pd.nDrawcalls_;

	_snprintf( statString, STR_SIZE - 1, "DrawCalls: %d Primitives: %d Primitives/DrawCall: %d\n",
		pd.nDrawcalls_, pd.nPrimitives_, primsPerDrawcall );

	console.print( statString );

	// Texture memory
	bw_snprintf( statString, STR_SIZE - 1, "Memory:\n"
										   " Texture (Total):                   %6d KB\n",
		Moo::TextureManager::instance()->textureMemoryUsed() / 1024 );
	console.print( statString );
	_snprintf( statString, STR_SIZE - 1,   " Texture (Frame):                   %6d KB\n",
		Moo::ManagedTexture::totalFrameTexmem_ / 1024 );
	console.print( statString );

#ifdef ENABLE_MEMTRACKER
	// Heap memory allocations
	MemTracker::AllocStats heapStats;
	MemTracker::instance().readStats( heapStats );

	bw_snprintf( statString, STR_SIZE - 1, " Heap Memory/Allocations (Current): %6d KB /%6d allocs\n",
		heapStats.curBytes_/1024, heapStats.curBlocks_ );
	console.print( statString );
#endif

	bw_snprintf( statString, STR_SIZE - 1, "Chunks/Items:\n");
	console.print( statString );
	bw_snprintf( statString, STR_SIZE - 1, " Chunk Instances (Current): %d\n",
		Chunk::s_instanceCount_ );
	console.print( statString );
	bw_snprintf( statString, STR_SIZE - 1, " Chunk Instances (Peak):    %d\n",
		Chunk::s_instanceCountPeak_ );
	console.print( statString );

	bw_snprintf( statString, STR_SIZE - 1, " Item Instances  (Current): %d\n",
		ChunkItemBase::s_instanceCount_ );
	console.print( statString );
	bw_snprintf( statString, STR_SIZE - 1, " Item Instances  (Peak):    %d\n",
		ChunkItemBase::s_instanceCountPeak_ );
	console.print( statString );


	// DirectX Resource Manager statistics	
	if (ResourceManagerStats::instance().enabled())	
	{
		ResourceManagerStats::instance().displayStatistics( console );		
	}

	// DogWatches

	// we use flags as:
	// 1: children visible
	// 2: graphed
	// 4: selected


	// first figure out how many frames ago 1s was
	DogWatchManager::iterator fIter = DogWatchManager::instance().begin();
	uint64	fAcc = 0;
	int		period;
	for (period = 1; period < 60 && fAcc < stampsPerSecond()/2; period++)
	{
		fAcc += fIter.value( period );
	}

	uint64		overall = max( uint64(1), fIter.average( period, 1 ) );
//	uint64		average = fIter.average( period, 1 );
//	uint64		overall = average > 1 ? average : 1;

	double lastFrameTime = double(fIter.value( 1 )) / stampsPerSecondD();

	int	numGraphs = 0;
	int selectedLine = -1;
	int headerLines = console.cursorY();

	// now print out the tree
	std::vector<TreeTrack>	stack;

    DogWatchManager::iterator dwmBegin = DogWatchManager::instance().begin();
    DogWatchManager::iterator dwmEnd = DogWatchManager::instance().end();

    TreeTrack treeTrack( dwmBegin, dwmEnd, (uint64)0 );
	
	stack.push_back( treeTrack );

	while (!stack.empty())
	{
		int iIndex = stack.size()-1;

		// have we come to the end of this lot?
		TreeTrack	&i = stack.back();
		if (i.i == i.end)
		{
			if (i.acc != 0 && i.tot != 0)
			{
				if (i.acc > i.tot) i.acc = i.tot;
				displayOneStatistic( console, "<Remainder>", stack.size()-1,
					i.tot - i.acc, overall );
			}

			stack.pop_back();

			continue;
		}

		bool	setColour = false;

		// change our colour if we're selected
		if (i.i.flags() & 2)
		{		// and draw a graph if we're doing that
			GraphTrack graph( i.i, fIter, numGraphs++ );
			graph.draw();

			console.lineColourOverride( console.cursorY(),
				graph.colour() );
			setColour = true;
		}

		if (i.i.flags() & 4)
		{
			selectedLine = console.cursorY();
			console.lineColourOverride( console.cursorY(), 0xFFFFFFFFU );
			setColour = true;
		}

		if (!setColour)
		{
			console.lineColourOverride( console.cursorY() );
		}

		// figure out what we're going to call it
		std::string	label( i.i.name() );
		if (!(i.i.flags() & 1) && i.i.begin() != i.i.end())
		{
			label.append( "..." );
		}

		// ok, print out i.i then
		uint64	val = i.i.average( period, 1 );
		displayOneStatistic( console, label, stack.size()-1,
			val, overall );
		i.acc += val;

		// do we draw our children?
		if (i.i.flags() & 1)
		{
			// fix up the stack
            DogWatchManager::iterator iiBegin =
            		i.i.begin();
		    DogWatchManager::iterator iiEnd =
            		i.i.end();

			stack.push_back( TreeTrack( iiBegin, iiEnd, (uint64)val ) );
		}

		// move on to the next one
		//  re-get reference after vector modification!
		++(stack[iIndex].i);
	}

	// scroll the console if the selection is off screen
	const int scro = console.scrollOffset();
	if (selectedLine <= scro + headerLines)
		console.scrollUp();
	if (selectedLine >= scro + console.visibleHeight()-2) // extra for frame remainder
		console.scrollDown();
}


/**
 *	This method updates the engine statistics given that the input number of
 *	seconds has elapsed.
 */
void EngineStatistics::tick( float dTime )
{
	BW_GUARD;
	// How often, in second, the frame-rate is updated.
	const float UPDATE_PERIOD = 1.f;

	timeToNextUpdate_ -= dTime;

	if (timeToNextUpdate_ < 0.f)
	{
		lastFrameTime_ = dTime;
		timeToNextUpdate_ = UPDATE_PERIOD;
	}

	DogWatchManager::instance().tick();
}

bool EngineStatistics::logSlowFrames_ = false;

// -----------------------------------------------------------------------------
// Section: Streaming
// -----------------------------------------------------------------------------

/**
 *	Streaming operator for EngineStatistics.
 */
std::ostream& operator<<(std::ostream& o, const EngineStatistics& t)
{
	BW_GUARD;
	o << "EngineStatistics\n";
	return o;
}

// engine_statistics.cpp