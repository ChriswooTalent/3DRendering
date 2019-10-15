#ifndef _VOLUMEFILTERING_H_
#define _VOLUMEFILTERING_H_

#include "DataTypes.h"
#include "OpenCLBase.h"
#include <stdlib.h>
#include <sys/types.h> 
using namespace std;
using namespace RenderingDataTypes;

const char VoFilterKernelFile[256] = "D:/CarbonMed/Config/Local/CL/VolumeFilter_kernel.cl";
class VolumeFiler:public OpenCLBase
{
public:
	VolumeFiler();
	~VolumeFiler();

	void Filter();
	void KernelInit(){}

	//opencl相关的初始化
	void VolumeFilterOpenCLini(cl_context cxGPUContext, cl_command_queue cqCommandQueue, cl_device_id device);

	//filtering process
	//interface
	void VolumeFilter(cl_mem d_in, cl_mem d_out);
	void VolumeFilterTex(cl_mem d_inArray, cl_mem d_out);
	void VolumeFilterKernelTex(cl_mem d_inArray, cl_mem d_out);
	void VolumeFilterTex3D(cl_mem d_inArray, cl_mem d_out);

	//初始化切片数据
	void InitVolumeFilter(int width, int height, int slicenum, float blurfactor);
private:
	void VolumeFilter_runFilterCLBuf(dim3 grid, dim3 threads, cl_mem d_volume, cl_mem d_out, cl_mem fkernel, cl_uint gridSize[4], cl_uint filtersize);

	void VolumeFilter_runFilter(dim3 grid, dim3 threads, cl_mem d_volumearray, cl_mem d_out, cl_mem fkernel, cl_uint gridSize[4], cl_uint filtersize,
                                cl_sampler volumeSampler);

	void VolumeFilter_runFilterKernelTex(dim3 grid, dim3 threads, cl_mem d_volumearray, cl_mem d_out, cl_mem fkernelArray, cl_uint gridSize[4], cl_uint filtersize,
		                        cl_sampler volumeSampler, cl_sampler kernelsampler);
	//平滑因子，越大平滑程度越大
	void VolumeFilterKernelConfiguration(float blurfactor);
private:
	int m_Width;
	int m_Height;
	int m_SliceNum;
	int m_FilterSize;

	//gridsize
	cl_uint GridSize[4];

	// Initial OpenCL component
	int                     m_GroupSize;

	//filter kernel on device
	cl_mem m_DFilterKernel;
	cl_mem d_DFilterArray;

	cl_sampler DFilterSampler;
	cl_sampler volumeSamplerLinear;
	float  m_HFilterKernel[4*27];
protected:
	// OpenlCL error
	cl_int error;
};

#endif
