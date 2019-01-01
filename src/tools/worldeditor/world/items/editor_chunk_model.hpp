/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_MODEL_HPP
#define EDITOR_CHUNK_MODEL_HPP


#include "editor_chunk_bsp_holder.hpp"
#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "chunk/chunk_model.hpp"
#include "romp/static_light_fashion.hpp"
#include "moo/visual.hpp"
#include <set>
#include <map>
#include <string>

class EditorEffectProperty;

/**
 *	This class is the editor version of a ChunkModel
 */
class EditorChunkModel : public ChunkModel, EditorChunkBspHolder
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkModel )
	DECLARE_CHUNK_ITEM_ALIAS( EditorChunkModel, shell )

	static std::map<std::string, std::set<EditorChunkModel*> > editorChunkModels_;
	static void add( EditorChunkModel*, const std::string& resourceName );
	static void remove( EditorChunkModel* );

public:
	static void reload( const std::string& filename );
	void clean();

	EditorChunkModel();
	~EditorChunkModel();

	virtual bool edShouldDraw();
	virtual void draw();

	bool load( DataSectionPtr pSection, Chunk * pChunk );
	void loadModels( Chunk* chunk );
	/** Called once after loading from the main thread */
	void edPostLoad();

	virtual void toss( Chunk * pChunk );

	virtual bool edSave( DataSectionPtr pSection );
	virtual void edChunkSave();

	virtual const Matrix & edTransform() { return transform_; }
	virtual bool edTransform( const Matrix & m, bool transient );
	virtual void edBounds( BoundingBox& bbRet ) const;

	virtual bool edAffectShadow() const;

	virtual bool edEdit( class GeneralEditor & editor );
	virtual Chunk * edDropChunk( const Vector3 & lpos );

	virtual std::string edDescription();

	virtual int edNumTriangles() const;

	virtual int edNumPrimitives() const;

	virtual std::string edAssetName() const;
	virtual std::string edFilePath() const;

	virtual std::vector<std::string> edCommand( const std::string& path ) const;
	virtual bool edExecuteCommand( const std::string& path, std::vector<std::string>::size_type index );

	virtual DataSectionPtr pOwnSect()	{ return pOwnSect_; }
	virtual const DataSectionPtr pOwnSect()	const { return pOwnSect_; }

	virtual bool isShellModel();
	virtual const char* sectionName();

	/**
	 * Make sure we've got our own unique lighting data after being cloned
	 */
	void edPostClone( EditorChunkItem* srcItem );

	/** Ensure lighting on the chunk is marked as dirty */
	void edPostCreate();


	/** If we've got a .lighting file, delete it */
	void edPreDelete();

	/**
	 * Invalid the existing static lighting data if there is any
	 */
	void edInvalidateStaticLighting();

	/**
	 * Update the static lighting for the model
	 */
	bool edRecalculateLighting( StaticLighting::StaticLightContainer& lights );

	Vector3 edMovementDeltaSnaps();
	float edAngleSnaps();

	std::string getModelName();

	std::string getAnimation() const { return animName_; }
	bool setAnimation( const std::string & newAnimationName );

	std::string getDyeTints( const std::string& dye ) const;
	bool setDyeTints( const std::string& dye, const std::string& tint );

	float getAnimRateMultiplier() const { return animRateMultiplier_; }
	bool setAnimRateMultiplier( const float& f );

	bool getOutsideOnly() const { return outsideOnly_; }
	bool setOutsideOnly( const bool& outsideOnly );

	bool getCastsShadow() const { return castsShadow_; }
	bool setCastsShadow( const bool& castsShadow );

	virtual Moo::LightContainerPtr edVisualiseLightContainer();

	EditorEffectProperty* findMaterialPropertyByName( const std::string& name ) const;
	EditorEffectProperty* findOriginalMaterialPropertyByName( const std::string& name ) const;
	bool getMaterialBool( const std::string& name ) const;
	bool setMaterialBool( const std::string& name, const bool& value );
	std::string getMaterialString( const std::string& name ) const;
	bool setMaterialString( const std::string& name, const std::string& value );
	float getMaterialFloat( const std::string& name ) const;
	bool setMaterialFloat( const std::string& name, const float& value );
	bool getMaterialFloatRange( const std::string& name, float& min, float& max, int& digits );
	bool getMaterialFloatDefault( const std::string& name, float& def );
	void setMaterialFloatToDefault( const std::string& name );
	Vector4 getMaterialVector4( const std::string& name ) const;
	bool setMaterialVector4( const std::string& name, const Vector4& value );
	Matrix getMaterialMatrix( const std::string& name ) const;
	bool setMaterialMatrix( const std::string& name, const Matrix& value );
	int getMaterialInt( const std::string& name ) const;
	bool setMaterialInt( const std::string& name, const int& value );
	bool getMaterialIntRange( const std::string& name, int& min, int& max, int& digits );

	Moo::EffectMaterialPtr findMaterialByName( const std::string& name ) const;
	std::string getMaterialCollision( const std::string& name ) const;
	bool setMaterialCollision( const std::string& name, const std::string& collisionType );
	std::string getMaterialKind( const std::string& name ) const;
	bool setMaterialKind( const std::string& name, const std::string& collisionType );
	std::string getMaterialEnum( const std::string& name ) const;
	bool setMaterialEnum( const std::string& name, const std::string& enumValue );

	void changedMaterial( const std::string& materialPropertyName );
	
