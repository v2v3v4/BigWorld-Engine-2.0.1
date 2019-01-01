/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	FILTER_SPEC: filters text according to its include/exclude rules
 */


#ifndef FILTER_SPEC_HPP
#define FILTER_SPEC_HPP

#include "cstdmf/smartpointer.hpp"


/**
 *	This class stores information about a single list filter.
 */
class FilterSpec : public ReferenceCount
{
public:
	FilterSpec( const std::wstring& name, bool active = false,
		const std::wstring& include = L"", const std::wstring& exclude = L"",
		const std::wstring& group = L"" );
	virtual ~FilterSpec();

	// Getters and setters
	virtual std::wstring getName() { return name_; };
	virtual void setActive( bool active ) { active_ = active; };
	virtual bool getActive() { return active_ && enabled_; };
	virtual std::wstring getGroup() { return group_; };

	virtual bool filter( const std::wstring& str );

	void enable( bool enable );

private:
	std::wstring name_;
	bool active_;
	bool enabled_;
	std::vector<std::wstring> includes_;
	std::vector<std::wstring> excludes_;
	std::wstring group_;
};
typedef SmartPointer<FilterSpec> FilterSpecPtr;


#endif // FILTER_SPEC_HPP
