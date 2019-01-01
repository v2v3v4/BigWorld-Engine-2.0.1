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
#include "worldeditor/editor/user_data_object_link_locator.hpp"
#include "worldeditor/editor/chunk_obstacle_locator.hpp"
#include "worldeditor/collisions/closest_obstacle_no_edit_stations.hpp"
#include "worldeditor/world/items/editor_chunk_user_data_object.hpp"
#include "worldeditor/world/items/editor_chunk_entity.hpp"
#include "chunk/chunk_item.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "gizmo/snap_provider.hpp"
#include <limits>
#include <string>


namespace
{
    /**
     *  This is a helper class that finds the closest linkable chunk item.
     */
    class ClosestLinkableCatcher : public CollisionCallback
    {
    public:
		explicit ClosestLinkableCatcher(const std::string& linkName, UserDataObjectLinkLocator::Type type ) : 
            chunkItem_(NULL),
            distance_(std::numeric_limits<float>::max()),
            type_(type),
			linkName_(linkName)
        {
        }

        /*virtual*/ int operator()
        ( 
            ChunkObstacle   const &obstacle,
		    WorldTriangle   const &/*triangle*/, 
            float           dist
        )
        {
			BW_GUARD;

            if (dist < distance_)
            {
                distance_ = dist;

                EditorChunkItem *item = 
                    (ChunkItem *)obstacle.pItem().getObject();
                
                if 
                (
                    (type_ & UserDataObjectLinkLocator::LOCATE_USER_DATA_OBJECTS) != 0
                    &&
                    item->isEditorUserDataObject()
                )
                {
                    EditorChunkUserDataObject *cci = (EditorChunkUserDataObject *)item;
                    chunkItem_ = obstacle.pItem();
                }
                else if 
                (
				   (type_ & UserDataObjectLinkLocator::LOCATE_ENTITIES) != 0
                    &&
                    item->isEditorEntity()
                )
                {
                    EditorChunkEntity *entity = (EditorChunkEntity *)item;
                    int idx = entity->patrolListPropIdx();
                    if (idx != -1)
                    {
                        chunkItem_ = obstacle.pItem();
                    }
				}
                    
            }
            return COLLIDE_BEFORE;
        }

        ChunkItemPtr                    chunkItem_;
        float                           distance_;
        UserDataObjectLinkLocator::Type    type_;
		std::string							linkName_;
    };
}


/**
 *  StationNodeLinkLocator constructor.
 */
/*explicit*/ UserDataObjectLinkLocator::UserDataObjectLinkLocator
(	
	const std::string& linkName,
    Type    type       /*= LOCATE_BOTH*/
) :
    chunkItem_(NULL),
    initialPos_(true),
    type_(type),
	linkName_(linkName)

{
	BW_GUARD;

    subLocator_ = ToolLocatorPtr(
        new ChunkObstacleToolLocator(ClosestObstacleNoEditStations::s_default),
		PyObjectPtr::STEAL_REFERENCE );
}


/**
 *  StationNodeLinkLocator destrcutor.
 */
/*virtual*/ UserDataObjectLinkLocator::~UserDataObjectLinkLocator()
{
}


/**
 *  Calculate the location given a ray through worldray.
 *
 *  @param worldRay     The world ray.
 *  @param tool         The originating tool.
 */
/*virtual*/ void 
UserDataObjectLinkLocator::calculatePosition
(
    Vector3             const &worldRay, 
    Tool                &tool
)
{
	BW_GUARD;

    chunkItem_ = NULL; // reset item

    // Allow the sub-locator to find the position first:
	subLocator_->calculatePosition(worldRay, tool);
	transform_ = subLocator_->transform();

	Vector3 extent = 
        Moo::rc().invView().applyToOrigin() 
        +
		worldRay*Moo::rc().camera().farPlane();

    ClosestLinkableCatcher clc(linkName_, type_);

	float distance = 
        ChunkManager::instance().cameraSpace()->collide
        ( 
		    Moo::rc().invView().applyToOrigin(),
		    extent,
		    clc
        );

    chunkItem_ = clc.chunkItem_;
}


/**
 *  Return the selected chunk item.
 */
ChunkItemPtr UserDataObjectLinkLocator::chunkItem()
{
    return chunkItem_;
}
