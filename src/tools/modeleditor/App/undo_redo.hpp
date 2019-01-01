/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UNDO_REDO_HPP
#define UNDO_REDO_HPP

#include "gizmo/undoredo.hpp"

typedef SmartPointer< class XMLSection > XMLSectionPtr;
typedef std::pair< std::string, std::string > StringPair;

class UndoRedoOp : public UndoRedo::Operation
{
public:
	UndoRedoOp( int kind, DataSectionPtr data, DataSectionPtr parent = NULL, bool materialFlagChange = false, StringPair item = StringPair() );
	~UndoRedoOp();
	virtual void undo();
	virtual bool iseq( const UndoRedo::Operation & oth ) const;
	const DataSectionPtr data() const	{ return data_; }
private:
	int kind_;
	DataSectionPtr data_;
	DataSectionPtr parent_;

	XMLSectionPtr state_;
	bool materialFlagChange_;
	StringPair item_;
};


class UndoRedoMatterName : public UndoRedo::Operation
{
public:
	UndoRedoMatterName( const std::string & oldName, const std::string & newName );
	~UndoRedoMatterName();

	virtual void undo();
	virtual bool iseq( const UndoRedo::Operation & oth ) const;
private:
	std::string oldName_;
	std::string newName_;

};


class UndoRedoTintName : public UndoRedo::Operation
{
public:
	UndoRedoTintName( const std::string & matterName, const std::string & oldName, const std::string & newName );
	~UndoRedoTintName();

	virtual void undo();
	virtual bool iseq( const UndoRedo::Operation & oth ) const;
private:
	std::string matterName_;
	std::string oldName_;
	std::string newName_;

};

#endif // UNDO_REDO_HPP