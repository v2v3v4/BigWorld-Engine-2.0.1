/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef XML_BILLING_SYSTEM_HPP
#define XML_BILLING_SYSTEM_HPP

#include "dbmgr_lib/billing_system.hpp"
#include "resmgr/datasection.hpp"

#include <map>

class EntityDefs;
class XMLDatabase;

/**
 *	This class implements the XML database functionality.
 */
class XMLBillingSystem : public BillingSystem
{
public:
	XMLBillingSystem( DataSectionPtr pLogonMapSection,
			const EntityDefs & entityDefs,
			XMLDatabase * pDatabase );

	virtual void getEntityKeyForAccount(
		const std::string & logOnName, const std::string & password,
		const Mercury::Address & clientAddr,
		IGetEntityKeyForAccountHandler & handler );
	virtual void setEntityKeyForAccount( const std::string & username,
		const std::string & password, const EntityKey & ekey );

private:
	void initLogOnMapping( XMLDatabase * pDatabase );

	// Equivalent of bigworldLogOnMapping table in MySQL.
	class LogOnMapping
	{
	public:
		LogOnMapping( const EntityKey & key, const std::string & password ) :
			entityKey_( key ),
			password_( password )
		{}
		LogOnMapping() : entityKey_( 0, 0 ), password_( "" ) {}

		bool matchesPassword( const std::string & password ) const 
		{
			return password == password_;
		}

		const EntityKey & entityKey() const	{ return entityKey_; }

	private:
		EntityKey		entityKey_;
		std::string		password_;
	};

	// The key is the "logOnName" column.
	typedef std::map< std::string, LogOnMapping > LogonMap;
	LogonMap		logonMap_;
	DataSectionPtr	pLogonMapSection_;

	const EntityDefs & entityDefs_;
};

#endif // XML_BILLING_SYSTEM_HPP
