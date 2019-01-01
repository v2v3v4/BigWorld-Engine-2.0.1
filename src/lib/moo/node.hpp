/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOO_NODE_HPP
#define MOO_NODE_HPP

#include "cstdmf/aligned.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stdmf.hpp"

#include "moo_math.hpp"

#ifdef _WIN32
#include "math/blend_transform.hpp"
#endif

#include "resmgr/datasection.hpp"

namespace Moo
{

class Node;

typedef SmartPointer< Node > NodePtr;
typedef std::vector< NodePtr > NodePtrVector;

/**
 *	This class maintains the transform at a node.
 *	It is used by visuals to make their skeletons of nodes.
 */
class Node : public SafeReferenceCount, public Aligned
{
public:
	Node();
	~Node();

#ifndef MF_SERVER
	void				traverse( );
	void				loadIntoCatalogue( );
#endif
	void				visitSelf( const Matrix& parent );

	void				addChild( NodePtr node );
	void				removeFromParent( );
	void				removeChild( NodePtr node );

	Matrix&				transform( );
	const Matrix&		transform( ) const;
	void				transform( const Matrix& m );

	const Matrix&		worldTransform( ) const;
	void				worldTransform( const Matrix& );

	NodePtr				parent( ) const;

	uint32				nChildren( ) const;
	NodePtr				child( uint32 i );

	const std::string&	identifier( ) const;
	void				identifier( const std::string& identifier );

	NodePtr				find( const std::string& identifier );
	uint32				countDescendants( ) const;

	void				loadRecursive( DataSectionPtr nodeSection );

	float				blend( int blendCookie );
	void				blend( int blendCookie, float blendRatio );
	void				blendClobber( int blendCookie, const Matrix & transform );

#ifdef _WIN32
	BlendTransform &	blendTransform();
#endif

private:

	Matrix				transform_;
	Matrix				worldTransform_;

	int					blendCookie_;
	float				blendRatio_;

#ifdef _WIN32
	BlendTransform		blendTransform_;
#endif
	bool				transformInBlended_;

	// parent is not a smart pointer, this is to ensure that the whole tree
	// will be destructed if the parent's reference count reaches 0
	Node*				parent_;
	NodePtrVector		children_;

	std::string			identifier_;


	Node(const Node&);
	Node& operator=(const Node&);

public:
	static int			s_blendCookie_;
};

}

#ifdef CODE_INLINE
#include "node.ipp"
#endif




#endif // MOO_NODE_HPP
