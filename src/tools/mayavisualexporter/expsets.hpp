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

#include <maya/MDistance.h>


class ExportSettings
{
public:
	~ExportSettings();

	void setSettingFileName( const std::string& settingFilename )
	{
		settingFilename_ = settingFilename;
	}
	bool readSettings();
	bool writeSettings();

	void setExportMeshes( bool b );
	void setExportEnvelopesAndBones( bool b );
	void setExportNodes( bool b );
	void setExportMaterials( bool b );
	void setExportAnimations( bool b );
	void setUseCharacterMode( bool b );

	void setIncludePortals( bool b );
	void setWorldSpaceOrigin( bool b );
	void setResolvePaths( bool b );
	void setUnitScale( float f );
	void setLocalHierarchy( bool b );

	void setAnimationName( const std::string &s );

	void setStaticFrame( unsigned int i );
	void setFrameRange( unsigned int first, unsigned int last );

	bool includeMeshes( void ) const;
	bool includeEnvelopesAndBones( void ) const;
	bool includeNodes( void ) const;
	bool includeMaterials( void ) const;
	bool includeAnimations( void ) const;
	bool useCharacterMode( void ) const;
	bool includePortals( void ) const;
	bool worldSpaceOrigin( void ) const;
	bool resolvePaths( void ) const;
	float unitScale( ) const;
	bool localHierarchy( ) const;

	const std::string &animationName( void ) const;

	const std::string &referenceNodesFile( void ) const;

	unsigned int staticFrame( void ) const;


	enum ExportMode
	{
		NORMAL = 0,
		STATIC,
		STATIC_WITH_NODES,
		MESH_PARTICLES
	};

	bool	exportAnim() const { return exportAnim_; };
	void		exportAnim( bool exportAnim ) { exportAnim_ = exportAnim; };

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

	bool		keepExistingMaterials( ) const { return keepExistingMaterials_; };
	void		keepExistingMaterials( bool state ) { keepExistingMaterials_ = state; };

	bool		snapVertices( ) const { return snapVertices_; };
	void		snapVertices( bool state ) { snapVertices_ = state; };

	bool		stripRefPrefix( ) const { return stripRefPrefix_; };
	void		stripRefPrefix( bool state ) { stripRefPrefix_ = state; };

	bool		referenceNode( ) const { return referenceNode_; };
	void		referenceNode( bool state ) { referenceNode_ = state; };

	bool		disableVisualChecker( ) const { return disableVisualChecker_; };
	void		disableVisualChecker( bool state ) { disableVisualChecker_ = state; };

	bool		useLegacyScaling( ) const { return useLegacyScaling_; };
	void		useLegacyScaling( bool state ) { useLegacyScaling_ = state; };

	bool		useLegacyOrientation( ) const { return useLegacyOrientation_; };
	void		useLegacyOrientation( bool state ) { useLegacyOrientation_ = state; };

	bool		sceneRootAdded( ) const { return sceneRootAdded_; };
	void		sceneRootAdded( bool state ) { sceneRootAdded_ = state; };

	void		visualTypeIdentifier( const std::string& s ) { visualTypeIdentifier_ = s; }
	const std::string& visualTypeIdentifier() const { return visualTypeIdentifier_; }
	
	int maxBones( ) const { return maxBones_; };
	void		maxBones( int value ) { maxBones_ = value; };

	static ExportSettings& ExportSettings::instance();

private:
	ExportSettings();
	//~ static BOOL CALLBACK dialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool			includeMeshes_;
	bool			includeEnvelopesAndBones_;
	bool			includeNodes_;
	bool			includeMaterials_;
	bool			includeAnimations_;
	bool			useCharacterMode_;
	bool			includePortals_;
	bool			worldSpaceOrigin_;
	bool			resolvePaths_;
	float			unitScale_;
	bool			localHierarchy_;
	bool			allowScale_;
	bool			bumpMapped_;
	bool			fixCylindrical_;	
	bool			keepExistingMaterials_;
	bool			snapVertices_;
	bool			stripRefPrefix_;
	bool			referenceNode_;
	bool			disableVisualChecker_;
	bool			useLegacyScaling_;
	bool			useLegacyOrientation_;
	bool			sceneRootAdded_;

	int				maxBones_;

	bool			exportAnim_;
	ExportMode		exportMode_;
	NodeFilter		nodeFilter_;

	std::string		animationName_;

	unsigned int	staticFrame_; //the static frame output
	unsigned int	frameFirst_;
	unsigned int	frameLast_;

	std::string		referenceNodesFile_;

	std::string		visualTypeIdentifier_;

	ExportSettings(const ExportSettings&);
	ExportSettings& operator=(const ExportSettings&);

	std::string getReferenceFile( HWND hWnd );
	std::string settingFilename_;
	friend std::ostream& operator<<(std::ostream&, const ExportSettings&);
};

#ifdef CODE_INLINE
#include "expsets.ipp"
#endif




#endif
/*expsets.hpp*/