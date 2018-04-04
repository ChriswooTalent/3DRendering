#ifndef _PRINTLOG_DLL_H_
#define _PRINTLOG_DLL_H_

#define PRINTLOG_EXPORTS
#ifdef  PRINTLOG_EXPORTS
#define PRINTLOG_API extern "C" __declspec(dllexport) 
#else
#define PRINTLOG_API extern "C" __declspec(dllimport) 
#endif  // PRINTLOG_EXPORTS

#endif	//_PRINTLOG_DLL_H_