/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_ENTITY_HPP
#define EDITOR_CHUNK_ENTITY_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_substance.hpp"
#include "chunk/chunk_item.hpp"
#include "entitydef/entity_description_map.hpp"
#include "model/super_model.hpp"
#include "gizmo/general_editor.hpp"
#include "common/properties_helper.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/unique_id.hpp"


/**
 *	This class is the editor version of an entity in a chunk.
 *
 *	Note that it does not derive from the client's version of a chunk entity
 *	(because the editor does not have the machinery necessary to implement it)
 */
class EditorChunkEntity : public EditorChunkSubstance<ChunkItem>, public Aligned
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkEntity )

public:
	// Constructor / destructor
	EditorChunkEntity();
	~EditorChunkEntity();

	// Chunk item methods
	virtual void	edMainThreadLoad();
	virtual bool	edTransform( const Matrix & m, bool transient );
	bool			edLoad(	const std::string type, DataSectionPtr pSection,
							std::string* errorString = NULL );
	bool			edLoad( DataSectionPtr pSection, bool loadTransform = true );
	virtual bool	edSave( DataSectionPtr pSection );
	virtual const Matrix&
					edTransform()	{ return transform_; }
	virtual std::string
					edDescription();
	virtual bool	edAffectShadow() const {	return false; }
	virtual bool	edEdit( class GeneralEditor & editor );
	bool			edEditProperties( class GeneralEditor & editor, MatrixProxy * pMP );
	void			edPreDelete();
	virtual bool	edCanDelete();
	virtual std::vector<std::string> edCommand( const std::string& path ) const;
	virtual bool	edExecuteCommand( const std::string& path, std::vector<std::string>::size_type index );
	virtual bool	edShouldDraw();

	virtual Moo::LightContainerPtr edVisualiseLightContainer();

	virtual std::string edAssetName() const { return pOwnSect_->readString( "type" ); }
	virtual std::string edFilePath() const;

	bool load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString = NULL );
    virtual void toss( Chunk * pChunk );
	virtual void draw();
    virtual void tick(float dtime);

	GeneralProperty* parseProperty(
		int i, PyObject* pyEnum, DataDescription* pDD, MatrixProxy * pMP );

	bool		clientOnlyGet() const			{ return clientOnly_; }
	bool		clientOnlySet( const bool & b );
	std::string	typeGet() const;
	bool		typeSet( const std::string & s )	{ return false; }
	std::string idGet() const;
	bool		idSet( const std::string & s )	{ return false; }
	std::string lseGet() const				{ return lastScriptError_; }
	bool		lseSet( const std::string & s )	{ return false; }
	ModelPtr	getReprModel() const { return reprModel(); }
	const EntityDescription*
				getTypeDesc() { return pType_; }
	void		setSurrogateMarker( DataSectionPtr pMarkerSect )
		{ surrogateMarker_ = true; pOwnSect_ = pMarkerSect; }

	typedef std::pair< PyObject *, std::string > BindingProperty;
	typedef std::vector< BindingProperty > BindingProperties;
	const BindingProperties & getBindingProps() { return bindingProps_; }

	virtual bool isEditorEntity() const { return true; }

	// Tell substance not to add us automatically, we'll do it ourselves.
	virtual bool autoAddToSceneBrowser() const { return false; }

	// Patrol methods
	void patrolListNode(std::string const &id);
    std::string patrolListNode() const;
    std::string patrolListGraphId() const;
    int patrolListPropIdx() const;
	bool patrolListRelink( const std::string& newGraph, const std::string& newNode );
    void disconnectFromPatrolList();

	/** Set the correct model for all entities that have changed recently */
	static void calculateDirtyModels();
	bool isDefaultModel() const;

	UniqueID guid() const{ return guid_; };
	void guid( UniqueID newGUID );

	PyObject* infoDict() const;
	bool canLinkTo( const std::string& propName, PyObject* otherInfo ) const;

	// Linker accessor
	EditorChunkItemLinkable* chunkItemLinker() const { return pChunkItemLinker_; }
	
	// Properties methods
	void clearProperties();
	void clearEditProps();
	void setEditProps( const std::list< std::string > & names );
	void clearPropertySection();
	PropertiesHelper* propHelper() { return &propHelper_; }
	bool firstLinkFound() const { return firstLinkFound_; }
	void firstLinkFound( bool value ) { firstLinkFound_ = value; }
	virtual void syncInit();

private:
	EditorChunkEntity( const EditorChunkEntity& );
	EditorChunkEntity& operator=( const EditorChunkEntity& );

	void propertyChangedCallback( int index );

	void recordBindingProps();
	void removeFromDirtyList();
	void markModelDirty();
	void calculateModel();

	virtual const char * sectName() const { return "entity"; }
	virtual const char * drawFlag() const { return "render/drawEntities"; }
	virtual ModelPtr reprModel() const;

	EditorChunkStationNode* connectedNode();

	std::string nodeId() const                   { return patrolListNode(); }
	bool nodeId( const std::string & /*s*/ )     { return false; }
	std::string graphId() const                  { return patrolListGraphId(); }
	bool graphId( const std::string & /*s*/ )    { return false; }
	bool initType( const std::string type, std::string* errorString );

	static void loadModelTask( void* self );
	static void loadModelTaskDone( void* self );

	bool loading() const;
	void waitDoneLoading();

	bool									clientOnly_;
	DataSectionPtr							pOriginalSect_;
	bool									surrogateMarker_;
    std::string								patrolListNode_;

	UniqueID								guid_;
	const EntityDescription *				pType_;
	Matrix									transform_;
	bool									transformLoaded_;
	std::string								lastScriptError_;
	PyObject *								pDict_;
	std::vector<bool>						allowEdit_;
	ModelPtr								model_;
	PyObject *								pyClass_;
    EditorChunkLinkPtr						link_;
	BindingProperties						bindingProps_;

	// Linker object
	EditorChunkItemLinkable*					pChunkItemLinker_;
	
	PropertiesHelper						propHelper_;
	bool									firstLinkFound_;

	BackgroundTaskPtr						loadBackgroundTask_;
	std::string								modelToLoad_;
	ModelPtr								loadingModel_;

	static SimpleMutex						s_dirtyModelMutex_;
	static SimpleMutex						s_loadingModelMutex_;
	static std::vector<EditorChunkEntity*>	s_dirtyModelEntities_;
};

typedef SmartPointer<EditorChunkEntity> EditorChunkEntityPtr;



// -----------------------------------------------------------------------------
// Section: EditorEntityType
// -----------------------------------------------------------------------------


class EditorEntityType
{
public:
	static void startup();
	static void shutdown();

	EditorEntityType();

	const EntityDescription * get( const std::string & name );
	PyObject * getPyClass( const std::string & name );

	static EditorEntityType & instance();
	bool isEntity( const std::string& name ) const	{	return eMap_.isEntity( name );	}
private:
	EntityDescriptionMap	eMap_;
	std::map<std::string,EntityDescription*> mMap_;

	std::map<std::string, PyObject *> pyClasses_;

	static EditorEntityType * s_instance_;
};


#endif // EDITOR_CHUNK_ENTITY_HPP
