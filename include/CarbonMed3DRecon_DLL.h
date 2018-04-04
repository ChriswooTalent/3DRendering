#ifndef _CARBONMED3DRECON_DLL_H_
#define _CARBONMED3DRECON_DLL_H_
#include "stdafx.h"

#ifdef  RENDERLIB_EXPORTS
#define CARBONMED3DRECON_API extern "C" __declspec(dllexport) 
#else
#define CARBONMED3DRECON_API extern "C" __declspec(dllimport) 
#endif  // CARBONMED3DRECON_EXPORTS

#endif // #ifndef _CARBONMED3DRECON_DLL_H_
