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
#include "diary_display.hpp"
#include <strstream>

#include "geometrics.hpp"
#include "moo/render_context.hpp"
#include "math/colour.hpp"
#include "romp/custom_mesh.hpp"
#include "romp/font_manager.hpp"
#include "romp/font_metrics.hpp"

float DD_XSCALE = 1.f;
float DD_YSLICE = 0.1f;

CustomMesh<Moo::VertexXYZDUV2> DiaryDisplay::textMesh_(D3DPT_TRIANGLELIST);
CustomMesh<Moo::VertexTL> DiaryDisplay::mesh_(D3DPT_TRIANGLELIST);

// -----------------------------------------------------------------------------
// Section: DiaryDisplay::XScale
// -----------------------------------------------------------------------------
DiaryDisplay::XScale::XScale( uint64 offset, float scale )
{
	halfScreenWidth_ = (double)Moo::rc().screenWidth() / 2.0;
	sps_ = 1.0 / stampsPerSecondD();
	//TODO : offset_ is more like an anchor point than an offset
	offset_ = timeInSeconds( offset );
	scale_ = (double)scale;	
}


void DiaryDisplay::XScale::syncOffset( int64 o )
{
	if (o<0)
		syncOffset_ = timeInSeconds( (uint64)(-o) );
	else
		syncOffset_ = -timeInSeconds( (uint64)(o) );
}


double DiaryDisplay::XScale::worldToClip( uint64 t )
{
	if (t == 0)
		return 0.f;

	double x = this->timeInSeconds(t) + syncOffset_;
	return (x - offset_) * scale_;
}

int DiaryDisplay::XScale::clipToScreen( double x )
{
	return (int)((x + 1.0) * halfScreenWidth_);
}

int DiaryDisplay::XScale::worldToScreen( uint64 x )
{
	double clip = this->worldToClip(x);
	return clipToScreen(clip);		
}

bool DiaryDisplay::XScale::clipInterval( int& startX, int& stopX )
{
	if (startX > stopX)
		return false;
	if (stopX < 0)
		return false;
	if (startX > (int)halfScreenWidth_ * 2)
		return false;
	startX = max(startX, 0);
	//don't clip the right hand side of blocks
	//stopX = min(stopX, (int)halfScreenWidth_*2);
	return true;
}

double DiaryDisplay::XScale::timeInSeconds( uint64 t )
{		
	return ( double(t) * sps_ );
}

// -----------------------------------------------------------------------------
// Section: DiaryDisplay::YScale
// -----------------------------------------------------------------------------
DiaryDisplay::YScale::YScale( float offset, float scale )
{
	offset_ = offset;
	scale_ = scale;
	halfScreenHeight_ = Moo::rc().screenHeight()/2;
}

int DiaryDisplay::YScale::worldToScreen( int y )
{
	float clipY = offset_ - float(y) * scale_;
	return (int)((1.f - clipY) * halfScreenHeight_);
}

bool DiaryDisplay::YScale::clipInterval( int& startY, int& stopY )
{
	if (startY > stopY)
		return false;
	if (stopY < 0)
		return false;
	if (startY > (int)halfScreenHeight_ * 2)
		return false;
	startY = max(startY, 0);
	stopY = min(stopY, (int)halfScreenHeight_*2);
	return true;
}


//-----------------------------------------------------------------------------
//	Section : Diary Display Entry Type
//-----------------------------------------------------------------------------
Moo::Colour DiaryDisplay::EntryType::s_lineColours[14] = {
	Moo::Colour( 1.f, 0.25f, 0.25f, 1.f ),
	Moo::Colour( 0.25f, 1.f, 0.25f, 1.f ),
	Moo::Colour( 0.25f, 0.25f, 1.f, 1.f ),
	Moo::Colour( 1.f, 0.25f, 1.f, 1.f ),
	Moo::Colour( 1.f, 1.f, 0.25f, 1.f ),
	Moo::Colour( 0.25f, 1.f, 1.f, 1.f ),
	Moo::Colour( 0.5f, 0.5f, 0.5f, 1.f ),
	Moo::Colour( 1.f, 1.f, 1.f, 1.f ),
	Moo::Colour( 1.f, 0.5f, 0.f, 1.f ),
	Moo::Colour( 0.f, 0.5f, 1.f, 1.f ),
	Moo::Colour( 0.f, 1.f, 0.5f, 1.f ),
	Moo::Colour( 1.f, 0.f, 0.5f, 1.f ),
	Moo::Colour( 0.5f, 1.f, 0.f, 1.f ),
	Moo::Colour( 0.5f, 0.f, 1.f, 1.f )
};


