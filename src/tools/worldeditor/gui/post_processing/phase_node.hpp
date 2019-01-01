/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PHASE_NODE_HPP
#define PHASE_NODE_HPP


#include "base_post_processing_node.hpp"


// Forward declarations
namespace PostProcessing
{
	class Phase;
	typedef SmartPointer<Phase> PhasePtr;
}

class PhaseNode;
typedef SmartPointer< PhaseNode > PhaseNodePtr;


/**
 *	This class implements a Phase node, implementing the necessary additional
 *	editor info for a Post-processing Phase.
 */
class PhaseNode : public BasePostProcessingNode
{
public:
	static const char * BACK_BUFFER_STR;

	PhaseNode( PostProcessing::Phase * pyPhase, NodeCallback * callback );
	~PhaseNode();

	const std::string & name() const { return name_; }

	std::string output() const;

	virtual PhaseNode * phaseNode() { return this; }

	PostProcessing::PhasePtr pyPhase() const { return pyPhase_; }

	PyObject * pyObject() const { return (PyObject *)(pyPhase_.get()); }

	PhaseNodePtr clone() const;

	void edEdit( GeneralEditor * editor );
	bool edLoad( DataSectionPtr pDS );
	bool edSave( DataSectionPtr pDS );

	std::string getName() const;
	bool setName( const std::string & name );

private:
	PostProcessing::PhasePtr pyPhase_;
	std::string name_;
	std::string output_;
};


#endif // PHASE_NODE_HPP
