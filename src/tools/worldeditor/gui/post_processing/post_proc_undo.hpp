/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#include "gizmo/undoredo.hpp"
#include "post_processing/effect.hpp"


/**
 *	Interface with the outside world
 */
class PostProcUndo
{
public:
	static bool undoRedoDone();
	static void undoRedoDone( bool val );
};


/**
 *	Chain-level undo-redo
 */
class ChainUndoOp : public UndoRedo::Operation
{
public:
	ChainUndoOp();

	virtual void undo();

	virtual bool iseq( const UndoRedo::Operation & oth ) const;

private:
	std::vector< PyObjectPtr > savedChain_;
};


/**
 *	Effect-level phases list undo-redo
 */
class PhasesUndoOp : public UndoRedo::Operation
{
public:
	PhasesUndoOp( PostProcessing::EffectPtr effect );

	virtual void undo();

	virtual bool iseq( const UndoRedo::Operation & oth ) const;

private:
	PostProcessing::EffectPtr effect_;
	std::vector< PyObjectPtr > savedPhases_;
};
