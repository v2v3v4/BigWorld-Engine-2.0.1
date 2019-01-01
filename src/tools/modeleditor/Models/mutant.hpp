/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MUTANT_HPP
#define MUTANT_HPP

#include <set>
// This is temporary as mutant will be made into a pymodel for 1.10
#include "duplo/py_attachment.hpp"
#include "duplo/action_queue.hpp"

class SuperModel;

typedef SmartPointer< class AnimLoadCallback > AnimLoadCallbackPtr;
typedef SmartPointer< class SuperModelAnimation > SuperModelAnimationPtr;
typedef SmartPointer< class SuperModelAction > SuperModelActionPtr;
typedef SmartPointer< class SuperModelDye > SuperModelDyePtr;
typedef SmartPointer< class Fashion> FashionPtr;
typedef std::vector< FashionPtr > FashionVector;
typedef SmartPointer< class Model > ModelPtr;

typedef SmartPointer< class XMLSection > XMLSectionPtr;

typedef std::pair < std::string , std::string > StringPair;
typedef std::pair < StringPair , std::vector < std::string > > TreeBranch;
typedef std::vector < TreeBranch > TreeRoot;

typedef std::pair < StringPair , float > LODEntry;
typedef std::vector < LODEntry > LODList;

typedef std::map < std::string, std::string > Dyes;
typedef std::set< Moo::EffectMaterialPtr > EffectMaterialSet;

class MatrixLiaisonIdentity : public MatrixLiaison
{
public:
	virtual const Matrix & getMatrix() const { return Matrix::identity; }
	virtual bool setMatrix( const Matrix & m ) {};
};

struct ChannelsInfo : public ReferenceCount
{
	std::vector< Moo::AnimationChannelPtr > list;
};
typedef SmartPointer<ChannelsInfo> ChannelsInfoPtr;

class AnimationInfo
{
public:
	AnimationInfo();
	AnimationInfo(
		DataSectionPtr cData,
		DataSectionPtr cModel,
		SuperModelAnimationPtr cAnimation,
		std::map< std::string, float >& cBoneWeights,
		DataSectionPtr cFrameRates,
		ChannelsInfoPtr cChannels,
		float animTime
	);
	~AnimationInfo();

	Moo::AnimationPtr getAnim();
	Moo::AnimationPtr backupChannels();
	Moo::AnimationPtr restoreChannels();

	void uncompressAnim( Moo::AnimationPtr anim, std::vector<Moo::AnimationChannelPtr>& oldChannels );
	void restoreAnim( Moo::AnimationPtr anim, std::vector<Moo::AnimationChannelPtr>& oldChannels );

	DataSectionPtr data;
	DataSectionPtr model;
	SuperModelAnimationPtr animation;
	std::map< std::string, float > boneWeights;
	DataSectionPtr frameRates;
	ChannelsInfoPtr channels;
};

class ActionInfo
{
public:
	ActionInfo();
	ActionInfo(
		DataSectionPtr cData,
		DataSectionPtr cModel
	);

	DataSectionPtr data;
	DataSectionPtr model;
};

class MaterialInfo
{
public:
	MaterialInfo();
	MaterialInfo(
		const std::string& cName,
		DataSectionPtr cNameData,
		EffectMaterialSet cEffect,
		std::vector< DataSectionPtr > cData,
		std::string cFormat,
		bool cColours,
		bool cDualUV
	);

	std::string name;
	DataSectionPtr nameData;
	EffectMaterialSet effect;
	std::vector< DataSectionPtr > data;
	std::string format;
	bool colours;
	bool dualUV;
};

class TintInfo
{
public:
	TintInfo();
	TintInfo(
		Moo::EffectMaterialPtr cEffect,
		DataSectionPtr cData,
		SuperModelDyePtr cDye,
		std::string cFormat,
		bool cColours,
		bool cDualUV
	);

	Moo::EffectMaterialPtr effect;
	DataSectionPtr data;
	SuperModelDyePtr dye;
	std::string format;
	bool colours;
	bool dualUV;
};

class ModelChangeCallback : public ReferenceCount
{
public:
	ModelChangeCallback() {}

