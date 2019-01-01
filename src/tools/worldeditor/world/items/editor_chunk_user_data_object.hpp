/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_USER_DATA_OBJECT_HPP
#define EDITOR_CHUNK_USER_DATA_OBJECT_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_substance.hpp"
#include "chunk/chunk_item.hpp"
#include "entitydef/user_data_object_description_map.hpp"
#include "common/properties_helper.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "gizmo/general_editor.hpp"
#include "model/super_model.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "cstdmf/unique_id.hpp"
#include "cstdmf/concurrency.hpp"


/**
 *	This class is the editor version of a user data object in a chunk.
 */
class EditorChunkUserDataObject : public EditorChunkSubstance<ChunkItem>, public Aligned
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkUserDataObject )

public:
	// Constructor / destructor
	EditorChunkUserDataObject();
	~EditorChunkUserDataObject();

	bool edLoad( const std::string type, DataSectionPtr pSection, std::string* errorString = NULL );
	bool edLoad( DataSectionPtr pSection, bool loadTransform = true );
	virtual const Matrix & edTransform()	{ return transform_; }
	virtual bool edTransform( const Matrix & m, bool transient );
	virtual bool edSave( DataSectionPtr pSection );
	virtual bool edIsSnappable() { return false; }
	virtual void edMainThreadLoad();
	virtual std::string edDescription();
	virtual bool edAffectShadow() const {	return false; }
	virtual bool edEdit( class GeneralEditor & editor );
	virtual bool edCanDelete();
	virtual void edPreDelete();
	virtual std::vector<std::string> edCommand( const std::string& path ) const;
	virtual bool edExecuteCommand( const std::string& path, std::vector<std::string>::size_type index );
	virtual bool edShouldDraw();
	bool edEditProperties( class GeneralEditor & editor, MatrixProxy * pMP );
	Vector3 edMovementDeltaSnaps();
	float edAngleSnaps();
	virtual void edPostClone( EditorChunkItem* srcItem );

	virtual std::string edAssetName() const { return pOwnSect_->readString( "type" ); }
	virtual std::string edFilePath() const;

	bool load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString = NULL );
    virtual void toss( Chunk * pChunk );
	virtual void draw();
    virtual void tick(float dtime);

	std::string typeGet() const;
	bool typeSet( const std::string & s )	{ return false; }
	std::string domainGet() const;
	bool domainSet( const std::string & s )	{ return false; }
	std::string idGet() const;
	bool idSet( const std::string & s )	{ return false; }
	std::string lseGet() const				{ return lastScriptError_; }
	bool lseSet( const std::string & s )	{ return false; }

	// Tell substance not to add us automatically, we'll do it ourselves.
	virtual bool autoAddToSceneBrowser() const { return false; }

	// Properties methods
	void clearProperties();
	void clearEditProps();
	void setEditProps( const std::list< std::string > & names );
	void clearPropertySection();
	PropertiesHelper* propHelper() { return &propHelper_; }
	bool firstLinkFound() const { return firstLinkFound_; }
	void firstLinkFound( bool value ) { firstLinkFound_ = value; }

	// Linker accessor
	EditorChunkItemLinkable* chunkItemLinker() const { return pChunkItemLinker_; }
	
	ModelPtr getReprModel() const { return reprModel(); }
	bool isDefaultModel() const;
	const UserDataObjectDescription * getTypeDesc() { return pType_; }

	typedef std::pair< PyObject *, std::string > BindingProperty;
	typedef std::vector< BindingProperty > BindingProperties;
	const BindingProperties & getBindingProps() { return bindingProps_; }

    virtual bool isEditorUserDataObject() const { return true; };
	UniqueID guid() const{ return guid_; };
	void guid( UniqueID newGUID );

	static void calculateDirtyModels();

	PyObject* infoDict() const;
	bool canLinkTo( const std::string& propName, const EditorChunkUserDataObject* other ) const;
	bool showAddGizmo( const std::string& propName ) const;
	void onDelete() const;
	void getLinkCommands( std::vector<std::string>& commands, const EditorChunkUserDataObject* other ) const;
	void executeLinkCommand( int cmdIndex, const EditorChunkUserDataObject* other ) const;
	
	virtual void syncInit();
	
private:
	EditorChunkUserDataObject( const EditorChunkUserDataObject& );
	EditorChunkUserDataObject& operator=( const EditorChunkUserDataObject& );

	virtual const char * sectName() const { return "UserDataObject"; }
	virtual const char * drawFlag() const { return "render/drawEntities"; }
	virtual ModelPtr reprModel() const;

	void propertyChangedCallback( int index );

	void recordBindingProps();
	EditorChunkStationNode* connectedNode();

	void markModelDirty();
	void removeFromDirtyList();
	void calculateModel();

	bool initType( const std::string type, std::string* errorString );

	static void loadModelTask( void* self );
	static void loadModelTaskDone( void* self );

	// Linker object
	EditorChunkItemLinkable*				pChunkItemLinker_;

	bool loading() const;
	void waitDoneLoading();

	UniqueID							guid_;
	const UserDataObjectDescription*	pType_;
	Matrix								transform_;
	bool								transformLoaded_;
	std::string							lastScriptError_;
	PyObject *							pDict_;
	std::vector<bool>					allowEdit_;
	ModelPtr							model_;
	PyObject *							pyClass_;
	EditorChunkLinkPtr					link_;
	BindingProperties					bindingProps_;

	PropertiesHelper					propHelper_;
	bool								firstLinkFound_;

	BackgroundTaskPtr					loadBackgroundTask_;
	std::string							modelToLoad_;
	ModelPtr							loadingModel_;

	static SimpleMutex								s_dirtyModelMutex_;
	static SimpleMutex								s_loadingModelMutex_;
	static std::vector<EditorChunkUserDataObject*>	s_dirtyModelEntities_;
};


typedef SmartPointer<EditorChunkUserDataObject> EditorChunkUserDataObjectPtr;



// -----------------------------------------------------------------------------
// Section: EditorUserDataObjectType
// -----------------------------------------------------------------------------


class EditorUserDataObjectType
{
public:
	static void startup();
	static void shutdown();

	EditorUserDataObjectType();

	const UserDataObjectDescription * get( const std::string & name );
	PyObject * getPyClass( const std::string & name );

	static EditorUserDataObjectType & instance();
	bool isUserDataObject( const std::string& name ) const	{	return cMap_.isUserDataObject( name );	}
private:
	UserDataObjectDescriptionMap	cMap_;
	std::map<std::string, PyObject *> pyClasses_;

	static EditorUserDataObjectType * s_instance_;
};


#endif // EDITOR_CHUNK_USER_DATA_OBJECT_HPP
