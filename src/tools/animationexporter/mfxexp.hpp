/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MFXEXP_HPP
#define MFXEXP_HPP

#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4530 )

#include <iostream>
#include <vector>
#include <algorithm>

#include "cstdmf/stringmap.hpp"
#include "resmgr/dataresource.hpp"

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "interpik.h"
#include "modstack.h"
#include "phyexp.h"

#include "expsets.hpp"
#include "mfxfile.hpp"
#include "mfxnode.hpp"
#include "visual_mesh.hpp"
#include "visual_envelope.hpp"


//external functions
extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

//Version number of exporter * 100
const int MFX_EXPORT_VERSION = 100;

//Global class id.
#if defined BW_EXPORTER_DEBUG
#define MFEXP_CLASS_ID	Class_ID(0x23472c6, 0x552c1156)
#else
#define MFEXP_CLASS_ID	Class_ID(0x25810e56, 0x61a93faa)
#endif

//Config fileName
const std::string CFGFilename("animationexporter.cfg");

typedef std::vector< INode* > INodeVector;

class MaterialList
{
private:

	std::vector< Mtl* > materials_;

public:
	void addMaterial( Mtl * mtl )
	{
		if( mtl )
		{
			std::vector< Mtl* >::iterator it = std::find( materials_.begin(), materials_.end(), mtl );
			if( it == materials_.end() )
			{
				materials_.push_back( mtl );
			}
		}
	}

	Mtl *getMaterial( int index ) const
	{
		return materials_[ index ];
	}

	int getNMaterials( void ) const
	{
		return materials_.size();
	}


};

typedef struct
{
	INode *node;
	std::vector< int > vertexIndices;
} Bone;

class BoneList
{
private:

	std::vector< Bone > bones_;

public:

	/*
	 * Find a bone by node reference
	 */
	Bone *findBone( INode *node )
	{
		for( uint i = 0; i < bones_.size(); i++ )
		{
			if( node == bones_[ i ].node )
				return &bones_[ i ];
		}
		return NULL;
	}

	/*
	 * Add a bone to the bone list
	 * If there is a bone attached to the node already, the index gets added to the
	 * vertexIndex list of that bone.
	 */
	void addBone( INode *node, int index )
	{
		Bone *bone = findBone( node );
		if( bone )
		{
			bone->vertexIndices.push_back( index );
		}
		else
		{
			Bone bn;
			bn.node = node;
			bn.vertexIndices.push_back( index );
			bones_.push_back( bn );
		}
	}

	Bone *getBone( int index )
	{
		return &bones_[ index ];
	}

	int getNBones( void ) const
	{
		return bones_.size();
	}
};

typedef struct 
{
	Mtl *mtl_;
	std::vector< std::string > children_;

} MultiSubMaterial;

class MFXExport : public SceneExport
{
public:
	MFXExport();
	~MFXExport();

	int				ExtCount();					// Number of extensions supported
	const TCHAR *	Ext(int n);					// Extension #n
	const TCHAR *	LongDesc();					// Long ASCII description
	const TCHAR *	ShortDesc();				// Short ASCII description
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box
	int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options = 0);	// Export file
	BOOL			SupportsOptions( int ext, DWORD options ) { return options == SCENE_EXPORT_SELECTED; }

	static Modifier* findPhysiqueModifier( INode* node );
	static Modifier* findSkinMod( INode* node );
	static Modifier* findMorphModifier( INode* node );
	static TriObject* getTriObject( INode *node, TimeValue t, bool &needDelete );

private:
	void preProcess( INode* node, MFXNode* mfxParent = NULL );
	void loadReferenceNodes( const std::string & mfxFile );
	/*
	 * Export functions
	 */
	void exportMeshes( void );
	void exportMesh( INode *node );

	void traverseExportAnimations( MFXNode *node );
	void exportEnvelopes( void );
	void exportEnvelope( INode *node );
	
	void readReferenceHierarchy( DataSectionPtr hierarchy );

	/*
	 * Helper functions
	 */
	TimeValue staticFrame( void ){ return ExportSettings::instance().staticFrame() * GetTicksPerFrame(); }

	std::string getCfgFilename( void );
	MultiSubMaterial *findMultiSubMaterial( Mtl *mtl );

	bool validateAnimationIDs( std::string& errorMsg );

	Interface*		ip_; //max Interface

	MaterialList	materials_; //Our list of materials
	unsigned int	nodeCount_; //Number of nodes
	unsigned int	animNodeCount_; //Number of animated nodes

	bool			errors_; //exported with errors

	MFXFile			mfx_;

	INodeVector portalNodes_;
	INodeVector envelopeNodes_;
	INodeVector meshNodes_;
	INodeVector touchedNodes_;
	std::vector< MultiSubMaterial > multiSubs_;

	std::vector<VisualMeshPtr> visualMeshes_;
	std::vector<VisualEnvelopePtr> visualEnvelopes_;


	std::vector< MFXNode* > mfxEnvelopeNodes_;
	MFXNode* mfxRoot_;

	StringHashMap<std::string> nodeParents_;
	
	MFXExport(const MFXExport&);
	MFXExport& operator=(const MFXExport&);
	friend std::ostream& operator<<(std::ostream&, const MFXExport&);
};

#endif
/*mfxexp.hpp*/