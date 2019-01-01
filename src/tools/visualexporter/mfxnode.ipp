/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

// INLINE void MFXNode::inlineFunction()
// {
// }


INLINE 
void MFXNode::setMaxNode( INode *node )
{
	node_ = node;
}

INLINE
INode* MFXNode::getMaxNode( void ) const
{
	return node_;
}

INLINE
void MFXNode::setTransform( const Matrix3 &m )
{
	transform_ = m;
}

INLINE
const Matrix3 &MFXNode::getTransform( void ) const
{
	return transform_;
}

INLINE
int MFXNode::getNChildren( void ) const
{
	return children_.size();
}

INLINE
MFXNode* MFXNode::getChild( int n ) const
{
	if( n < children_.size() )
		return children_[ n ];

	return NULL;
}

INLINE
MFXNode* MFXNode::getParent( void ) const
{
	return parent_;
}

INLINE
void MFXNode::addChild( MFXNode *node )
{
	if( node )
	{
		children_.push_back( node );
		node->parent_ = this;
	}

}

INLINE
const std::string &MFXNode::getIdentifier( void )
{
	if( node_ )
		identifier_ = node_->GetName();

	return identifier_;
}

INLINE
void MFXNode::setIdentifier( const std::string &s )
{
	identifier_ = s;
}

INLINE
MFXNode *MFXNode::find( INode *node )
{
	if( node == node_ )
		return this;
	uint32 i = 0;
	MFXNode *ret = NULL;
	while( children_.size() > i && ret == NULL )
	{
		ret = children_[ i++ ]->find( node );
	}
	return ret;
}

INLINE
MFXNode *MFXNode::find( const std::string &identifier )
{
	if( identifier == getIdentifier() )
		return this;
	uint32 i = 0;
	MFXNode *ret = NULL;
	while( children_.size() > i && ret == NULL )
	{
		ret = children_[ i++ ]->find( identifier );
	}
	return ret;

}

INLINE
bool MFXNode::contentFlag( ) const
{
	return contentFlag_;
}

INLINE
void MFXNode::contentFlag( bool state )
{
	contentFlag_ = state;
}


/*mfxnode.ipp*/