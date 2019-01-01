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
#include "undoredo_op.hpp"
#include "main_frame.hpp"

UndoRedoOp::UndoRedoOp(ActionKind actionKind, DataSectionPtr pDS)
:
UndoRedo::Operation((int)actionKind),
data_(pDS)
{
}

UndoRedoOp::~UndoRedoOp()
{
    data_ = NULL;
}

void UndoRedoOp::undo()
{
	BW_GUARD;

    MainFrame::instance()->CopyFromDataSection
    ( 
        this->kind_, 
        this->data_ 
    );
    MainFrame::instance()->RefreshGUI(this->kind_);
}

bool UndoRedoOp::iseq(Operation const &oth) const
{
	BW_GUARD;

    const UndoRedoOp& oth2 =
        static_cast<const UndoRedoOp&>(oth);
    return XmlSectionsAreEq(data_, oth2.data());
}

DataSectionPtr UndoRedoOp::data() const 
{ 
    return data_; 
}

bool UndoRedoOp::XmlSectionsAreEq
( 
    DataSectionPtr const &one, 
    DataSectionPtr const &two 
) const
{
	BW_GUARD;

    const BinaryPtr s1 = one->asBinary();
    const BinaryPtr s2 = two->asBinary();

    if (s1->len() != s2->len())
        return false;

    if (strncmp( (char*)s1->data(), (char*)s2->data(), s1->len() ))
        return false;

    return true;
}