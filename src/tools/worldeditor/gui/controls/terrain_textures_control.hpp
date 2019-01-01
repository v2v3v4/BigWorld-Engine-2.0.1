/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TEXTURES_CONTROL_HPP
#define TERRAIN_TEXTURES_CONTROL_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "controls/dib_section32.hpp"


/**
 *  This class is used to show textures within a chunk in list form.
 */
class TerrainTexturesControl : public CWnd
{
public:
    TerrainTexturesControl();
    ~TerrainTexturesControl();

    BOOL Create(DWORD style, CRect const &extents, CWnd *parent, UINT id = 0);

    void terrain(EditorChunkTerrainPtr terrain);
	EditorChunkTerrainPtr terrain() const;
    
    Terrain::EditorBaseTerrainBlock* terrainBlock() const;

	void refresh();

    int height() const;

	int layerAtPoint( const CPoint& point ) const;
	bool layerRectAtPoint( const CPoint& point, CRect &rect ) const; 

	void resizeFullHeight( int width );

	std::vector<size_t> const &selection() const;
	void selection(std::vector<size_t> const &sel);

    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC *dc);

protected:
    typedef std::vector<controls::DibSection32Ptr>  DibSecVec;

    void rebuildTextureLayerEntries();

    void addLayer
    (
        Terrain::TerrainTextureLayer    const &layer,  
        std::vector<std::string>		const &oldTextureNames,
        DibSecVec						const &oldTextures
    );

    CRect textureExtent(size_t idx, CRect const *client = NULL) const;

    CRect blendExtent(size_t idx, CRect const *client = NULL) const;

	int selectionIdx(size_t idx) const;

	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point );
    DECLARE_MESSAGE_MAP()

private:
    EditorChunkTerrainPtr			terrain_;
    DibSecVec                       textures_;
    DibSecVec                       blends_;
    std::vector<std::string>        textureNames_;
	std::vector<size_t>				selection_;
};

#endif // TERRAIN_TEXTURES_CONTROL_HPP