	virtual const bool execute() = 0;

	virtual const void* parent() const = 0;
};

template< class C >
class ModelChangeFunctor: public ModelChangeCallback
{
public:
	typedef bool (C::*Method)();
	
	ModelChangeFunctor( C* parent, Method method ):
		ModelChangeCallback(),
		parent_(parent),
		method_(method)
	{}

	const bool execute()
	{
		return (parent_->*method_)();
	}

	const void* parent() const
	{
		return parent_;
	}
private:
	C* parent_;
	Method method_;
};

class Mutant
{
public:
	Mutant( bool groundModel, bool centreModel );
	~Mutant();

	void registerModelChangeCallback( SmartPointer <ModelChangeCallback> mcc );
	void unregisterModelChangeCallback( void* parent );

	void groundModel( bool lock );
	void centreModel( bool lock );

	bool hasVisibilityBox( std::string modelPath = "" );
	int animCount( std::string modelPath = "" );
	bool nodefull( std::string modelPath = "" );

	size_t blendBoneCount() const;

	const std::string& modelName() { return modelName_; }

	void numNodes ( unsigned numNodes ) { numNodes_ = numNodes; }
	unsigned numNodes () { return numNodes_; }

	bool canBatch() const;
	void batched( bool val, bool undo = true );
	bool batched() const;

	void dpvsOccluder( bool val );
	bool dpvsOccluder();

	std::string editorProxyName();
	void editorProxyName( std::string editorProxyName );
	void removeEditorProxy();

	bool texMemUpdate()
	{
		bool val = texMemDirty_;
		texMemDirty_ = false;
		return val;
	}
	
	uint32 texMem() { return texMem_; }
		
	void bumpMutant( DataSectionPtr visual, const std::string& primitivesName );

	void revertModel();
	void reloadModel();
	bool loadModel( const std::string& name, bool reload = false );
	void postLoad();
	bool addModel( const std::string& name, bool reload = true );
	
	bool hasAddedModels();
	void removeAddedModels();

	void buildNodeList( DataSectionPtr data, std::set< std::string >& nodes );
	bool canAddModel( std::string modelPath );
	
	void reloadAllLists();

	void regenAnimLists();
	void regenMaterialsList();

	DataSectionPtr model( const std::string& modelPath = "" )
	{
		if (models_.find( modelPath ) == models_.end())
			return currModel_;

		return models_[modelPath];
	}
	DataSectionPtr visual() { return currVisual_; }

	void recreateFashions( bool dyesOnly = false );

	void recreateModelVisibilityBox( AnimLoadCallbackPtr pAnimLoadCallback, bool undoable );

	void updateAnimation( const StringPair& animID, float dTime );

	Vector3 getCurrentRootNode();
	BoundingBox zoomBoundingBox();
	BoundingBox modelBoundingBox();

	BoundingBox& transformBoundingBox( BoundingBox& inout );
	BoundingBox visibililtyBox();
	BoundingBox shadowVisibililtyBox();

	void updateFrameBoundingBox();
	void updateVisibilityBox();
	void updateActions(float dTime);
	bool visibilityBoxDirty() const { return visibilityBoxDirty_; }

	Matrix transform( bool grounded, bool centred = true );

	TreeRoot* animTree() { return &animList_; }
	TreeRoot* actTree() { return &actList_; }
	LODList*  lodList() { return &lodList_; }
	TreeRoot* materialTree() { return &materialList_; }
	
	void dirty( DataSectionPtr data );
	void forceClean();
	bool dirty();
	bool isReadOnly() const { return isReadOnly_; }

	void saveCorrectPrimitiveFile( bool isSaving, bool isForcedSave = false );
	void save();
	bool saveAs( const std::string& newName );

	int updateCount( const std::string& name )
	{
		return updateCount_[name];
	}

	void triggerUpdate( const std::string& name )
	{
		updateCount_[name]++;
	}

	void normalsLength( int length ) { normalsLength_ = length; }
	int normalsLength() { return normalsLength_; }
	
	// Render related functions