std::string DiaryDisplay::EntryType::type( const std::string& desc )
{
	int i = desc.find_first_of(" ");
	return (i!=-1) ? desc.substr(0,i) : desc;
}


Moo::Colour& DiaryDisplay::EntryType::colour( const std::string& desc )
{
	DiaryDisplay::EntryType& et = findEntryType( desc );
	return et.colour_;
}


int DiaryDisplay::EntryType::s_colourIdx = 0;
DiaryDisplay::EntryType::EntryTypeMap DiaryDisplay::EntryType::s_types;


//Constructor.
DiaryDisplay::EntryType::EntryType( const std::string& type ):
	minTime_(std::numeric_limits<double>::max()),
	maxTime_(std::numeric_limits<double>::min()),
	totalTime_(0.0),	
	nSamples_(0.0)
{
	colour_ = s_lineColours[s_colourIdx++%14];
}


//Get EntryType for the given diary entry
DiaryDisplay::EntryType& DiaryDisplay::EntryType::findEntryType( const std::string& desc )
{
	std::string t = type(desc);
	EntryTypeMap::iterator it = s_types.find(t);
	if (it == s_types.end())
	{
		s_types.insert( std::make_pair(t,EntryType(t)) );
		it = s_types.find(t);
	}
	return it->second;	
}


void DiaryDisplay::EntryType::addSample( double timeTaken )
{
	totalTime_ += timeTaken;
	nSamples_ += 1.0;
	minTime_ = min( minTime_, timeTaken );
	maxTime_ = max( maxTime_, timeTaken );
}


//-----------------------------------------------------------------------------
//	Section : Diary Display
//-----------------------------------------------------------------------------
/**
 *	Static display all method
 */
void DiaryDisplay::displayAll()
{
	static int shouldDisplayDiaries = -1;
	static bool shouldDisplayXAxisTimes = true;
	if (shouldDisplayDiaries == -1)
	{
		shouldDisplayDiaries = 0;
		MF_WATCH( "Diaries/enabled", Diary::s_enabled_, Watcher::WT_READ_WRITE,
			"Enable event diary" );
		MF_WATCH( "Diaries/display", shouldDisplayDiaries, Watcher::WT_READ_WRITE,
			"Display event diary" );
		MF_WATCH( "Diaries/x axis times", shouldDisplayXAxisTimes, Watcher::WT_READ_WRITE,
			"Display x axis times" );
		MF_WATCH( "Diaries/xscale", DD_XSCALE, Watcher::WT_READ_WRITE,
			"The x axis scale of the diary" );
		MF_WATCH( "Diaries/yslice", DD_YSLICE, Watcher::WT_READ_WRITE,
			"The y axis scale of the diary" );		
	}
	if (!shouldDisplayDiaries) return;

	std::vector<Diary*> diaries;
	Diary::look( diaries );

	uint disz = diaries.size();
	float heightEach = 2.f / float(disz+1);
	float height = 1.f - heightEach * 0.5f;

	DiaryEntries entries;
	diaries[0]->look( entries );
	DiaryEntry & de = *entries[0];
	XAxis xAxis( de.start_ );

	//	
	double sps = stampsPerSecondD();
	double txscale = (sps / DD_XSCALE);
	uint64 curEnd = timestamp();
	curEnd -= int64(txscale);	

	//
	static FontPtr font = FontManager::instance().get( "system_medium.font" );

	DiaryDisplay::clearDisplay();

	for (uint i = 0; i < disz; i++)
	{
		diaries[i]->look( entries );
		DiaryDisplay::calculateDisplay( entries, height, xAxis, font, curEnd );		
		height -= heightEach;
	}

	DiaryDisplay::drawAxis(xAxis, true, shouldDisplayXAxisTimes);
	DiaryDisplay::drawDisplay( font );
	
}


static Moo::Colour s_bkColours[2] = {
	Moo::Colour( 0.1f, 0.1f, 0.1f, 0.75f ),
	Moo::Colour( 0.1f, 0.1f, 0.1f, 0.5f )
};


void DiaryDisplay::displayBk( float height, int maxLevels, uint32 colourIdx )
{		
	float screenWidth = Moo::rc().screenWidth();
	float halfScreenHeight = Moo::rc().screenHeight()/2;

	float level0 = height;
	float level1 = height - (float)(maxLevels+1) * DD_YSLICE;

	int startPixY = (int)((1.f - (level0)) * halfScreenHeight);
	int stopPixY = (int)((1.f - (level1)) * halfScreenHeight);

	//draw background
	Geometrics::drawRect(
		Vector2( (float)0, (float)startPixY ),
		Vector2( (float)screenWidth, (float)stopPixY ),
		s_bkColours[colourIdx%2] );
}


