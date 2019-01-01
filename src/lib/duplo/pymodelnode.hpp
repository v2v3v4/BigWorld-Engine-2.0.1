/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PYMODELNODE_HPP
#define PYMODELNODE_HPP

#include "py_attachment.hpp"
#include "pymodel.hpp"
#include "pyscript/script_math.hpp"


/*~ class BigWorld.PyModelNode
 *
 *	A PyModelNode is a MatrixProvider
 *	which is weighted with other nodes to calculate the transformation matrix
 *  for a particular piece of geometry.  In a biped for example, "biped Spine",
 *  "biped Spine1" and "biped Pelvis" are all nodes which are used to transform
 *  the torso geometry.
 *
 *	PyAttachments can be attached to PyModelNodes, which will store them in the
 *  attachments attribute.
 *
 *	PyModelNodes cannot be created dynamically, but are part of a PyModel, which
 *	is loaded from a .model file. They can be accessed using the node function,
 *	which obtains the PyModelNode of the specified name from a model.
 *
 *	A PyModelNode's matrix is only updated after the owning model's nodes are
 *	updated, which is at draw time.  This means that from script the very first
 *	time you create a PyModelNode referece it's internal transform wil not
 *	yet be set.  You will have to wait until the owning model updates and draws
 *	at least once before accessing the node's matrix.
 *
 *	For example:
 *	@{
 *	myNode = model.node( "biped Head" )
 *	@}
 *	obtains the head node of the model.
 */
/**
 *	This class is a node in a PyModel
 */
class PyModelNode : public MatrixProviderLiaison, public Aligned
{
	Py_Header( PyModelNode, MatrixProviderLiaison )

public:
	PyModelNode( PyModel * pOwner,
		Moo::NodePtr pNode,
		MatrixProviderPtr local,
		PyTypePlus * pType = &s_type_ );
	~PyModelNode();

	void tick( float dTime );
	void move( float dTime );
	void draw( float lod );

	void enterWorld();
	void leaveWorld();

	void worldBoundingBox( BoundingBox & bb, const Matrix& world );

	void tossed( bool outside );

	void latch();
	void detach();

	PyObject * pyGetHardPoint();
	int pySetHardPoint( PyObject * value );

	virtual void matrix( Matrix & m ) const;

	virtual const Matrix & getMatrix() const	{ return lastWorldTransform_; }
	virtual bool setMatrix( const Matrix & m )	{ return true; }

	MatrixProviderPtr localTransform() const	{ return localTransform_; }
	void localTransform(MatrixProviderPtr l)	{ localTransform_ = l; }

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_RO_ATTRIBUTE_DECLARE( pOwner_ && pOwner_->isInWorld(), inWorld )
	PY_RW_ATTRIBUTE_DECLARE( attachmentsHolder_, attachments )
	PY_RO_ATTRIBUTE_DECLARE( pNode_->identifier(), name )
	PY_RO_ATTRIBUTE_DECLARE( localTransform_, local )

	bool attach( PyAttachmentPtr pAttachment );
	bool detach( PyAttachmentPtr pAttachment );
	bool hasAttachments() const					{ return !attachments_.empty(); }
	PY_AUTO_METHOD_DECLARE( RETOK, attach, NZARG( PyAttachmentPtr, END ) )
	PY_AUTO_METHOD_DECLARE( RETOK, detach, NZARG( PyAttachmentPtr, END ) )

	// Should we have the dictionary-like named attachments here?

	Moo::NodePtr		pNode()				{ return pNode_; }

private:
	PyModel				* pOwner_;
	Moo::NodePtr		pNode_;

	PyAttachments						attachments_;
	PySTLSequenceHolder<PyAttachments>	attachmentsHolder_;

	PyAttachment		* pHardpointAttachment_;

	bool				lodded_;
	MatrixProviderPtr	localTransform_;
	Matrix				lastWorldTransform_;
};


#endif // PYMODELNODE_HPP
