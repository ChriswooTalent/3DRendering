#include "stdafx.h"
#include "Volumefiltering.h"
#include <iostream> 
#include <fstream> 
#include <sstream>
 
using namespace std;

//////////////////////////////////////////////////////////////////////////
// 3D FILTER

static float filteroffsets[3 * 3 * 3][3] =
{
	{ -1, -1, -1 }, { 0, -1, -1 }, { 1, -1, -1 },
	{ -1, 0, -1 }, { 0, 0, -1 }, { 1, 0, -1 },
	{ -1, 1, -1 }, { 0, 1, -1 }, { 1, 1, -1 },

	{ -1, -1, 0 }, { 0, -1, 0 }, { 1, -1, 0 },
	{ -1, 0, 0 }, { 0, 0, 0 }, { 1, 0, 0 },
	{ -1, 1, 0 }, { 0, 1, 0 }, { 1, 1, 0 },

	{ -1, -1, 1 }, { 0, -1, 1 }, { 1, -1, 1 },
	{ -1, 0, 1 }, { 0, 0, 1 }, { 1, 0, 1 },
	{ -1, 1, 1 }, { 0, 1, 1 }, { 1, 1, 1 },
};
static float filterblur[3 * 3 * 3] =
{
	0, 1, 0,
	1, 2, 1,
	0, 1, 0,

	1, 2, 1,
	2, 4, 2,
	1, 2, 1,

	0, 1, 0,
	1, 2, 1,
	0, 1, 0,
};
static float filtersharpen[3 * 3 * 3] =
{
	0, 0, 0,
	0, -2, 0,
	0, 0, 0,

	0, -2, 0,
	-2, 15, -2,
	0, -2, 0,

	0, 0, 0,
	0, -2, 0,
	0, 0, 0,
};
static float filterpassthru[3 * 3 * 3] =
{
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,

	0, 0, 0,
	0, 1, 0,
	0, 0, 0,

	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
};

enum VoFilterKernelMethods
{
	VOLUMEFILTERBUF = 0,
	VOLUMEFILTER = 1,
	VOLUMEFILTERKERNELTEX = 2
};

static const char *VoFilterKernelNames[] =
{
	"VolumeFilterBuf",
	"VolumeFilter",
	"VolumeFilterKernelTex"
};

VolumeFiler::VolumeFiler()
{
	m_Width = WIDTH;
	m_Height = HEIGHT;
	m_SliceNum = NSLICE;
	m_FilterSize = 0;
	m_KernelCount = sizeof(VoFilterKernelNames) / sizeof(char *);
	m_kernelFile = (char *)VoFilterKernelFile;
	m_DFilterKernel = 0;
	for (int i = 0; i < 4*27; i++)
	{
		m_HFilterKernel[i] = 0.0f;
	}
	d_DFilterArray = 0;
	DFilterSampler = 0;
	volumeSamplerLinear = 0;
}

VolumeFiler::~VolumeFiler()
{
}


void VolumeFiler::VolumeFilterOpenCLini(cl_context cxGPUContext, cl_command_queue cqCommandQueue, cl_device_id device)
{
	m_Context = cxGPUContext;
	m_Queue = cqCommandQueue;
	m_Device = device;
	BuildKernel(VoFilterKernelNames);
}

void VolumeFiler::VolumeFilter(cl_mem d_in, cl_mem d_out)
{
	int totalpts = m_Width*m_Height*m_SliceNum;
	int threads = THREAD;
	dim3 grid(totalpts, 1, 1);
	dim3 threadsd(threads, 1, 1);
	VolumeFilter_runFilterCLBuf(grid, threadsd, d_in, d_out, m_DFilterKernel, GridSize, m_FilterSize);
}

void VolumeFiler::VolumeFilterTex(cl_mem d_inArray, cl_mem d_out)
{
	int totalpts = m_Width*m_Height*m_SliceNum;
	int threads = THREAD;
	dim3 grid(totalpts, 1, 1);
	dim3 threadsd(threads, 1, 1);
	VolumeFilter_runFilter(grid, threadsd, d_inArray, d_out, m_DFilterKernel, GridSize, m_FilterSize, volumeSamplerLinear);
}

void VolumeFiler::VolumeFilterKernelTex(cl_mem d_inArray, cl_mem d_out)
{
	int totalpts = m_Width*m_Height*m_SliceNum;
	int threads = THREAD;
	dim3 grid(totalpts, 1, 1);
	dim3 threadsd(threads, 1, 1);
	VolumeFilter_runFilterKernelTex(grid, threadsd, d_inArray, d_out, d_DFilterArray, GridSize, m_FilterSize, volumeSamplerLinear, DFilterSampler);
}

void VolumeFiler::VolumeFilterTex3D(cl_mem d_inArray, cl_mem d_out)
{
	int totalpts = m_Width*m_Height*m_SliceNum;
	int threads = THREAD;
	dim3 grid(m_Width, m_Height, m_SliceNum);
	dim3 threadsd(BLOCKX, BLOCKY, 1);
	VolumeFilter_runFilter(grid, threadsd, d_inArray, d_out, m_DFilterKernel, GridSize, m_FilterSize, volumeSamplerLinear);
}

