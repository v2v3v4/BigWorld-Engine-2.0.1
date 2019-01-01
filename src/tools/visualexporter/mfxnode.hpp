/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma warning ( disable : 4530 )

#ifndef MFXNODE_HPP
#define MFXNODE_HPP

#include <iostream>
#include <vector>
#include <string>

#include "resmgr/datasection.hpp"
#include "max.h"
#include "cstdmf/stdmf.hpp"

#include <algorithm>

class MFXNode
{
public:
	MFXNode();
	MFXNode( INode *node );
	~MFXNode();

	void setMaxNode( INode *node );
	INode* getMaxNode( void ) const;

	void setTransform( const Matrix3 &m );
	const Matrix3 &getTransform( void ) const;

	int getNChildren( void ) const;
	MFXNode* getChild( int n ) const;
	MFXNode* getParent( void ) const;

	void addChild( MFXNode *node );

	void removeChild( int n );
	void removeChild( MFXNode *n );

	//removeChildAddChildren removes the child from this node, and adds all children
	//from the child to this node
	void removeChildAddChildren( MFXNode *n );
	void removeChildAddChildren( int n );

	
	Matrix3 getTransform( TimeValue t, bool normalise = false );
	Matrix3 getRelativeTransform( TimeValue t, bool normalise = false,
		MFXNode * idealParent = NULL );

	const std::string &getIdentifier( void );
	void setIdentifier( const std::string &s );

	int treeSize( void );

	MFXNode *find( INode *node );
	MFXNode *find( const std::string &identifier );

	bool contentFlag( ) const;
	void contentFlag( bool state );

	bool include( ) const {return include_;};
	void include( bool state ) {include_ = state;};

	void includeAncestors();
	void delChildren();

	void exportTree( DataSectionPtr pParentSection, MFXNode* idealParent = NULL, bool* hasInvalidTransforms = NULL );

private:

	std::string identifier_;
	MFXNode* parent_;
	std::vector < MFXNode * > children_;

	INode *node_;
	Matrix3 transform_;

	bool contentFlag_;
	bool include_;

	MFXNode(const MFXNode&);
	MFXNode& operator=(const MFXNode&);

	friend std::ostream& operator<<(std::ostream&, const MFXNode&);
};

#ifdef CODE_INLINE
#include "mfxnode.ipp"
#endif


#endif
/*mfxnode.hpp*/