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
#include "auto_config.hpp"
#include "multi_file_system.hpp"
#include "xml_section.hpp"

// -----------------------------------------------------------------------------
// Section: AutoConfig
// -----------------------------------------------------------------------------


const std::string AutoConfig::s_resourcesXML = "resources.xml";


/**
 *	Constructor.
 */
AutoConfig::AutoConfig()
{
	if (s_all == NULL) s_all = new std::vector<AutoConfig*>();
	s_all->push_back( this );
}

/**
 *	Destructor.
 */
AutoConfig::~AutoConfig()
{
	if ( s_all )
	{
		// Find self in all list
		std::vector<AutoConfig*>::iterator it = 
		std::find( s_all->begin(), s_all->end(), this );
	
		// If present, then remove self.
		if ( it != s_all->end() )
		{
			s_all->erase( it );
		}

		// If list is empty, then delete it.
		if ( s_all->empty() )
		{
			delete s_all;
			s_all = NULL;
		}
	}
}


/**
 *	Static method to configure all from given section
 */
void AutoConfig::configureAllFrom( DataSectionPtr pConfigSection )
{
	if (s_all == NULL) return;

	for (AutoConfigs::iterator it = s_all->begin();
		it != s_all->end();
		it++)
	{
		(**it).configureSelf( pConfigSection );
	}

	delete s_all;
	s_all = NULL;
}


/**
 *	Static method to configure all from given xml file name.
 *	If there are multiple XML files with the given name in different
 *	resource paths, then all will be considered.
 *	This method only works with XML data.
 */
bool AutoConfig::configureAllFrom( const std::string& xmlResourceName )
{
	DataSectionPtr pResourcesXML = BWResource::openSection(xmlResourceName);
	if ( !pResourcesXML )
	{
		return false;
	}

	MultiFileSystemPtr mfs = BWResource::instance().fileSystem();
	std::vector<BinaryPtr> sections;
	mfs->collateFiles(xmlResourceName,sections);

	std::vector<DataSectionPtr> xmlSections;

	for (std::vector<BinaryPtr>::iterator it = sections.begin();
		 it != sections.end();
		 it++ )
	{		
		DataSectionPtr pSec = DataSection::createAppropriateSection( "root", *it );
		xmlSections.push_back(pSec);		
	}

	return AutoConfig::configureAllFrom(xmlSections);
}


/**
 *	Static method to configure all from given list of sections.
 *	The sections should be ordered most-important to least important,
 *	i.e. the most important section has the value you want to hold
 *	in the end.
 */
bool AutoConfig::configureAllFrom( std::vector<DataSectionPtr>& pConfigSections )
{
	if (s_all == NULL) return true;
	if (!pConfigSections.size()) return false;

	for (AutoConfigs::iterator it = s_all->begin();
		it != s_all->end();
		it++)
	{
		//do it in reverse order, so the least important resource sections
		//are done first, and are overriden by the most important.
		for (uint32 i=1; i<=pConfigSections.size(); i++)
		{
			(**it).configureSelf( pConfigSections[pConfigSections.size()-i] );
		}
	}

	delete s_all;
	s_all = NULL;
	return true;
}


/// static initialiser
AutoConfig::AutoConfigs * AutoConfig::s_all = NULL;

// -----------------------------------------------------------------------------
// Section: AutoConfigString
// -----------------------------------------------------------------------------


/**
 *	Constructor.
 */
BasicAutoConfig< std::string >::BasicAutoConfig( const char * path,
		const std::string & deft ) :
	AutoConfig(),
	path_( path ),
	value_( deft )
{
}

/**
 *	Destructor.
 */
BasicAutoConfig< std::string >::~BasicAutoConfig()
{
}


/**
 *	Configure yourself
 */
void BasicAutoConfig< std::string >::configureSelf( DataSectionPtr pConfigSection )
{
	DataSectionPtr pValue = pConfigSection->openSection( path_ );
	if (pValue)
		value_ = pValue->asString();	// even if empty
}


// -----------------------------------------------------------------------------
// Section: AutoConfigStrings
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
AutoConfigStrings::AutoConfigStrings( const char * path, const char * tag ) :
	AutoConfig(),
	path_( path ),
	tag_( tag )
{
}


/**
 *	Configure yourself
 */
void AutoConfigStrings::configureSelf( DataSectionPtr pConfigSection )
{
	DataSectionPtr pSect = pConfigSection->openSection( path_ );
	if (pSect) pSect->readStrings(tag_,value_);
}

// auto_config.cpp
