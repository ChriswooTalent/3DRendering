#ifndef  _SCAN_APPLE_H_
#define  _SCAN_APPLE_H_

//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <malloc.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <assert.h>

#include <CL/opencl.h>
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_INFO      (0)
#define NUM_BANKS       (16)
#define MAX_ERROR       (1e-7)
#define SEPARATOR       ("----------------------------------------------------------------------\n")

#define min(A,B) ((A) < (B) ? (A) : (B))

//static int iterations = 1000;

inline void clCheckErrorIP(cl_int iSample, cl_int iReference) 
{
	// An error condition is defined by the sample/test value not equal to the reference
	if (iReference != iSample)
	{
		assert(iSample == iReference);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Shortcut typenames
////////////////////////////////////////////////////////////////////////////////
typedef cl_uint uint;

////////////////////////////////////////////////////////////////////////////////
// OpenCL scan
////////////////////////////////////////////////////////////////////////////////
extern "C" void InitScanAPPLEMem(int Ccount);
extern "C" int initScanAPPLE(cl_context cxGPUContext, cl_command_queue cqParamCommandQue, cl_device_id device);
extern "C" void closeScanAPPLE(void);
extern "C" void ScanAPPLEProcess(cl_mem d_Dst, cl_mem d_Src, int Ccount);

#endif // !_SCAN_APPLE_H_