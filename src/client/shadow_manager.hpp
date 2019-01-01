/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SHADOW_MANAGER_HPP
#define SHADOW_MANAGER_HPP

#include "moo/render_target.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/vectornodest.hpp"
#include "math/boundbox.hpp"
#include "resmgr/datasection.hpp"
#include "moo/effect_material.hpp"
#include "moo/visual.hpp"
#include "terrain/base_terrain_block.hpp"
#include "moo/graphics_settings.hpp"
#include "duplo/shadow_caster.hpp"

class Entity;
typedef SmartPointer<Entity> EntityPtr;

class ChunkEmbodiment;
typedef SmartPointer<ChunkEmbodiment> ChunkEmbodimentPtr;

namespace Moo
{
	class GraphicsSetting;
	class EffectManager::IListener;
}
typedef SmartPointer<Moo::GraphicsSetting> GraphicsSettingPtr;

/**
 *	This class manages the entities that can potentially cast shadows.
 *	The ShadowManager manages the shadow maps and the properties used by
 *	the shadows. When rendering, the ShadowManager picks the closest 
 *	entities to the camera and renders shadows for them. The number of
 *	shadows to render is configurable in the property "maxCount"
 *	in shadows.xml.
 *	
 */
class ShadowManager : public Moo::EffectManager::IListener
{
public:
	~ShadowManager();
	void init( DataSectionPtr pSection );
	void initCasters(int shadowCount);
	
	static ShadowManager& instance();

	void tick( float dTime );

	void captureShadows();
	void renderShadows( bool useTerrain );
	void renderFloraShadows( class Flora* flora );

	void addShadowEntity( Entity * pEntity );
	void delShadowEntity( Entity * pEntity );
	
	void setQualityOption(int optionIndex);
	void setCastersOption(int optionIndex);
	virtual void onSelectPSVersionCap(int psVerCap);

	void fini();
private:
	ShadowManager();
	
	Entity * getEntity(int index);
	Entity * getEntityOnHold(int index);
	
	bool		active_;
	uint32		maxShadowCount_;
	uint32		curShadowCount_;
	bool		halfRes_;

	std::vector< PyObjectPtr > pEntities_;
	std::vector< PyObjectPtr > pEntitiesOnHold_;

	std::vector< ShadowCasterPtr > pCasters_;
	std::vector< ChunkEmbodimentPtr > pEmbodiments_;
	uint32		nEmbodiments_;
	ShadowCasterCommon* pShadowCommon_;

	GraphicsSettingPtr qualitySettings_;
	GraphicsSettingPtr castersSettings_;
};


#endif // SHADOW_MANAGER_HPP