void DiaryDisplay::drawAxis( XAxis& xAxis, bool drawGlobals, bool drawTimes )
{
	//Draw cached x axis grid lines.	
	if (drawTimes)
		xAxis.drawTimeAxis();

	if (drawGlobals)
		xAxis.drawGlobalsAxis();
}


void DiaryDisplay::drawDisplay( FontPtr font )
{
	//Draw Blocks
	if (!mesh_.empty())
	{
		Geometrics::setVertexColour( Moo::Colour(1,1,1,0.5f) );
		mesh_.draw();
	}

	//Draw text labels
	if (!textMesh_.empty())
	{
		if (FontManager::instance().begin( *font, 0, false ))
		{
			textMesh_.drawEffect();	
			FontManager::instance().end();
		}
	}
}


/**
 *	This method calculates min, max, averages and variations for all
 *	entry types.
 */
void DiaryDisplay::calculateStatistics( const DiaryEntries & entries )
{
	for (int i = int(entries.size())-1; i >= 0; i--)
	{
		DiaryEntry & de = *entries[i];

		if (de.start_ == 0)	//erroneous entry
			continue;

		static double sps = 1.0 / stampsPerSecondD();
		double time = double(de.stop_ - de.start_) * sps;		
		
		EntryType& et = EntryType::findEntryType( de.desc_ );
		et.addSample( time );
	}
}


/**
 *	This method draws the statistics summary onto the screen
 */
void DiaryDisplay::drawStatistics()
{
	static FontPtr myfont = FontManager::instance().get( "system_medium.font" );	
	CustomMesh<Moo::VertexXYZDUV2> textMesh_;
	
	float y = 1.f;
	float w = 0.f;
	float h = 0.f;
	float maxX = -1.f;

	EntryType::EntryTypeMap::iterator it = EntryType::s_types.begin();
	EntryType::EntryTypeMap::iterator end = EntryType::s_types.end();
	while (it != end)
	{
		EntryType& et = it->second;
		wchar_t buf[256];
		bw_snwprintf( buf, sizeof(buf)/sizeof(wchar_t), L"%S - min %2.4f, avg %2.4f, max %2.4f, total %2.4f, num %d", it->first.c_str(), et.minTime_, et.totalTime_ / et.nSamples_, et.maxTime_, et.totalTime_, (int)(et.nSamples_ + 0.5) );
		myfont->colour( et.colour_ );
		myfont->drawIntoMesh( textMesh_, buf, -1.f, y, &w, &h );
		maxX = max( maxX, w );
		y -= h;
		it++;
	}
	
	float sx = (maxX + 1.f) * Moo::rc().halfScreenWidth();	
	float sy = (1.f - y) * Moo::rc().halfScreenHeight();

	//draw background
	Geometrics::drawRect(
		Vector2( 0.f, 0.f ),
		Vector2( sx, sy ),
		s_bkColours[0] );

	//draw text
	if (FontManager::instance().begin( *myfont ))
	{
		textMesh_.drawEffect();
		FontManager::instance().end();
	}	
}


//font drawing into mesh uses clip coords not screen coords.
//convert pixels to clip
void pixelsToClip( int pixX, int pixY, Vector2& retClipXY )
{		
	float halfx = Moo::rc().halfScreenWidth();
	float halfy = Moo::rc().halfScreenHeight();
	retClipXY.x = ((float)max(pixX,0) - halfx) / halfx;
	retClipXY.y = (halfy - (float)pixY) / halfy;
}


void DiaryDisplay::clearDisplay()
{
	mesh_.clear();
	textMesh_.clear();
}


/**
 *	Static calculateDisplay method.
 *
 *	entries		-
 *	height		-
 *	xAxis		- 
 *	now			-  
 */
