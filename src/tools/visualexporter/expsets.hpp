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

	bool readSettings( const std::string &fileName );
	bool writeSettings( const std::string &fileName );
	bool displayDialog( HWND hWnd );

	void setStaticFrame( unsigned int i ) {  staticFrame_ = i; }
	unsigned int staticFrame( void ) const { return staticFrame_; }

	enum ExportMode
	{
		NORMAL = 0,
		STATIC,
		STATIC_WITH_NODES,
		MESH_PARTICLES
	};

	ExportMode	exportMode( ) const { return exportMode_; };
	void		exportMode( ExportMode exportMode ) { exportMode_ = exportMode; };

	enum NodeFilter
	{
		ALL = 0,
		SELECTED,
		VISIBLE
	};

	NodeFilter	nodeFilter( ) const { return nodeFilter_; };
	void		nodeFilter( NodeFilter nodeFilter ) { nodeFilter_ = nodeFilter; };

	bool		allowScale( ) const { return allowScale_; };
	void		allowScale( bool state ) { allowScale_ = state; };

	bool		bumpMapped( ) const { return bumpMapped_; };
	void		bumpMapped( bool state ) { bumpMapped_ = state; };

	bool		fixCylindrical( ) const { return fixCylindrical_; };
	void		fixCylindrical( bool state ) { fixCylindrical_ = state; };

	bool		useLegacyOrientation( ) const { return useLegacyOrientation_; };
	void		useLegacyOrientation( bool state ) { useLegacyOrientation_ = state; };

	bool		keepExistingMaterials() const { return keepExistingMaterials_; }
	void		keepExistingMaterials( bool state ) { keepExistingMaterials_ = state; }

	bool		snapVertices( ) const { return snapVertices_; };
	void		snapVertices( bool state ) { snapVertices_ = state; };

	float		unitScale( ) const;

	void		boneCount( int count ) { boneCount_ = count; }
	int			boneCount() const { return boneCount_; }

	void		visualCheckerEnable( bool enable ) { visualCheckerEnable_ = enable; }
	bool		visualCheckerEnable( ) const { return visualCheckerEnable_; }

	void		visualTypeIdentifier( const std::string& s ) { visualTypeIdentifier_ = s; }


	static ExportSettings& ExportSettings::instance();

private:
	ExportSettings();
	static BOOL CALLBACK dialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK settingsDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	int				boneCount_;

	bool			allowScale_;
	bool			bumpMapped_;
	bool			fixCylindrical_;
	bool			useLegacyOrientation_;
	bool			keepExistingMaterials_;
	bool			snapVertices_;
	bool			visualCheckerEnable_;

	ExportMode		exportMode_;
	NodeFilter		nodeFilter_;

	//the static frame output
	unsigned int	staticFrame_; 

	std::string		referenceNodesFile_;

	std::string		visualTypeIdentifier_;

	ExportSettings(const ExportSettings&);
	ExportSettings& operator=(const ExportSettings&);

	std::string getReferenceFile( HWND hWnd );

	friend std::ostream& operator<<(std::ostream&, const ExportSettings&);
};

#endif
/*expsets.hpp*/