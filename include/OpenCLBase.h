//  OpenCLBase:  
//  A Class that contain the basic process of OpenCL, such as the
//  Initialization of the platform, device, program, command_queue and context
//  In OpenCL development
//  context for allocating of the memory
//  command_queue for the reading and writing of the memory
//  program for the compiling of the kernel file
//  often call by other models as the interface  
//  Created by Chriswoo on 2017-08-02.
//  Contact: wsscx01@aliyun.com
// 
#ifndef _OPENCLBASE_H_
#define _OPENCLBASE_H_
#include "stdafx.h"
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <string>
#include <sys/stat.h>
#include <fstream> 
#include <sstream>
#include <wingdi.h>
#include <math.h>
using namespace std;

// PrintLog
extern "C" __declspec(dllimport) void OpenLog();
extern "C" __declspec(dllimport) void CloseLog();
extern "C" __declspec(dllimport) void PrintLog(const char * sysmessage);
class OpenCLBase
{
public:
	OpenCLBase();
	virtual ~OpenCLBase(){}
	virtual void KernelInit()=0;

	void BuildKernel(const char**kernelnames);
	void InitCL(string gpuinfo);
	void OpenCLini(const char *gpuinfo);
	cl_context CreateContext(cl_device_id &devicechosen, char *gpucardinfo);
	char *LoadProgramSourceFromFile(const char *filename);
	void OpenCLRelease();

	//AMDFLag
	void SetAMDFlag(bool flag)
	{
		m_AMDFlag = flag;
	}

	string GetKernelFileNames()
	{
		return m_kernelFile;
	}
	int GetKernelCount()
	{
		return m_KernelCount;
	}
protected:
	inline void GetGPURunTime();
	inline void PrintLogIN(const char * runningmessage);
	inline void check_CL_error(cl_int error, char* error_messgae);
protected:
	cl_platform_id	*m_Platforms;
	cl_device_id	 m_Device;
	cl_context		 m_Context;
	cl_command_queue m_Queue;
	cl_program       m_Program;
	cl_kernel       *m_ComputeKernels;
	string           m_kernelFile;
	int m_KernelCount;
	//AMD flag
	bool m_AMDFlag;
	// OpenCL error
	cl_int error;
	// Kernel running time
	cl_ulong startTime, endTime;
	cl_double kernelExecTimeNs;
	// OpenCL event
	cl_event ndrEvt;
	//GPU group info
	// GPU dim 1D
	size_t GlobalWorkSize;
	size_t LocalWorkSize;

	// GPU dim 2D
	size_t _GlobalWorkSize[2];  // global image size
	size_t _LocalWorkSize[2];   // thread num in one work group
};
#endif // !_OPENCLBASE_H_