	float drawModel( bool atOrigin = false, float atDist = -1.f, bool drawIt = true );
	void drawOriginalModel();
	void drawBoundingBoxes();
	void drawSkeleton();
	void drawPortals();
	void reloadBSP();
	void drawBsp();
	void drawNormals( bool showNormals, bool showBinormals );
	void drawHardPoints();
	void calcCustomHull();
	void drawCustomHull();
	float render( float dTime, int renderStates );

	// Validation related functions
	
	void clipToDiffRoot( std::string& strA, std::string& strB );

	void fixTexAnim( const std::string& texRoot, const std::string& oldRoot, const std::string& newRoot );
	bool fixTextures( DataSectionPtr mat, const std::string& oldRoot, const std::string& newRoot );

	bool locateFile( std::string& fileName, std::string modelName, const std::string& ext,
		const std::string& what, const std::string& criticalMsg = "" );

	bool isFileReadOnly ( const std::string& file );
	bool testReadOnly( const std::string& modelName, const std::string& visualName, const std::string& primitivesName );

	bool ensureModelValid( const std::string& name, const std::string& what,
		DataSectionPtr* model = NULL, DataSectionPtr* visual = NULL,
		std::string* vName = NULL, std::string* pName = NULL,
		bool* readOnly = NULL);

	bool isFormatDepreciated( DataSectionPtr visual, const std::string& primitivesName );

	static void clearFilesMissingList();

	// Animation related functions
	
	bool canAnim( const std::string& modelPath );
	bool hasAnims( const std::string& modelPath );
		
	StringPair createAnim( const StringPair& animID, const std::string& animPath );
	void changeAnim( const StringPair& animID, const std::string& animPath );
	void removeAnim( const StringPair& animID );
	void cleanAnim( const StringPair& animID );

	Moo::AnimationPtr getMooAnim( const StringPair& animID );
	Moo::AnimationPtr restoreChannels( const StringPair& animID );
	Moo::AnimationPtr backupChannels( const StringPair& animID );
	
	void setAnim( size_t pageID, const StringPair& animID );
	const StringPair& getAnim( size_t pageID ) const;
	void stopAnim( size_t pageID );

	bool animName( const StringPair& animID, const std::string& animName );
	std::string animName( const StringPair& animID );

	void firstFrame( const StringPair& animID, int frame );
	int firstFrame( const StringPair& animID );

	void lastFrame( const StringPair& animID, int frame );
	int lastFrame( const StringPair& animID );

	void localFrameRate( const StringPair& animID, float rate, bool final = false );
	float localFrameRate( const StringPair& animID );
	void lastFrameRate( float rate ) { lastFrameRate_ = rate; }
	
	void saveFrameRate( const StringPair& animID );
	float frameRate( const StringPair& animID );

	void frameNum( const StringPair& animID, int num );
	int frameNum( const StringPair& animID );

	int numFrames( const StringPair& animID );

	void animFile( const StringPair& animID, const std::string& animFile );
	std::string animFile( const StringPair& animID );

	void animBoneWeight( const StringPair& animID, const std::string& boneName, float val );
	float animBoneWeight( const StringPair& animID, const std::string& boneName );

	void removeAnimNode( const StringPair& animID, const std::string& boneName );

	void playAnim( bool play );
	bool playAnim() { return playing_; }

	void loopAnim( bool loop) { looping_ = loop; }
	bool loopAnim() { return looping_; }

	// Action related functions

	bool hasActs( const std::string& modelPath );
	
	StringPair createAct( const StringPair& actID, const std::string& animName, const StringPair& afterAct );
	void removeAct( const StringPair& actID );

	void swapActions( const std::string& what, const StringPair& actID, const StringPair& act2ID, bool reload = true );
	
	void setAct ( const StringPair& actID );
	const StringPair& getAct() const	{	return currAct_;	}
	void stopAct();

	void setupActionMatch( SuperModelActionPtr action );
	bool alreadyLooping( const std::vector< SuperModelActionPtr >& actionsInUse, SuperModelActionPtr action );
	void takeOverOldAction( const std::vector< SuperModelActionPtr >& actionsInUse, SuperModelActionPtr action );

