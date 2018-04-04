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
#include "OpenCLBase.h"
#include <assert.h>

#pragma warning( disable : 4996 )

OpenCLBase::OpenCLBase()
{
	m_KernelCount = 0;
	m_kernelFile = "";
	m_AMDFlag = false;
	//OpenCL Engine
	m_Platforms = 0;
	m_Context = 0;
	m_Device = 0;
	m_Queue = 0;
	m_Program = 0;
	m_ComputeKernels = NULL;
	error = 0;
	startTime = 0;
	endTime = 0;
	kernelExecTimeNs = 0.0;
	ndrEvt = 0;
	LocalWorkSize = 0;
	GlobalWorkSize = 0;
	_LocalWorkSize[0] = 0;
	_LocalWorkSize[1] = 0;
	_GlobalWorkSize[0] = 0;
	_GlobalWorkSize[1] = 0;
}

void OpenCLBase::BuildKernel(const char **kernelnames)
{
	char programlog[256] = {0};
	sprintf(programlog, "Loading program %s...", m_kernelFile.c_str());
	PrintLogIN(programlog);

	char *source = LoadProgramSourceFromFile(m_kernelFile.c_str());
	if (!source)
	{
		check_CL_error(-1, "Error: Failed to load compute program from file");
	}

	// Create the compute program from the source buffer
	// load CL file
	std::ifstream kernelFile(m_kernelFile.c_str(), std::ios::in);
	if (!kernelFile.is_open())
	{ 
		check_CL_error(-1, "Opening CL file failed");
	}
	ostringstream oss;
	oss << kernelFile.rdbuf();
	string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	size_t src_size = srcStdStr.length();
	m_Program = clCreateProgramWithSource(m_Context, 1, &srcStr, &src_size, &error);

	if (!m_Program || error != CL_SUCCESS)
	{
		check_CL_error(error, source);
		check_CL_error(error, "Error: Failed to create compute program");
	}

	// Build the program executable
	//
	error = clBuildProgram(m_Program, 1, &m_Device, NULL, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		size_t length;
		char build_log[2048];
		
		clGetProgramBuildInfo(m_Program, m_Device, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, &length);
		printf("%s\n", build_log);
		/*check_CL_error(error, source);
		check_CL_error(-1, "Error: Failed to build program executable");
		check_CL_error(-1, build_log);*/
	}

	m_ComputeKernels = (cl_kernel*)malloc(m_KernelCount * sizeof(cl_kernel));
	for (int i = 0; i < m_KernelCount; i++)
	{
		// Create each compute kernel from within the program
		//
		char *Kernellog = NULL;
		m_ComputeKernels[i] = clCreateKernel(m_Program, kernelnames[i], &error);
		if (!m_ComputeKernels[i] || error != CL_SUCCESS)
		{
			sprintf(Kernellog, "Error: Failed to create compute kernel %s", kernelnames[i]);
			PrintLogIN(Kernellog);
		}

		size_t wgSize;
		error = clGetKernelWorkGroupInfo(m_ComputeKernels[i], m_Device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &wgSize, NULL);
		check_CL_error(error, "Error: Failed to get kernel work group size");
	}
	free(source);
}

void OpenCLBase::InitCL(string gpuinfo)
{
	OpenCLini(gpuinfo.c_str());
}

void OpenCLBase::OpenCLini(const char *gpuinfo)
{
	//////////////////////////////// initialize the openCL elements //////////////////////////////////////////////
	error = 0;
	kernelExecTimeNs = 0;

	cl_device_id devicechosen = NULL;
	char gpucardinfo[256] = { 0 };
	sprintf_s(gpucardinfo, sizeof(gpucardinfo), gpuinfo);
	//////////////////////////////////////////////////////////////////////////
	// 创建设备上下文(context)
	if (!m_Context)
	{
		m_Context = CreateContext(m_Device, gpucardinfo);
		if (m_Context == NULL)
		{
			printf("create context failed!\n");
			return;
		}
	}

	// Command-queue
	m_Queue = clCreateCommandQueue(m_Context, m_Device, CL_QUEUE_PROFILING_ENABLE, &error);
	check_CL_error(error, "Creating command queue failed");
}

