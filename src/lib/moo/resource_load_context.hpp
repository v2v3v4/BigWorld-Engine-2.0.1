/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RESOURCE_LOAD_CONTEXT_HPP
#define RESOURCE_LOAD_CONTEXT_HPP


namespace Moo
{

/**
 *	This class keeps track of the resource currently being loaded, useful for
 *	when reporting errors for example.
 */
class ResourceLoadContext
{
public:
	static void push( const std::string & requester );
	static void pop();

	static std::string formattedRequesterString();
};


/**
 *	This class allows for easy scoped setup of a resource load context.
 */
class ScopedResourceLoadContext
{
public:
	ScopedResourceLoadContext( const std::string & requester )
	{
		ResourceLoadContext::push( requester );
	}
	~ScopedResourceLoadContext()
	{
		ResourceLoadContext::pop();
	}
};


} // namespace Moo


#endif // RESOURCE_LOAD_CONTEXT_HPP