	std::string actName( const StringPair& actID );
	bool actName( const StringPair& actID, const std::string& actName );

	std::string actAnim( const StringPair& actID );
	void actAnim( const StringPair& actID, const std::string& animName );

	float actBlendTime( const StringPair& actID, const std::string& fieldName );
	void actBlendTime( const StringPair& actID, const std::string& fieldName, float val );

	bool actFlag( const StringPair& actID, const std::string& flagName );
	void actFlag( const StringPair& actID, const std::string& flagName, bool flagVal );

	int actTrack( const StringPair& actID );
	void actTrack( const StringPair& actID, int track );

	float actMatchFloat( const StringPair& actID, const std::string& typeName, const std::string& flagName, bool& valSet );
	bool actMatchVal( const StringPair& actID, const std::string& typeName, const std::string& flagName, bool empty, float val, bool& setUndo );
	
	std::string actMatchCaps( const StringPair& actID, const std::string& typeName, const std::string& capsType );
	void actMatchCaps( const StringPair& actID, const std::string& typeName, const std::string& capsType, const std::string& newCaps );

	void actMatchFlag( const StringPair& actID, const std::string& flagName, bool flagVal );

	// LOD functions
	
	float lodExtent( const std::string& modelFile );
	void lodExtent( const std::string& modelFile, float extent );

	void lodParents( std::string modelName, std::vector< std::string >& parents );
	bool hasParent( const std::string& modelName );

	bool isHidden( const std::string& modelFile );
		
	std::string lodParent( const std::string& modelFile );
	void lodParent( const std::string& modelFile, const std::string& parent );

	void lodList( LODList* newList );

	void virtualDist( float dist = -1.f );

	// Material functions

	std::string materialDisplayName( const std::string& materialName );
	
	void setDye( const std::string& matterName, const std::string& tintName, Moo::EffectMaterialPtr & material  );

	void getMaterial( const std::string& materialName, EffectMaterialSet & material );

	std::string getTintName( const std::string& matterName );

	bool setMaterialProperty( const std::string& materialName, const std::string& descName, const std::string& uiName, const std::string& propType, const std::string& val );
	
	void instantiateMFM( DataSectionPtr data );
	void overloadMFM( DataSectionPtr data, DataSectionPtr mfmData );

	void cleanMaterials();
	void cleanTints();
	void cleanMaterialNames();
		
	bool setTintProperty( const std::string& matterName, const std::string& tintName, const std::string& descName, const std::string& uiName, const std::string& propType, const std::string& val );

	bool materialName( const std::string& materialName, const std::string& new_name );

	bool matterName( const std::string& matterName, const std::string& new_name, bool undoRedo = true );

	bool tintName( const std::string& matterName, const std::string& tintName, const std::string& new_name, bool undoRedo = true );

	std::string newTint( const std::string& materialName, const std::string& matterName, const std::string& oldTintName, const std::string& newTintName, const std::string& fxFile, const std::string& mfmFile );
	
	bool saveMFM( const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& mfmFile );
	
	void deleteTint( const std::string& matterName, const std::string& tintName );
	
	bool ensureShaderCorrect( const std::string& fxFile, const std::string& format, bool dualUV );
		
	bool effectHasNormalMap( const std::string& effectFile );
	bool doAnyEffectsHaveNormalMap();
	
	bool isSkyBox() const { return isSkyBox_; }
	bool effectIsSkybox( const std::string& effectFile ) const;
	void checkMaterials();

    bool materialShader( const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& fxFile, bool undoable = true );
	std::string materialShader( const std::string& materialName, const std::string& matterName = "", const std::string& tintName = "" );

	bool materialMFM( const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& mfmFile, std::string* fxFile /* = NULL */ );

	void tintFlag( const std::string& matterName, const std::string& tintName, const std::string& flagName, uint32 val );
	uint32 tintFlag( const std::string& matterName, const std::string& tintName, const std::string& flagName );

	void materialFlag( const std::string& materialName, const std::string& flagName, uint32 val );
	uint32 materialFlag( const std::string& materialName, const std::string& flagName );

