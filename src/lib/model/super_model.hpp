/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SUPER_MODEL_HPP
#define SUPER_MODEL_HPP


#include <vector>

#include "cstdmf/concurrency.hpp"
#include "moo/forward_declarations.hpp"
#include "moo/moo_math.hpp"
#include "network/basictypes.hpp"	// this include is most unfortunate...
#include "physics2/worldtri.hpp"
#include "resmgr/forward_declarations.hpp"

#include "forward_declarations.hpp"
#include "fashion.hpp"
#include "model.hpp"


class Action;
class Animation;
class BoundingBox;
class BSPTree;
class StaticLightValues;


typedef std::vector< Moo::NodePtr > MooNodeChain;


int initMatter_NewVisual( Matter & m, Moo::Visual & bulk );
bool initTint_NewVisual( Tint & t, DataSectionPtr matSect );



/**
 *	This class defines a super model. It's not just a model,
 *	it's a whole lot more - discrete level of detail, billboards,
 *	continuous level of detail, static mesh animations, multi-part
 *	animated models - you name it, this baby's got it.
 */
class SuperModel
{
public:
	explicit SuperModel( const std::vector< std::string > & modelIDs );
	~SuperModel();

#ifdef EDITOR_ENABLED
	virtual void reload();
#endif//EDITOR_ENABLED

	// If you want to do stuff between the dressing and drawing,
	//  then make a class that derives from fashion and pass it in.
	float draw( const FashionVector * pFashions = NULL,
		int nLateFashions = 0, float atLod = -1.f, bool doDraw = true );

	void dressInDefault();

	Moo::NodePtr rootNode();
	Moo::NodePtr findNode( const std::string & nodeName,
		MooNodeChain * pParentChain = NULL );

	SuperModelAnimationPtr	getAnimation( const std::string & name );
	SuperModelActionPtr		getAction( const std::string & name );
	void getMatchableActions( std::vector<SuperModelActionPtr> & actions );
	SuperModelDyePtr		getDye( const std::string & matter, const std::string & tint );

	int nModels() const						{ return nModels_; }
	Model * curModel( int i )				{ return models_[i].curModel; }
	ModelPtr topModel( int i )				{ return models_[i].topModel; }

	float lastLod() const					{ return lod_; }

	void localBoundingBox( BoundingBox & bbRet ) const;
	void localVisibilityBox( BoundingBox & vbRet ) const;

	void checkBB( bool checkit = true )		{ checkBB_ = checkit; }
	void redress()							{ redress_ = true; }

	bool isOK() const						{ return isOK_; }

	int numTris() const;
	int numPrims() const;

private:
	struct ModelStuff
	{
		ModelPtr	  topModel;
		Model		* preModel;
		Model		* curModel;
	};
	std::vector<ModelStuff>		models_;

	int							nModels_;

	float						lod_;
	float						lodNextUp_;
	float						lodNextDown_;

	bool	checkBB_;
	bool	redress_;	// HACK!

	bool	isOK_;
};



#ifdef CODE_INLINE
	#include "super_model.ipp"
#endif


#endif // SUPER_MODEL_HPP
