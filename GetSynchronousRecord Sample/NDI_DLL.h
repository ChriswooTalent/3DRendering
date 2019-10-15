#ifndef _NDI_DLL_H_
#define _NDI_DLL_H_

#define NDIDRIVEVAY_EXPORTS
#ifdef NDIDRIVEVAY_EXPORTS
#define NDIDRIVEVAY_API  extern "C" __declspec(dllexport)
#else
#define NDIDRIVEVAY_API  extern "C" __declspec(dllimport)
#endif

#endif