/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_SUBSTANCE
#define EDITOR_CHUNK_SUBSTANCE

#include "math/boundbox.hpp"
#include "worldeditor/world/items/editor_chunk_substance.hpp"
#include "worldeditor/project/project_module.hpp"
#include "worldeditor/misc/options_helper.hpp"
#include "chunk/chunk_model_obstacle.hpp"
#include "chunk/chunk_model.hpp"
#include "appmgr/module_manager.hpp"
#include "appmgr/options.hpp"
#include "romp/fog_controller.hpp"

class   Chunk;
class   Model;
typedef SmartPointer<Model> ModelPtr;

/**
 *	This template class gives substance in the editor to items that
 *	are ordinarily without it.
 */
template <class CI> class EditorChunkSubstance : public CI
{
public:
	EditorChunkSubstance() { }
	~EditorChunkSubstance() { }

	bool load( DataSectionPtr pSection );
	bool load( DataSectionPtr pSection, Chunk * pChunk );

	virtual void toss( Chunk * pChunk );

	virtual bool edShouldDraw();
	virtual void draw();

	virtual void edBounds( BoundingBox& bbRet ) const;

	virtual const char * sectName() const = 0;
	virtual const char * drawFlag() const = 0;
	virtual ModelPtr reprModel() const = 0;

	virtual bool autoAddToSceneBrowser() const { return true; }

	virtual void addAsObstacle();	// usually don't override

	virtual DataSectionPtr pOwnSect()	{ return pOwnSect_; }
	virtual const DataSectionPtr pOwnSect()	const { return pOwnSect_; }

	bool reload();

	virtual bool addYBounds( BoundingBox& bb ) const;

protected:
	DataSectionPtr	pOwnSect_;

	static uint32	s_settingsMark_;
	static bool		s_drawAlways_;
};




#endif // EDITOR_CHUNK_SUBSTANCE