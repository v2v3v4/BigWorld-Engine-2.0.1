/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "base_backup_switch_mailbox_visitor.hpp"
#include "backup_hash_chain.hpp"

/**
 *	Switches mailboxes over to use the new backup location when a BaseApp has
 *	shut down.
 */
void BaseBackupSwitchMailBoxVisitor::onMailBox( PyEntityMailBox * pMailBox )
{
	if (deadAddr_ == Mercury::Address::NONE || pMailBox->address() == deadAddr_)
	{
		pMailBox->address( hashChain_.addressFor( 
			pMailBox->address(), pMailBox->id() ) );
	}
}

// base_backup_switch_mailbox_visitor.cpp
