#ifndef _VOLUMERENDERING_H_
#define _VOLUMERENDERING_H_
#include "RenderBase.h"

#include "DataTypes.h"
#include "Rotateslice.h"
#ifdef RENDERLIB_EXPORTS  
#define VOLUMERENDERLIB_API __declspec(dllexport)  
#else  
#define VOLUMERENDERLIB_API __declspec(dllimport)  
#endif  
   
const char VRKernelFile[256] = "D:/CarbonMed/Config/Local/CL/VolumeRender_kernel.cl";
 
class VOLUMERENDERLIB_API VolumeRender : public RenderBase
{
public:
	VolumeRender();
	~VolumeRender();

	//opencl
	void KernelInit();
	//opengl buffer
	int OpenGLBufferInit();
	//texture
	void TextureInit();
	//compute
	void Volumecomputing();
	//rendering
	void RenderProcess();

	//Rendering Feature:alpha value
	void SetRenderingFeature(RenderParam param);
	void GetRenderingFeature(RenderParam &param);

	//Rendering TexSlice
	void RenderTexSlice(GLuint tex2d, int orderflag);

	//animation
	void animation();

	//keyboard
	void KeyBoard();

	//processing
	void RayCastingProcessing();

	//rendering
	void RayCastingRendering();
	void GLDataRendering(cl_float translate[4], cl_float clrotate[4], int orderflag);

	//3D Texture Testing
	void Testing3DTex();

	//2D Texture Testing
	void Testing2DTex();

	void VolumeOpenCLRelease();
private:
	//kernel function
	void RayCasting3DTexture(dim3 grid, dim3 threads, cl_mem pbocl, cl_uint gridSize[4], float density, float brightness, float transferOffset, float transferScale,
		cl_mem invMatrix, cl_mem d_volumearray, cl_mem transferfuncarray, cl_sampler volumeSampler, cl_sampler transfersampler);

	void RayCasting3DTextureBTF(dim3 grid, dim3 threads, cl_mem pbocl, cl_uint gridSize[4], float density, float brightness, float transferOffset, float transferScale,
		cl_mem invMatrix, cl_mem d_volumearray, cl_mem transferfuncarray, cl_sampler volumeSampler, cl_sampler transfersampler);

	void RayCasting3DMem(dim3 grid, dim3 threads, cl_mem pbocl, cl_uint gridSize[4], float density, float brightness, float transferOffset, float transferScale,
		cl_mem invMatrix, cl_mem volume, cl_mem transferbuf);

	void Texture3dTesting(dim3 grid, dim3 threads, cl_mem volumetex, cl_sampler volumeSampler, cl_mem d_out, cl_uint imageW, cl_uint imageH, cl_uint Nslice);
	void Texture2dTesting(dim3 grid, dim3 threads, cl_mem imagetex, cl_sampler transferSampler, cl_mem d_out, cl_uint length);
private:
	int m_GroupSize;
	//invViewMatrix
	float m_InvViewMatrix[12];

	//rendering param
	float m_Density;
	float m_Brightness;
	float m_TransferOffset;
	float m_TransferScale;
	bool m_LinearFiltering;

	//openGL pixel buffer object
	GLuint m_Pbo;

	//opencl buffer
	//pbo
	cl_mem m_PboCL;
	//volumeArray and transferfunc
	cl_mem d_volumeArray;
	cl_mem d_alphaBuf;
	cl_mem d_densityBuf;
	cl_mem d_brightnessBuf;
	cl_mem d_transferFuncArray;
	cl_mem d_transferFuncBuf;
	//modeviewMatrix
	cl_mem d_invViewMatrix;

	//texture related
	cl_bool g_bImageSupport;
	cl_sampler volumeSamplerLinear;
	cl_sampler volumeSamplerNearest;
	cl_sampler transferFuncSampler;
};
#endif // !_VOLUMERENDERING_H_
