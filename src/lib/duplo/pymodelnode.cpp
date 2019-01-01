/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "pymodelnode.hpp"

#pragma warning (disable:4355)	// this used in initialiser list

#include "moo/render_context.hpp"

DECLARE_DEBUG_COMPONENT2( "Duplo", 0 )


// -----------------------------------------------------------------------------
// Section: PyModelNode
// -----------------------------------------------------------------------------



PY_TYPEOBJECT( PyModelNode )

PY_BEGIN_METHODS( PyModelNode )
	PY_METHOD( attach )
	PY_METHOD( detach )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyModelNode )
	/*~ attribute PyModelNode.inWorld
	 *
	 *	This attribute is used to determine whether the parent model has been
	 *	added to the world.  It is read only, and is set to 1 if the Model is
	 *	in the world, 0 otherwise.  In order to add a model to the world, you
	 *	can attach it to another model, or set the model property of an entity
	 *	to this Model.
	 *
	 *	@type	Read-Only Integer
	 */
	PY_ATTRIBUTE( inWorld )
	/*~ attribute PyModelNode.attachments
	 *
	 *	This attribute is the list of all PyAttachments which are attached to
	 *	this node.  PyAttachments can be added and removed from the list using
	 *	the attach and detach functions.
	 *
	 *	@type	List of PyAttachments
	 */
	PY_ATTRIBUTE( attachments )
	/*~ attribute PyModelNode.name
	 *
	 *	This attribute is the name of the node
	 *
	 *	@type	Read-Only String
	 */
	PY_ATTRIBUTE( name )
	/*~ attribute PyModelNode.local
	 *
	 *	This attribute is the local transform for the node.  The local transform
	 *	is pre-multiplied to the node matrix and affects every attached object.
	 *
	 *	@type	Read-Only MatrixProvider
	 */
	PY_ATTRIBUTE( local )
PY_END_ATTRIBUTES()




/**
 *	Constructor
 */
PyModelNode::PyModelNode( PyModel * pOwner,
		Moo::NodePtr pNode,
		MatrixProviderPtr l,
		PyTypePlus * pType ):
	MatrixProviderLiaison( false, pType ),
	localTransform_( l ),
	pOwner_( pOwner ),
	pNode_( pNode ),
	attachmentsHolder_( attachments_, this, true ),
	pHardpointAttachment_( NULL ),
	lodded_( true ),
	lastWorldTransform_( Matrix::identity )
{
}

/**
 *	Destructor
 */
PyModelNode::~PyModelNode()
{
	BW_GUARD;
	MF_ASSERT( pOwner_ == NULL )
}


/**
 *	Tick method
 */
void PyModelNode::tick( float dTime )
{
	BW_GUARD;
	for (PyAttachments::iterator it = attachments_.begin();
		it != attachments_.end();
		it++)
	{
		(*it)->tick( dTime );
	}
}


/**
 *	Draw method
 */
void PyModelNode::draw( float lod )
{
	BW_GUARD;
	if (lodded_) return;

	for (PyAttachments::iterator it = attachments_.begin();
		it != attachments_.end();
		it++)
	{
		(*it)->draw( lastWorldTransform_, lod );
	}
}

/**
 *	Our model has entered the world, so tell all attachments
 */
void PyModelNode::enterWorld()
{
	BW_GUARD;
	for (PyAttachments::iterator it = attachments_.begin();
		it != attachments_.end();
		it++)
	{
		(*it)->enterWorld();
	}
}

/**
 *	Our model has left the world, so tell all attachments
 */
void PyModelNode::leaveWorld()
{
	BW_GUARD;
	for (PyAttachments::iterator it = attachments_.begin();
		it != attachments_.end();
		it++)
	{
		(*it)->leaveWorld();
	}
}


/**
 *	Our model has been moved, so tell all attachments
 */
void PyModelNode::move( float dTime )
{
	for (PyAttachments::iterator it = attachments_.begin();
		it != attachments_.end();
		it++)
	{
		(*it)->move( dTime );
	}
}


/**
 *	We want to add up everyone's bounding boxes
 */
void PyModelNode::worldBoundingBox( BoundingBox & bb, const Matrix& world )
{
	BW_GUARD;
	if (attachments_.empty()) return;

	for (PyAttachments::iterator it = attachments_.begin();
		it != attachments_.end();
		it++)
	{
		(*it)->worldBoundingBox( bb, lastWorldTransform_ );
	}
}

/**
 *	Our model has been tossed, so tell all attachments
 */
void PyModelNode::tossed( bool outside )
{
	BW_GUARD;
	for (PyAttachments::iterator it = attachments_.begin();
		it != attachments_.end();
		it++)
	{
		(*it)->tossed( outside );
	}
}


/**
 *	Latch the current value of our node's world transform
 */
void PyModelNode::latch()
{
	BW_GUARD;
	if (pNode_->blend( Model::blendCookie() ) <= 0.f)
	{
		lodded_ = true;
		lastWorldTransform_ = Moo::rc().world();
	}
	else
	{
		lodded_ = false;
		lastWorldTransform_ = pNode_->worldTransform();
	}
	if (localTransform_.exists())
	{
		Matrix l;
		localTransform_->matrix(l);
		lastWorldTransform_.preMultiply(l);
	}
}

/**
 *	Detach since our model has been destroyed
 */
void PyModelNode::detach()
{
	BW_GUARD;
	for (PyAttachments::iterator it = attachments_.begin();
		it != attachments_.end();
		it++)
	{
		(*it)->detach();
	}

	pOwner_ = NULL;
}



/**
 *	This allows scripts to get the attachments attached to this node,
 *	interpreted as if it were a hardpoint.
 */
