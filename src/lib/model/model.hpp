/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef _MSC_VER 
#pragma once
#endif

#ifndef MODEL_HPP
#define MODEL_HPP

#include "forward_declarations.hpp"
#include "math/forward_declarations.hpp"
#include "moo/forward_declarations.hpp"
#include "physics2/forward_declarations.hpp"
#include "resmgr/forward_declarations.hpp"
#include "romp/static_light_values.hpp"
#include "moo/visual.hpp"

#include "dye_property.hpp"
#include "dye_selection.hpp"
#include "material_override.hpp"
#include "matter.hpp"
#include "model_action.hpp"
#include "model_dye.hpp"
#include "node_tree.hpp"


#ifdef EDITOR_ENABLED
#include "gizmo/meta_data.hpp"
#include "gizmo/meta_data_helper.hpp"
#endif

typedef std::vector< Moo::NodePtr > MooNodeChain;


/**
 *	Helper class for sorting pointers to strings by their string value
 */
class StringOrderProxy
{
public:
	StringOrderProxy( const std::string * pString ) : pString_( pString ) { }

	bool operator <( const StringOrderProxy & other ) const
		{ return (*pString_) < (*other.pString_); }
	bool operator ==( const StringOrderProxy & other ) const
		{ return (*pString_) == (*other.pString_); }
private:
	const std::string * pString_;
};



/**
 * TODO: to be documented.
 */
class AnimLoadCallback : public ReferenceCount
{
public:
	AnimLoadCallback() {}

	virtual void execute() = 0;
};



/**
 *	This class manages a model. There is only one instance
 *	of this class for each model file, as it represents only
 *	the static data of a model. Dynamic data is stored in the
 *	referencing SuperModel file.
 */
class Model : public SafeReferenceCount
{
	// TODO: remove use of friend
	friend ModelAction;
	friend ModelActionsIterator;
	friend ModelMap;
	friend class ModelPLCB;
public:

	virtual ~Model();
	void addToMap();

	static void clearReportedErrors();
	static ModelPtr get( const std::string & resourceID,
		bool loadIfMissing = true );
	ModelPtr reload( DataSectionPtr pFile = NULL, bool reloadChildren = true ) const;



#ifdef EDITOR_ENABLED
	static void setAnimLoadCallback( SmartPointer< AnimLoadCallback > pAnimLoadCallback ) { s_pAnimLoadCallback_ = pAnimLoadCallback; }
	MetaData::MetaData& metaData() { return metaData_; }
	virtual void reload() = 0;
#endif

	virtual bool valid() const;
	virtual bool occluder() const { return false; }

	static const int blendCookie();
	static int getUnusedBlendCookie();
	static int incrementBlendCookie();

	void soakDyes();
	virtual void dress() = 0;
	virtual void draw( bool checkBB ) = 0;

	virtual Moo::VisualPtr getVisual() { return NULL; }

	virtual const BSPTree * decompose() const		{ return NULL; }

	virtual const BoundingBox & boundingBox() const = 0;
	virtual const BoundingBox & visibilityBox() const = 0;
	virtual bool hasNode( Moo::Node * pNode,
		MooNodeChain * pParentChain ) const		{ return false; }

	const std::string & resourceID() const		{ return resourceID_; }

	// intentionally not returning a smart pointer here 'coz we're
	//  the only ones that should be keeping references to our parent
	//  (when not keeping references to us)
	Model * parent()							{ return parent_.getObject(); }
	float extent() const						{ return extent_; }

#ifdef NOT_MULTITHREADED
	static NodeCatalogue & nodeCatalogue()		{ return s_nodeCatalogue_; }
#endif

	int getAnimation( const std::string & name ) const;
	void tickAnimation( int index, float dtime, float otime, float ntime );
	void playAnimation( int index, float time, float blendRatio, int flags );



	// Don't try to store the pointer this returns, unless you guarantee
	//  the model will be around.
	ModelAnimation * lookupLocalAnimation( int index );
	ModelAnimation * lookupLocalDefaultAnimation()
		{ return pDefaultAnimation_.getObject(); }



	const ModelAction * getAction( const std::string & name ) const;

	ModelActionsIterator lookupLocalActionsBegin() const;
	ModelActionsIterator lookupLocalActionsEnd() const;

	typedef StringMap< Vector4 > PropCatalogue;
	static int getPropInCatalogue( const char * name );
	static const char * lookupPropName( int index );

	// use with care... lock all accesses to the catalogue
	static PropCatalogue & propCatalogueRaw()	{ return s_propCatalogue_; }
	static SimpleMutex & propCatalogueLock()	{ return s_propCatalogueLock_; }


	typedef std::vector< DyeSelection > DyeSelections;

	ModelDye getDye(	const std::string & matterName,
						const std::string & tintName,
						Tint ** ppTint = NULL );

	virtual void soak( const ModelDye & dye );

	const Matter * lookupLocalMatter( int index ) const;


	typedef StringMap< MaterialOverride >		MaterialOverrides;

public:
	virtual MaterialOverride overrideMaterial(	const std::string & identifier,
												Moo::EffectMaterialPtr material );
	virtual int gatherMaterials(	const std::string & materialIdentifier,
									std::vector< Moo::Visual::PrimitiveGroup * > & uses,
									Moo::ConstEffectMaterialPtr * ppOriginal = NULL );

	virtual ModelStaticLightingPtr getStaticLighting( StaticLightValueCachePtr, const DataSectionPtr );
	void setStaticLighting( ModelStaticLightingPtr pLighting );

	uint32 sizeInBytes() const;

	virtual NodeTreeIterator nodeTreeBegin() const;
	virtual NodeTreeIterator nodeTreeEnd() const;

protected:
	static ModelPtr load( const std::string & resourceID, DataSectionPtr pFile );
	Model( const std::string & resourceID, DataSectionPtr pFile );

	virtual int initMatter( Matter & m )					  { return 0; }
	virtual bool initTint( Tint & t, DataSectionPtr matSect ) { return false; }

	int  readDyes( DataSectionPtr pFile, bool multipleMaterials );
	bool readSourceDye( DataSectionPtr pSourceDye, DyeSelection & dsel );
	void postLoad( DataSectionPtr pFile );
	

	std::string				resourceID_;
	ModelPtr				parent_;
	float					extent_;

	typedef std::vector< ModelAnimationPtr >	Animations;
	Animations				animations_;

	typedef std::map< std::string, int >	AnimationsIndex;
	AnimationsIndex			animationsIndex_;

	ModelAnimationPtr		pDefaultAnimation_;

#ifdef EDITOR_ENABLED
	static SmartPointer< AnimLoadCallback >		s_pAnimLoadCallback_;
	MetaData::MetaData metaData_;
#endif


	typedef std::vector< ModelActionPtr >	Actions;
	typedef std::map< StringOrderProxy, int >	ActionsIndex;
	Actions					actions_;
	ActionsIndex			actionsIndex_;


	typedef std::vector< Matter * >		Matters;
	Matters					matters_;

	static PropCatalogue	s_propCatalogue_;
	static SimpleMutex		s_propCatalogueLock_;

	static std::vector< std::string > s_warned_; 

private:

	// incremented first thing in each SuperModel::draw call.
	static int				s_blendCookie_;
};



#endif // MODEL_HPP
