#include "../../llmozlib_virtual_wrapper.h"
#define FROM_WITHIN
#include "llmozlib_dll.h"

static LLMozlibVirtualWrapper* pObj = NULL;

extern "C" LLMozlibVirtualWrapper* getInstance()
{
	if (pObj == NULL)
	{
		pObj = new LLMozlibVirtualWrapper;
	}
	return pObj;
}

extern "C" void deleteInstance()
{
	delete pObj;
	pObj = NULL;
}

