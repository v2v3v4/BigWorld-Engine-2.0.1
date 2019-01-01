/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MODULE_MANAGER_HPP
#define MODULE_MANAGER_HPP

#include <iostream>
#include <stack>

#include "factory.hpp"
#include "cstdmf/smartpointer.hpp"

class Module;
typedef SmartPointer< class Module> ModulePtr;

/**
 * The ModuleManager class manages a stack of application modules,
 * and encapsulates the logical requirements of said stack.
 */

class ModuleManager : public Factory<Module>
{
public:
	ModuleManager();
	~ModuleManager();

	static ModuleManager& instance();
	static void fini();
	void	popAll();

	bool	push( const std::string & moduleIdentifier );
	void	push( ModulePtr module );
	ModulePtr currentModule();
	void	pop();

private:
	ModuleManager(const ModuleManager&);
	ModuleManager& operator=(const ModuleManager&);

	// Modules
	typedef std::stack< ModulePtr >	ModuleStack;
	ModuleStack			modules_;

	friend std::ostream& operator<<(std::ostream&, const ModuleManager&);
};

#ifdef CODE_INLINE
#include "module_manager.ipp"
#endif




#endif
/*module_manager.hpp*/