void VolumeFiler::VolumeFilterKernelConfiguration(float blurfactor)
{
	float sumblur = 0.0f;
	float sumsharpen = 0.0f;
	m_FilterSize = 27;

	for (int i = 0; i < 3 * 3 * 3; i++)
	{
		sumblur += filterblur[i];
		sumsharpen += filtersharpen[i];
	}

	for (int i = 0; i < 3 * 3 * 3; i++)
	{
		filterblur[i] /= sumblur;
		filtersharpen[i] /= sumsharpen;

		m_HFilterKernel[i*4] = filteroffsets[i][0];
		m_HFilterKernel[i * 4 + 1] = filteroffsets[i][1];
		m_HFilterKernel[i * 4 + 2] = filteroffsets[i][2];
		m_HFilterKernel[i * 4 + 3] = filterblur[i] * blurfactor + filterpassthru[i] * (1.0f - blurfactor);
	}
}

void VolumeFiler::InitVolumeFilter(int width, int height, int slicenum, float blurfactor)
{
	/*GridSize[0] = width;
	GridSize[1] = height;
	GridSize[2] = slicenum;
	GridSize[3] = 0;
	m_Width = GridSize[0];
	m_Height = GridSize[1];
	m_SliceNum = GridSize[2];

	VolumeFilterKernelConfiguration(blurfactor);

	m_DFilterKernel = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, 27 * 4 * sizeof(float), 0, &error);
	check_CL_error(error, "Create Device Buffer for FilterKernel Failed");
	error = clEnqueueWriteBuffer(m_Queue, m_DFilterKernel, CL_TRUE, 0, 27 * 4 * sizeof(float),m_HFilterKernel, 0, 0, NULL);
	check_CL_error(error, "Init m_DFilterKernel failed");
	cl_image_format transferFunc_format;
	transferFunc_format.image_channel_order = CL_RGBA;
	transferFunc_format.image_channel_data_type = CL_FLOAT;
	d_DFilterArray = clCreateImage2D(m_Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &transferFunc_format,
		27, 1, sizeof(float) * 27 * 4,
		m_HFilterKernel, &error);
	check_CL_error(error, "Create 2D Texture for FilterKernel Failed");

	// Create samplers for FilterKernel, linear interpolation and nearest interpolation 
	DFilterSampler = clCreateSampler(m_Context, true, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR, &error);
	check_CL_error(error, "Create DFilterSampler Failed");
	volumeSamplerLinear = clCreateSampler(m_Context, true, CL_ADDRESS_REPEAT, CL_FILTER_LINEAR, &error);
	check_CL_error(error, "Create volumeSamplerLinear Failed");*/
}

void VolumeFiler::VolumeFilter_runFilterCLBuf(dim3 grid, dim3 threads, cl_mem d_volume, cl_mem d_out, cl_mem fkernel, cl_uint gridSize[4], cl_uint filtersize)
{
	unsigned int k = VOLUMEFILTERBUF;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&d_out);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&d_volume);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&fkernel);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), gridSize);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &filtersize);
	check_CL_error(error, "Setting VolumeFilter_runFilterCLBuf failed");

	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t *)&grid, (size_t *)&threads, 0, 0, 0);
	check_CL_error(error, "VolumeFilter_runFilterCLBuf failed!");
}

void VolumeFiler::VolumeFilter_runFilter(dim3 grid, dim3 threads, cl_mem d_volumearray, cl_mem d_out, cl_mem fkernel, cl_uint gridSize[4], cl_uint filtersize,
	cl_sampler volumeSampler)
{
	unsigned int k = VOLUMEFILTER;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &d_out);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &d_volumearray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_sampler), &volumeSampler);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &fkernel);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), gridSize);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &filtersize);
	check_CL_error(error, "Setting VolumeFilter_runFilter failed");

	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t *)&grid, (size_t *)&threads, 0, 0, 0);
	check_CL_error(error, "VolumeFilter_runFilter failed!");
}

void VolumeFiler::VolumeFilter_runFilterKernelTex(dim3 grid, dim3 threads, cl_mem d_volumearray, cl_mem d_out, cl_mem fkernelArray, cl_uint gridSize[4], cl_uint filtersize,
	cl_sampler volumeSampler, cl_sampler kernelsampler)
{
	unsigned int k = VOLUMEFILTERKERNELTEX;
	unsigned int a = 0;

	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&d_out);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&d_volumearray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_sampler), &volumeSampler);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&fkernelArray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_sampler), &kernelsampler);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), gridSize);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &filtersize);
	check_CL_error(error, "Setting VolumeFilter_runFilter failed");

	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t *)&grid, (size_t *)&threads, 0, 0, 0);
	check_CL_error(error, "VolumeFilter_runFilter failed!");
}