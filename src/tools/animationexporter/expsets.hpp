/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EXPSETS_HPP
#define EXPSETS_HPP

#pragma warning ( disable : 4530 )

#include <iostream>

#include <windows.h>

class ExportSettings
{
public:
	~ExportSettings();

	bool readSettings( const char* fileName );
	bool writeSettings( std::string fileName );
	bool displayDialog( HWND hWnd );

	const std::string &referenceNodesFile( void ) const { return referenceNodesFile_; }

	enum NodeFilter
	{
		ALL = 0,
		SELECTED,
		VISIBLE
	};

	void setFrameRange( int first, int last ) { frameFirst_ = first; frameLast_ = last; }

	int firstFrame() const { return frameFirst_; };
	int lastFrame() const { return frameLast_; };

	void setStaticFrame( int i ) { staticFrame_ = i; }
	int staticFrame( void ) const { return staticFrame_; }
	
	float	unitScale( ) const;

	NodeFilter	nodeFilter( ) const { return nodeFilter_; };
	void		nodeFilter( NodeFilter nodeFilter ) { nodeFilter_ = nodeFilter; };

	bool		allowScale( ) const { return allowScale_; };
	void		allowScale( bool state ) { allowScale_ = state; };

	bool		exportMorphAnimation() const { return exportMorphAnimation_; }
	void		exportMorphAnimation( bool state ) { exportMorphAnimation_ = state; }

	bool		exportNodeAnimation() const { return exportNodeAnimation_; }
	void		exportNodeAnimation( bool state ) { exportNodeAnimation_ = state; }

	bool		useLegacyOrientation() const { return useLegacyOrientation_; }
	void		useLegacyOrientation( bool state ) { useLegacyOrientation_ = state; }

	bool		exportCueTrack() const { return exportCueTrack_; }

	static ExportSettings& ExportSettings::instance();

private:
	ExportSettings();
	static BOOL CALLBACK dialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK settingsDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool			allowScale_;
	bool			exportMorphAnimation_;
	bool			exportNodeAnimation_;
	bool			exportCueTrack_;	
	bool			useLegacyOrientation_;	

	NodeFilter		nodeFilter_;

	int	staticFrame_; //the static frame output
	int	frameFirst_;
	int	frameLast_;

	std::string	referenceNodesFile_;
	std::string getReferenceFile( HWND hWnd );
	void displayReferenceHierarchyFile( HWND hWnd ) const;

	ExportSettings(const ExportSettings&);
	ExportSettings& operator=(const ExportSettings&);

	friend std::ostream& operator<<(std::ostream&, const ExportSettings&);
};

#endif
/*expsets.hpp*/
