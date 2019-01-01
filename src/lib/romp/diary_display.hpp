/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DIARY_DISPLAY_HPP
#define DIARY_DISPLAY_HPP

#include "cstdmf/diary.hpp"
#include "custom_mesh.hpp"
#include "font.hpp"

/**
 *	This class displays the contents of a diary
 */
class DiaryDisplay
{
public:
	static void displayAll();

	class XScale
	{
	public:
		XScale( uint64 offset, float scale );
		void syncOffset( int64 o );
		int worldToScreen( uint64 t );
		bool clipInterval( int& startX, int& stopX );
		double timeInSeconds( uint64 t );
	private:
		double worldToClip( uint64 t );		
		int clipToScreen( double clip );
		double syncOffset_;
		double offset_;
		double scale_;
		double halfScreenWidth_;
		double sps_;
	};

	class YScale
	{
	public:
		YScale( float offset, float scale );
		int worldToScreen( int y );
		bool clipInterval( int& startY, int& stopY );
	private:
		float offset_;
		float scale_;
		float halfScreenHeight_;
	};

	class XAxis
	{
	public:
		XAxis( uint64 first );		

		void addTimeLabel(int pixX, uint64 t );
		void addTimeGridLine(int pixX);

		void addGlobalLabel(int pixX, const std::string& msg );		
		void addGlobalGridLine(int pixX);

		void drawTimeAxis();
		void drawGlobalsAxis();
	private:
		float timeInSeconds( uint64 t )
		{		
			return float( double(t) * sps_ );
		}
		FontPtr font_;
		int last_;
		int lastGridLine_;
		double sps_;
		uint64 first_;		
		CustomMesh<Moo::VertexXYZDUV2> timeLabelsMesh_;		
		CustomMesh<Moo::VertexXYZDUV2> globalLabelsMesh_;
		CustomMesh< Moo::VertexTL > timeLineMesh_;
		CustomMesh< Moo::VertexTL > globalLineMesh_;
	};

	class EntryType
	{
	public:
		static EntryType& findEntryType( const std::string& desc );
		static std::string type( const std::string& desc );
		static Moo::Colour& colour( const std::string& desc );

		EntryType( const std::string& type );
		void addSample( double timeTaken );		

		Moo::Colour colour_;
		double totalTime_;		
		double nSamples_;
		double minTime_;
		double maxTime_;

		typedef std::map< std::string, EntryType > EntryTypeMap;
		static EntryTypeMap s_types;
		static Moo::Colour s_lineColours[14];
		static int s_colourIdx;
	};

	static void displayBk(float height, int maxLevels, uint32 idx);

	static void clearDisplay();
	static void calculateDisplay( const DiaryEntries & entries, float height, XAxis& xAxis, FontPtr font, uint64 now = timestamp(), int64 syncOffset = 0 );
	static void drawAxis(XAxis& xAxis, bool drawGlobals, bool drawTimes);
	static void drawDisplay( FontPtr font );

	static void calculateStatistics( const DiaryEntries & entries );
	static void drawStatistics();

	static CustomMesh<Moo::VertexXYZDUV2> textMesh_;
	static CustomMesh<Moo::VertexTL> mesh_;
};


#endif // DIARY_DISPLAY_HPP
