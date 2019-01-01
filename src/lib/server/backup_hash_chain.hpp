/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BACKUP_HASH_CHAIN_HPP
#define BACKUP_HASH_CHAIN_HPP

#include "network/basictypes.hpp"
#include "backup_hash.hpp"
#include <map>

class BinaryIStream;
class BinaryOStream;

class BackupHashChain
{
public:
	BackupHashChain();
	~BackupHashChain();

	void adjustForDeadBaseApp( const Mercury::Address & deadApp, 
							   const BackupHash & hash );
	Mercury::Address addressFor( const Mercury::Address & address,
								 EntityID entityID ) const;

private:
	typedef std::map< Mercury::Address, BackupHash > History;
	History history_;

	friend BinaryOStream & operator<<( BinaryOStream & os, 
		const BackupHashChain & hashChain );
	friend BinaryIStream & operator>>( BinaryIStream & is, 
		BackupHashChain & hashChain );
};

BinaryOStream & operator<<( BinaryOStream & os, 
	const BackupHashChain & hashChain );


BinaryIStream & operator>>( BinaryIStream & is, 
		BackupHashChain & hashChain );

#endif // BACKUP_HASH_CHAIN_HPP