// 创建上下文并返回
cl_context OpenCLBase::CreateContext(cl_device_id &devicechosen, char *gpucardinfo)
{
	cl_uint numPlatforms;
	cl_uint num_devices;
	cl_platform_id platformchosen = 0;
	cl_device_id *devicesin = 0;
	cl_context contextinside = NULL;

	char name_data[48];
	error = clGetPlatformIDs(1, NULL, &numPlatforms);

	if ((error != CL_SUCCESS) || (numPlatforms <= 0))
	{
		return NULL;
	}
	/* 选取所有的platforms*/
	m_Platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id)* numPlatforms);
	error = clGetPlatformIDs(numPlatforms, m_Platforms, NULL);
	check_CL_error(error, "Couldn't find any platforms");
	for (int i = 0; i < (int)numPlatforms; i++)
	{
		if (m_AMDFlag)
		{
			error = clGetDeviceIDs(m_Platforms[i], CL_DEVICE_TYPE_GPU, 0, 0, &num_devices);
		}
		else
		{
			error = clGetDeviceIDs(m_Platforms[i], CL_DEVICE_TYPE_ALL, 1, NULL, &num_devices);
		}
		check_CL_error(error, "Couldn't find any devices");

		/* Access connected devices */
		devicesin = (cl_device_id*)
			malloc(sizeof(cl_device_id)* num_devices);
		clGetDeviceIDs(m_Platforms[i], CL_DEVICE_TYPE_ALL,
			num_devices, devicesin, NULL);
		//printf("num of devices is %d\n", num_devices);

		/*循环显示platform的所有device（CPU和显卡）信息。*/
		for (int j = 0; j < (int)num_devices; j++) {

			error = clGetDeviceInfo(devicesin[j], CL_DEVICE_NAME,
				sizeof(name_data), name_data, NULL);
			check_CL_error(error, "Couldn't read extension data");

			if (strcmp(name_data, gpucardinfo) == 0)
			{
				platformchosen = m_Platforms[i];
				devicechosen = devicesin[j];
			}
		}
	}

	cl_context_properties properties[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), //获得OpenGL上下文
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), //获得OpenGl设备信息
		CL_CONTEXT_PLATFORM, (cl_context_properties)platformchosen, //获得平台信息
		0 };
	//利用刚刚创建的属性创建上下文
	contextinside = clCreateContext(properties, 1, &devicechosen, NULL, NULL, &error);
	check_CL_error(error, "CreateContext failed");
	free(devicesin);
	return contextinside;
}

char *OpenCLBase::LoadProgramSourceFromFile(const char *filename)
{
	struct stat statbuf;
	FILE        *fh = 0;
	char        *source;

	fopen_s(&fh, filename, "r");
	if (fh == 0)
		return 0;

	stat(filename, &statbuf);
	source = (char *)malloc(statbuf.st_size + 1);
	fread(source, statbuf.st_size, 1, fh);
	source[statbuf.st_size] = '\0';

	return source;
}

void OpenCLBase::OpenCLRelease()
{
	delete[] m_Platforms;
	m_Platforms = NULL;
	m_Device = NULL;
	if (m_Context)
	{
		clReleaseContext(m_Context);
	}
	if (m_Program)
	{
		clReleaseProgram(m_Program);
	}

	for (int i = 0; i < m_KernelCount; i++)
	{
		if (m_ComputeKernels[i])
		{
			clReleaseKernel(m_ComputeKernels[i]);
		}
	}
	free(m_ComputeKernels);
	m_ComputeKernels = NULL;
}

inline void OpenCLBase::GetGPURunTime()
{
	error = clWaitForEvents(1, &ndrEvt);
	check_CL_error(error, "kernel run wait for event");
	clGetEventProfilingInfo(ndrEvt, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL);
	clGetEventProfilingInfo(ndrEvt, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL);
	kernelExecTimeNs += cl_double(endTime - startTime);
}

inline void OpenCLBase::PrintLogIN(const char * _RunningMessage)
{
	PrintLog(_RunningMessage);
}

inline void OpenCLBase::check_CL_error(cl_int error, char* error_messgae)
{
	if (error != CL_SUCCESS){
		PrintLogIN(error_messgae);
		//assert(0);
		exit(error);
	}
}