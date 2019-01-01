/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASE_BACKUP_SWITCH_MAILBOX_VISITOR_HPP
#define BASE_BACKUP_SWITCH_MAILBOX_VISITOR_HPP

#include "entitydef/mailbox_base.hpp"

#include "network/basictypes.hpp"

class BackupHashChain;

class BaseBackupSwitchMailBoxVisitor : public PyEntityMailBoxVisitor
{
public:
	/**
	 *	Constructor.
	 *
	 *	@param hashChain		The hash chain to use for the remapping.
	 *	@param deadAddr			If this is not Address::NONE, this is checked
	 *							against the mailbox's address as a
	 *							precondition before the remapping for a match
	 *							occurs.
	 */
	BaseBackupSwitchMailBoxVisitor( const BackupHashChain & hashChain, 
			const Mercury::Address & deadAddr = Mercury::Address::NONE ) :
		deadAddr_( deadAddr ),
		hashChain_( hashChain )
	{}

	virtual ~BaseBackupSwitchMailBoxVisitor() {}

	virtual void onMailBox( PyEntityMailBox * pMailBox );

private:
	Mercury::Address 			deadAddr_;
	const BackupHashChain & 	hashChain_;
};

#endif // BASE_BACKUP_SWITCH_MAILBOX_VISITOR_HPP