protected:
	virtual void addStaticLighting( DataSectionPtr ds, Chunk* pChunk );

private:
	EditorChunkModel( const EditorChunkModel& );
	EditorChunkModel& operator=( const EditorChunkModel& );

	bool			resourceIsOutsideOnly() const;

	bool								hasPostLoaded_;
	DataSectionPtr						pOwnSect_;
	std::string							animName_;
	std::map<std::string,std::string>	tintName_;
	std::set<std::string>				changedMaterials_;
	
	StringHashMap<int>	  				collisionFlags_;
	std::vector<std::string>			collisionFlagNames_;
	static StringHashMap<int>			s_materialKinds_;

	uint								primGroupCount_;
	bool								customBsp_;		// if the user specified their own bsp
	bool								outsideOnly_;
	bool								castsShadow_;

	class SuperModel*					pEditorModel_;

	/**
	 * Load the visual from the data section, for creating initial
	 * static lighting
	 */
	std::vector<Moo::VisualPtr> extractVisuals();

	/**
	 * As extractVisuals(), but returns the names of the visuals rather than
	 * a Ptr to them.
	 */
	std::vector<std::string> extractVisualNames() const;

	/**
	 * Calculate lighting infomation, and store it in values
	 *
	 * returns false if we were interupted while recalculating
	 */
	bool calculateLighting( StaticLighting::StaticLightContainer& lights,
		Moo::VisualPtr visual, std::vector<size_t>& sizes );

	/**
	 * Remove the static lighting fastion from the base model, and set it to NULL
	 */
	void clearLightingFashion();

	/** The fashion with the static lighting info */
	StaticLightFashionPtr pStaticLightFashion_;

	bool isModelNodeless()	{ return isModelNodeless_; }
	bool isModelNodeless_;
	void detectModelType();

	// check if the model file is a new date then the binary data file
	bool isVisualFileNewer() const;
	bool firstToss_;

	// for edDescription
	std::string desc_;

	// is a model is not found, the standin is loaded
	bool standinModel_;
	DataSectionPtr originalSect_;

	// animations available to be played in this model
	std::vector<std::string> animationNames_;
	std::vector<std::wstring> animationNamesW_;

	// for bsp index
	std::string bspModelName_;

	// dye and tint available to be played in this model
	// map dye name to tints names vector
	std::map<std::string, std::vector<std::string> > dyeTints_;
	std::map<std::string, std::vector<std::wstring> > dyeTintsW_;

	void edit( Moo::EffectMaterialPtr material, GeneralEditor & editor );
};


typedef SmartPointer<EditorChunkModel> EditorChunkModelPtr;


#endif // EDITOR_CHUNK_MODEL_HPP
