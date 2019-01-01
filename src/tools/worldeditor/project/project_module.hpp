/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROJECT_MODULE_HPP
#define PROJECT_MODULE_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/project/grid_coord.hpp"
#include "worldeditor/project/lock_map.hpp"
#include "worldeditor/project/space_map.hpp"
#include "worldeditor/project/world_editord_connection.hpp"
#include "appmgr/module.hpp"
#include "ashes/simple_gui.hpp"
#include "ashes/text_gui_component.hpp"
#include "romp/font.hpp"


class ProjectModule : public FrameworkModule
{
public:
	ProjectModule();
	~ProjectModule();

	virtual bool init( DataSectionPtr pSection );

	virtual void onStart();
	virtual int  onStop();

	virtual void onPause();
	virtual void onResume( int exitCode );

	virtual bool updateState( float dTime );
	virtual void render( float dTime );

	virtual bool handleKeyEvent( const KeyEvent & event );
	virtual bool handleMouseEvent( const MouseEvent & event );

	bool isReadyToLock() const;
	bool isReadyToCommitOrDiscard() const;

	// the locked chunks under selection
	std::set<std::string> lockedChunksFromSelection( bool insideOnly );

	// the graph file under selection
	std::set<std::string> graphFilesFromSelection();

	// the vlo file under selection
	std::set<std::string> vloFilesFromSelection();

	void getChunksFromRectRecursive( const std::string& spaceDir, const std::string& subDir,
							std::set<std::string>& chunks, BWLock::Rect rect, bool insideOnly );
							
	void getChunksFromRect( std::set<std::string>& chunks, BWLock::Rect rect, bool insideOnly );

	/** Lock the current selection, if any, returns if the lock was successful */
	bool lockSelection( const std::string& description );

	/** Discard any locks we have, returning if we were able */
	bool discardLocks( const std::string& description );

	/** Inform bigbangd we just commited the space */
	void commitDone();

	/** Get the updated lock data from bigbangd, and update the lockTexture */
	void updateLockData();

	/** Set blend value for space map **/
	void projectMapAlpha( float a );
	float projectMapAlpha();

    /** Draw all dirty chunks **/
    static void regenerateAllDirty();

	/** Get the current instance on the module stack, if any */
	static ProjectModule* currentInstance();
	static std::string currentSpaceDir();
	static std::string currentSpaceResDir();
	static std::string currentSpaceAbsoluteDir();
private:
	ProjectModule( const ProjectModule& );
	ProjectModule& operator=( const ProjectModule& );

	void writeStatus();

	LockMap		lockMap_;
	float		spaceAlpha_;
	uint32		spaceColour_;
	Moo::BaseTexturePtr handDrawnMap_;

	/** Camera position, X & Y specify position, Z specifies zoom */
	Vector3 viewPosition_;
	float	minViewZ_;
	//we must allow at least a far plane of minViewZ when viewing the bitmap.
	//this allows us to return bigbang to its previous state.
	float	savedFarPlane_;	

	/** Extent of the grid, in number of chunks. It starts at 0,0 */
	unsigned int gridWidth_;
	unsigned int gridHeight_;
	int minX_;
	int minY_;

	/**
	 * Add to a GridCoord to transform it from a local (origin at 0,0) to a
	 * world (origin in middle of map) grid coord
	 */
	GridCoord localToWorld_;

	/**
	 * Where the cursor was when we started looking around,
	 * so we can restore it to here when done
	 */
	POINT lastCursorPosition_;

	/** The start of the current selection */
	Vector2 selectionStart_;

	/** The currently selecting, with the mouse, rect */
	GridRect currentSelection_;
	GridCoord currentSelectedCoord_;

	SmartPointer<ClosedCaptions> cc_;

	/** Where in the grid the mouse is currently pointing */
	Vector2 currentGridPos();

	/** Where in the grid for a certain pixel position */
	Vector2 pixelToGridPos( int x, int y );

	/** Get a world position from a grid position */
	Vector3 gridPosToWorldPos( Vector2 gridPos );

    /** Get a 2d world position from a screen position. */
    Vector2 gridPos(POINT pt) const;

	/** The last created ProjectModule */
	static ProjectModule* currentInstance_;

	// true iff the lmb was depressed over the 3D window
	bool mouseDownOn3DWindow_;

	TextGUIComponent*	text_;
	TextGUIComponent*	shadow_;

	FontPtr				font_;
	FontPtr				boldFont_;
};


#endif // PROJECT_MODULE_HPP