PyObject * PyModelNode::pyGetHardPoint()
{
	BW_GUARD;
	for (uint i = 0; i < attachments_.size(); i++)
	{
		PyAttachment * pAttachment = &*attachments_[i];

		// Note: The pHardpointAttachment_ pointer will be (knowingly) left
		// dangling if the attachment is removed manually from the attachments
		// list, i.e. if pySetHardPoint was not used. So the cached value here
		// can never be accessed, only compared. And yes, it is possible to
		// remove the hardpoint attachment, reattach it in a different way
		// (or destroy it and a new object gets the same pointer) but those
		// are the (user-imposed) hazards of mixing node attachment interfaces.
		if (pAttachment == pHardpointAttachment_)
		{
			Py_INCREF( pAttachment );
			return pAttachment;
		}
	}

	Py_Return;
}


/**
 *	This allows scripts to set the attachments attached to this node,
 *	interpreted as if it were a hardpoint.
 */
int PyModelNode::pySetHardPoint( PyObject * value )
{
	BW_GUARD;
	// ok, now make sure that value is an attachment
	if (value == NULL || (!PyAttachment::Check( value ) && value != Py_None) )
	{
		PyErr_Format( PyExc_TypeError, "Model.%s "
			"must be set to an Attachment", pNode_->identifier().c_str() );
		return -1;
	}

	// if we are attaching a model that's not blank,
	//  make sure it has a corresponding hard point
	PyModel * pSubModel = NULL;
	if (PyModel::Check( value ) && ((PyModel*)value)->pSuperModel() != NULL)
	{
		pSubModel = ((PyModel*)value);

		if (!pSubModel->pSuperModel()->findNode( pNode_->identifier() ))
		{
			PyErr_Format( PyExc_ValueError, "Model.%s can only be set to a "
				"non-blank Model if it has a corresponding hardpoint",
				pNode_->identifier().c_str() );
			return -1;
		}
	}

	// ok then, erase the existing hardpoint, if we still have it
	for (uint i = 0; i < attachments_.size(); i++)
	{
		if (&*attachments_[i] == pHardpointAttachment_)
		{
			// unless we're attaching the same one!
			if ((PyAttachment*)value == pHardpointAttachment_)
				return 0;	// 'value' OK to be Py_None above

			attachmentsHolder_.erase( i, 1 );
			// cannot have duplicates in list (would be 'already attached')
			break;
		}
	}
	pHardpointAttachment_ = NULL;

	// and if there's a new one
	if (value != Py_None)
	{
		PyAttachment * pAttachment = (PyAttachment*)value;
		pHardpointAttachment_ = pAttachment;

		// insert the new one
		attachmentsHolder_.insertRange( 0, 1 );
		if (attachmentsHolder_.insert( pAttachment ) != 0)
		{
			// get out now if it failed
			attachmentsHolder_.cancel();
			return -1;
		}

		// reroute the root to be the corresponding hardpoint (if applicable)
		if (pSubModel != NULL)
		{
			if (!pSubModel->origin( pNode_->identifier() ))
			{
				WARNING_MSG( "PyModelNode::pySetHardPoint: "
					"origin() unexpectedly failed. Safe, but bad.\n" );
				// get out now if it failed (it shouldn't - we already checked)
				attachmentsHolder_.cancel();
				return -1;
			}
		}
	}

	// and finally commit to those changes!
	attachmentsHolder_.commit();

	return 0;
}


/**
 *	MatrixProvider matrix method
 */
void PyModelNode::matrix( Matrix & m ) const
{
	m = lastWorldTransform_;
}


/**
 *	Python get attribute method
 */
PyObject * PyModelNode::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return this->MatrixProviderLiaison::pyGetAttribute( attr );
}

/**
 *	Python set attribute method
 */
int PyModelNode::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return this->MatrixProviderLiaison::pySetAttribute( attr, value );
}


/*~ function PyModelNode.attach
 *
 *	This function takes a PyAttachment and attaches it to the node.  The
 *  attachment must not already be attached to something. Attaching it causes
 *  the attachment to appear with its root node with the same position and
 *	rotation as this node. It also sets the attachments inWorld attribute to
 *  the inWorld Attribute of this PyModelNode.
 *
 *	Examples of things which can be attached are models and particle systems.
 *
 *	@param	attachment	the PyAttachment to be attached to this node.
 *
 */
/**
 *	Attach called from python
 */
bool PyModelNode::attach( PyAttachmentPtr pAttachment )
{
	BW_GUARD;
	attachmentsHolder_.insertRange( attachments_.size(), 1 );
	if (attachmentsHolder_.insert( &*pAttachment ) != 0)
	{
		// PySTLSequenceHolder insert will raise an error via Script::setData
		attachmentsHolder_.cancel();
		return false;
	}

	attachmentsHolder_.commit();
	return true;
}

/*~ function PyModelNode.detach
 *
 *	This function takes a PyAttachment, and tries to detach it from this node.
 *	This will fail and error if the attachment wasn't attached to this node
 *  when the call was made.
 *
 *  Raises a ValueError if the given attachment is not attached to this node.
 *
 *	@param	attachment	the PyAttachment to be detached from this node.
 */
/**
 *	Detach called from python
 */
bool PyModelNode::detach( PyAttachmentPtr pAttachment )
{
	BW_GUARD;
	PyAttachments::iterator found = std::find(
		attachments_.begin(), attachments_.end(), pAttachment );
	if (found != attachments_.end())
	{
		int index = found-attachments_.begin();
		attachmentsHolder_.erase( index, index+1 );
		attachmentsHolder_.commit();
		return true;
	}

	PyErr_SetString( PyExc_ValueError, "PyModelNode.detach(): "
		"The given Attachment is not attached here." );
	return false;
}

// pymodelnode.cpp