void DiaryDisplay::calculateDisplay( const DiaryEntries & entries,
	float height, XAxis& xAxis, FontPtr font, uint64 now, int64 syncOffset )
{	
	XScale xScale( now, DD_XSCALE );
	YScale yScale( height, DD_YSLICE );
	xScale.syncOffset(syncOffset);
	int screenWidth = (int)Moo::rc().screenWidth();

	//Draw blocks
	for (int i = int(entries.size())-1; i >= 0; i--)
	{
		DiaryEntry & de = *entries[i];

		if (de.start_ == 0)	//erroneous entry
			continue;

		int startPixX = xScale.worldToScreen( de.start_ );
		int stopPixX = de.stop_ == 0 ? (int)Moo::rc().screenWidth() : xScale.worldToScreen( de.stop_ );

		if (!xScale.clipInterval( startPixX, stopPixX ) )
			continue;

		int startPixY = yScale.worldToScreen( de.level_ );
		int stopPixY = yScale.worldToScreen( de.level_ + 1 );

		if (!yScale.clipInterval( startPixY, stopPixY ))
			continue;
		
		Moo::Colour& col = EntryType::colour( de.desc_ );
		
		Geometrics::createRectMesh(
			Vector2( (float)startPixX, (float)startPixY ),
			Vector2( (float)stopPixX, (float)stopPixY ),
			col * 0.7f,				
			mesh_ );	

		//turn strip into a list (need to see how createRectMesh
		//works to get the maths below... but we are basically
		//duplicating the appropriate vertices to fully form the
		//second triangle in the list.)
		mesh_.push_back( mesh_[ mesh_.size()-2 ] );
		mesh_.push_back( mesh_[ mesh_.size()-4 ] );
	}	

	//Draw text labels	
	int sHeight = yScale.worldToScreen(1)  - yScale.worldToScreen(0);
	float fHeight = font->metrics().clipHeight() * Moo::rc().halfScreenHeight();
	int nRows = (int)( (float)sHeight / fHeight );	

	for (int i = int(entries.size())-1; i >= 0; i--)
	{
		DiaryEntry & de = *entries[i];	

		if (de.start_ == 0)	//erroneous entry
			continue;

		int startPixX = xScale.worldToScreen( de.start_ );
		int stopPixX = de.stop_ == 0 ? screenWidth : xScale.worldToScreen( de.stop_ );

		if (!xScale.clipInterval( startPixX, stopPixX ) )
			continue;

		int startPixY = yScale.worldToScreen( de.level_ );
		int stopPixY = yScale.worldToScreen( de.level_ + 1 );

		if (!yScale.clipInterval( startPixY, stopPixY ))
			continue;

		Moo::Colour& col = EntryType::colour( de.desc_ );		

		bool simpleDraw = false;
		int width = stopPixX - startPixX;

		//don't clip text off the right of the screen if
		//the entry hasn't been stopped yet.
		if (de.stop_ == 0)
			width = 100000;

		//
		int npos = de.desc_.find_first_of(" ");
		std::string t = de.desc_.substr(0,npos);
		if (t == "Global")
			xAxis.addGlobalLabel( startPixX, de.desc_.substr(npos+1, de.desc_.size()));
		
		if (nRows > 0 && width > 1)
		{
			Vector2 clipPos;
			::pixelsToClip( startPixX, startPixY, clipPos );
			
			//add x-axis labels at the start of each frame
			if ( t == "Frame" )
				xAxis.addTimeLabel( startPixX, de.start_ );
			
			//draw the type and the description			
			int sWidth = font->metrics().stringWidth( t );
			std::string label;

			if (width > sWidth)
			{
				//Enough room for at least the type
				sWidth = font->metrics().stringWidth( de.desc_ );
				if (width > sWidth)
				{
					//Enough room for at the type and further description
					label = de.desc_;					
				}
				else
				{
					//Just draw the type
					label = t;					
				}				
			}
			else
			{							
				//Not enough room for the type
				sWidth = font->metrics().stringWidth(t.substr(0,1));
				if (width > sWidth )
				{
					//enough to draw just one letter
					label = t.substr(0,1);					
				}
			}

			if ( !label.empty() )
			{
				wchar_t buf[256];
				MF_ASSERT( label.size() < 256 );
				bw_snwprintf( buf, sizeof(buf)/sizeof(wchar_t), L"%S\0", label.c_str() );
				font->colour( Colour::getUint32FromNormalised( (Vector4&)col ) );
				font->drawIntoMesh( textMesh_, buf, clipPos.x, clipPos.y );
			}

			//draw the time taken
			if (nRows > 1)
			{
				std::strstream	ss;
				if (de.stop_ > de.start_)
					ss << NiceTime( de.stop_ - de.start_ ) <<  std::ends;
				else
					ss << NiceTime( timestamp() - de.start_ ) <<  std::ends;
				int sWidth = font->metrics().stringWidth( ss.str() );
				if (width > sWidth)
				{
					wchar_t buf[256];					
					bw_snwprintf( buf, sizeof(buf)/sizeof(wchar_t), L"%S\0", ss.str() );
					clipPos.y -= font->metrics().clipHeight();
					font->drawIntoMesh( textMesh_, buf, clipPos.x, clipPos.y );
				}
			}
		}		
	}	
}