	void tintNames( const std::string& matterName, std::vector< std::string >& names );

	int modelMaterial () const;
	void modelMaterial ( int id );

	std::string materialPropertyVal( const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& propName, const std::string& dataType );
	std::string materialTextureFeedName( const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& propName );
	void changeMaterialFeed( DataSectionPtr data, const std::string& propName, const std::string& feedName );
	void materialTextureFeedName( const std::string& materialName, const std::string& matterName, const std::string& tintName, const std::string& propName, const std::string& feedName );

	std::string exposedToScriptName( const std::string& matterName, const std::string& tintName, const std::string& propName );
	void toggleExposed( const std::string& matterName, const std::string& tintName, const std::string& propName );

	Vector4 getExposedVector4( const std::string& matterName, const std::string& tintName, const std::string& descName, const std::string& propType, const std::string& val );

	uint32 recalcTextureMemUsage();

	Moo::EffectMaterialPtr getEffectForTint(	const std::string& matterName, const std::string& tintName, const std::string& materialName );

	void updateTintFromEffect( const std::string& matterName, const std::string& tintName );

	void edit( GeneralEditor& editor );
	void updateModificationInfo();
private:
	std::string modelName_;
	std::string visualName_;
	std::string primitivesName_;

	std::string editorProxyName_;

	DataSectionPtr extraModels_;

	Moo::EffectMaterialPtr pPortalMat_;

	unsigned numNodes_;
	
	bool texMemDirty_;
	uint32 texMem_;
	
	SuperModel* superModel_;
	SuperModel* editorProxySuperModel_;
	ActionQueue actionQueue_;
	MatrixLiaisonIdentity* matrixLI_;

	BoundingBox modelBB_;
	BoundingBox visibilityBB_;
	BoundingBox frameBB_;

	// length of the vertex normals
	int normalsLength_;

	// for drawing the custom hull
	std::vector< std::vector<Vector3> > customHullPoints_;

	// for drawing the bsp
	std::vector<Moo::VertexXYZL> verts_;

	DataSectionPtr currModel_;
	DataSectionPtr currVisual_;

	std::map < std::string , DataSectionPtr > modelHist_;

	std::map < std::string , DataSectionPtr > models_;
	std::map < DataSectionPtr , std::string > dataFiles_;

	std::map < StringPair , AnimationInfo > animations_;
	std::map < StringPair , ActionInfo > actions_;

	Dyes currDyes_;

	std::map< std::string, MaterialInfo > materials_;
	std::map< std::string, DataSectionPtr > dyes_;

	typedef std::map< std::string, std::map < std::string, TintInfo > > TintMap;
	TintMap tints_;

	std::map< std::string, std::string > tintedMaterials_;

	std::vector< DataSectionPtr > materialNameDataToRemove_;
	
	TreeRoot animList_;
	TreeRoot actList_;
	LODList  lodList_;
	TreeRoot materialList_;

	typedef std::map< size_t, StringPair >::iterator AnimIt;
		
	bool animMode_;
	bool clearAnim_;
	std::map< size_t, StringPair > currAnims_;
	StringPair currAct_;

	FashionVector fashions_;

	bool playing_;
	bool looping_;

	float lastFrameRate_;
	
	std::set< DataSectionPtr > dirty_;

	std::map< std::string, int > updateCount_;

	float virtualDist_;

	bool groundModel_;
	bool centreModel_;

	bool hasRootNode_;
	bool foundRootNode_;
	Vector3 initialRootNode_;

	bool visibilityBoxDirty_;

	typedef std::vector< SmartPointer < ModelChangeCallback > > ModelChangeCallbackVect;
	ModelChangeCallbackVect modelChangeCallbacks_;

	bool modelBeenBumped_;
	uint64 modifiedTime_;

	bool isReadOnly_;

	bool isSkyBox_;
	
	static std::vector< std::string > s_missingFiles_;

	uint32 materialSectionTextureMemUsage( DataSectionPtr data, std::set< std::string >& texturesDone );

	bool executeModelChangeCallbacks();

	MetaData::MetaData metaData_;
};

#endif // MUTANT_HPP