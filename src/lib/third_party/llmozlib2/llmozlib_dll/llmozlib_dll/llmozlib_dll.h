#ifndef LLMOZLIB_DLL_
#define LLMOZLIB_DLL_

#ifdef FROM_WITHIN
#define DECLSPEC __declspec (dllexport)
#else
#define DECLSPEC __declspec (dllimport)
#endif

extern "C" 
{
DECLSPEC LLMozlibVirtualWrapper* getInstance();

DECLSPEC void deleteInstance();
}

#endif