//-----------------------------------------------------------------------------
//	Section : Diary Display XAxis
//-----------------------------------------------------------------------------
DiaryDisplay::XAxis::XAxis(uint64 first):
	last_( -(1<<16) ),
	lastGridLine_( -(1<<16) ),
	first_( first ),
	sps_( 1.0 / stampsPerSecondD() ),
	timeLineMesh_( D3DPT_LINELIST ),
	globalLineMesh_( D3DPT_LINELIST ),
	timeLabelsMesh_( D3DPT_TRIANGLELIST ),
	globalLabelsMesh_( D3DPT_TRIANGLELIST )
{
	font_ = FontManager::instance().get( "system_medium.font" );
}


void DiaryDisplay::XAxis::addTimeLabel(int pixX, uint64 t )
{	
	float secs = timeInSeconds( t - first_ );	
	static int xMargin = 2;
	int fHeight = (int)font_->metrics().heightPixels();
	font_->colour( 0xb0b0b0b0 );

	char buf[256];
	bw_snprintf( buf, sizeof(buf), "%2.3f", secs );	
	int pixW = font_->metrics().stringWidth( buf );	

	if ( (last_==(-(1<<16))) || (pixX+pixW)<last_)
	{
		Vector2 clipPos;
		::pixelsToClip( pixX, (int)Moo::rc().screenHeight(), clipPos );
		clipPos.y += font_->metrics().clipHeight();

		wchar_t wbuf[256];
		bw_snwprintf( wbuf, sizeof(wbuf)/sizeof(wchar_t), L"%S",buf );				
		font_->drawIntoMesh( timeLabelsMesh_, wbuf, clipPos.x, clipPos.y );		
		last_ = pixX - xMargin;
	}

	this->addTimeGridLine(pixX);
}


void DiaryDisplay::XAxis::addGlobalLabel(int pixX, const std::string& msg )
{	
	font_->colour( 0xb0b0b0b0 );	
	Vector2 clipPos;
	::pixelsToClip( pixX, 0, clipPos );	
	wchar_t wbuf[256];
	bw_snwprintf( wbuf, sizeof(wbuf)/sizeof(wchar_t), L"%S",msg.c_str() );
	font_->drawIntoMesh( globalLabelsMesh_, wbuf, clipPos.x, clipPos.y );
	this->addGlobalGridLine(pixX);
}


void DiaryDisplay::XAxis::addTimeGridLine(int pixX)
{
	static int xMargin = 5;

	if ( pixX == lastGridLine_)
		return;		

	Moo::VertexTL v[2];	

	v[0].pos_.set( (float)pixX, (float)0, 0.f, 0.f );
	v[1].pos_.set( (float)pixX, (float)Moo::rc().screenHeight(), 0.f, 0.f );

	if ( (lastGridLine_==(-(1<<16))) || (pixX<lastGridLine_))
	{		
		v[0].colour_ = v[1].colour_ = 0x40808080;	
		lastGridLine_ = pixX - xMargin;
	}
	else
	{
		v[0].colour_ = v[1].colour_ = 0x20808080;
	}

	timeLineMesh_.push_back( v[0] );
	timeLineMesh_.push_back( v[1] );
}


void DiaryDisplay::XAxis::addGlobalGridLine(int pixX)
{
	Moo::VertexTL v[2];	

	v[0].pos_.set( (float)pixX, (float)0, 0.f, 0.f );
	v[1].pos_.set( (float)pixX, (float)Moo::rc().screenHeight(), 0.f, 0.f );
	v[0].colour_ = v[1].colour_ = 0x80808080;

	globalLineMesh_.push_back( v[0] );
	globalLineMesh_.push_back( v[1] );
}


void DiaryDisplay::XAxis::drawGlobalsAxis()
{
	Geometrics::setVertexColour( Moo::Colour(1,1,1,0.5f) );	
	globalLineMesh_.draw();
	if ( FontManager::instance().begin(*font_ ) )
	{
		globalLabelsMesh_.drawEffect();
		FontManager::instance().end();
	}
}

void DiaryDisplay::XAxis::drawTimeAxis()
{
	Geometrics::setVertexColour( Moo::Colour(1,1,1,0.5f) );
	timeLineMesh_.draw();		
	if (FontManager::instance().begin( *font_ ))
	{
		timeLabelsMesh_.drawEffect();
		FontManager::instance().end();
	}
}

// diary_display.cpp
