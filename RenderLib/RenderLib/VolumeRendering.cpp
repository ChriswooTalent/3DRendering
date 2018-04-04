#include "VolumeRendering.h"
#include <iostream> 
using namespace std;
           
enum VolumeKernelMethods
{
	RAYCASTING3DTex = 0,
	RAYCASTINGMEMORY = 1,
	RAYCASTING3DTexBTF = 2,
	TESTRW3DTEXTURE = 3,
	TESTRW2DTEXTURE = 4,
};
 
static const char* VolumeKernelNames[] =
{
	"d_render3DTexture",
	"d_renderMemory",
	"d_render3DTextureBTF",
	"TestRW3DTexture",
	"TestRW2DTexture",
};

VolumeRender::VolumeRender()
{
	m_KernelCount = sizeof(VolumeKernelNames) / sizeof(char *);
	m_kernelFile = (char *)VRKernelFile;
	m_Pbo = 0;
	d_alphaBuf = 0;
	d_densityBuf = 0;
	d_brightnessBuf = 0;
	m_Density = 0.0f;
	m_Brightness = 0.0f;
	m_TransferOffset = 0.0f;
	m_TransferScale = 0.0f;
	m_LinearFiltering = true;
	for (int i = 0; i < 12; i++)
	{
		m_InvViewMatrix[i] = 0.0f;
	}
}

VolumeRender::~VolumeRender()
{
	VolumeOpenCLRelease();
}

void VolumeRender::VolumeOpenCLRelease()
{
	if (volumeSamplerLinear)
	{
		clReleaseSampler(volumeSamplerLinear);
	}
	if (volumeSamplerNearest)
	{
		clReleaseSampler(volumeSamplerNearest);
	}
	if (transferFuncSampler)
	{
		clReleaseSampler(transferFuncSampler);
	}
	if (d_volumeArray)
	{
		clReleaseMemObject(d_volumeArray);
	}
	if (d_transferFuncArray)
	{
		clReleaseMemObject(d_transferFuncArray);
	}
	if (d_alphaBuf)
	{
		clReleaseMemObject(d_alphaBuf);
	}
	if (d_densityBuf)
	{
		clReleaseMemObject(d_densityBuf);
	}
	if (d_brightnessBuf)
	{
		clReleaseMemObject(d_brightnessBuf);
	}
	if (d_invViewMatrix)
	{
		clReleaseMemObject(d_invViewMatrix);
	}
	if (m_Pbo) 
	{
		// delete old buffer
		clReleaseMemObject(m_PboCL);
		glDeleteBuffersARB(1, &m_Pbo);
	}
}

void VolumeRender::SetRenderingFeature(RenderParam param)
{
	m_Density = param.Density;
	m_Brightness = param.Brightness;
	m_TransferOffset = param.TransferOffset;
	m_TransferScale = param.TransferScale;
}
void VolumeRender::GetRenderingFeature(RenderParam &param)
{
	param.Density = m_Density;
	param.Brightness = m_Brightness;
	param.TransferOffset = m_TransferOffset;
	param.TransferScale = m_TransferScale;
}

void VolumeRender::KeyBoard()
{}

void VolumeRender::GLDataRendering(cl_float translate[4], cl_float clrotate[4], int orderflag)
{
	GLfloat modelView[16];
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRotatef(-clrotate[0], 1.0, 0.0, 0.0);
	glRotatef(-clrotate[1], 0.0, 1.0, 0.0);
	glTranslatef(-translate[0], -translate[1], -translate[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
	glPopMatrix();

	SetModelView(modelView);
	
	if (m_ComputeFlag) 
	{
		Volumecomputing();
		m_ComputeFlag = false;
	}
	// display results
	glClear(GL_COLOR_BUFFER_BIT);

	// draw image from PBO
	glDisable(GL_DEPTH_TEST);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// draw using glDrawPixels (slower)
	glRasterPos2i(0, 0);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_Pbo);
	glDrawPixels(m_RenderWidth, m_RenderHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);


	glutSwapBuffers();
	glutReportErrors();
}


void VolumeRender::RenderTexSlice(GLuint tex2d, int orderflag)
{
}

//animation
void VolumeRender::animation()
{

}

void VolumeRender::TextureInit()
{
	/*if (g_bImageSupport)
	{
		// create 3D array and copy data to device
		cl_image_format volume_format;
		volume_format.image_channel_order = CL_RGBA;
		volume_format.image_channel_data_type = CL_SNORM_INT16;
		short* h_tempVolume = (short*)malloc(GridSize[0] * GridSize[1] * GridSize[2] * sizeof(short)*4);
		memset(h_tempVolume, 0, GridSize[0] * GridSize[1] * GridSize[2] * sizeof(short)* 4);
		for (int i = 0; i < (int)(GridSize[0] * GridSize[1] * GridSize[2]); i++)
		{
			h_tempVolume[4 * i] = m_HostVolume[i];
		}
		d_volumeArray = clCreateImage3D(m_Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &volume_format,
			GridSize[0], GridSize[1], GridSize[2],
			(GridSize[0] * sizeof(short) * 4), (GridSize[0] * GridSize[1] * sizeof(short) * 4),
			h_tempVolume, &error);
		check_CL_error(error, "Create 3D Texture Failed");
		free(h_tempVolume);

		int maxsize = GridSize[0] * GridSize[1] * GridSize[2] * sizeof(float);

		d_alphaBuf = clCreateBuffer(m_Context, CL_MEM_READ_ONLY, maxsize, 0, &error);
		check_CL_error(error, "Create Alpha Buffer Failed!");
		d_densityBuf = clCreateBuffer(m_Context, CL_MEM_READ_ONLY, maxsize, 0, &error);
		check_CL_error(error, "Create Density Buffer Failed!");
		d_brightnessBuf = clCreateBuffer(m_Context, CL_MEM_READ_ONLY, maxsize, 0, &error);
		check_CL_error(error, "Create Brightness Buffer Failed!");

		// create transfer function texture
		float transferFunc[] = {
			0.0f, 0.0f, 0.0f, 0.0f,
			0.5f, 0.0f, 0.0f, 0.5f,
			0.0f, 0.5f, 0.0f, 0.5f,
			0.0f, 0.0f, 0.5f, 0.5f,
			1.0f, 0.0f, 0.0f, 0.6f,
			0.0f, 1.0f, 0.0f, 0.6f,
			0.0f, 0.0f, 1.0f, 0.6f,
			1.0f, 0.5f, 0.5f, 0.7f,
			0.5f, 0.5f, 1.0f, 0.7f,
			1.0f, 1.0f, 0.0f, 0.8f,
			0.0f, 1.0f, 1.0f, 0.8f,
			1.0f, 0.0f, 1.0f, 0.8f,
			1.0f, 1.0f, 0.5f, 0.9f,
			1.0f, 0.5f, 1.0f, 0.9f,
			0.5f, 1.0f, 1.0f, 0.9f,
			1.0f, 1.0f, 1.0f, 1.0f
		};

		cl_image_format transferFunc_format;
		transferFunc_format.image_channel_order = CL_RGBA;
		transferFunc_format.image_channel_data_type = CL_FLOAT;
		d_transferFuncArray = clCreateImage2D(m_Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &transferFunc_format,
			16, 1, sizeof(float) * 16 * 4,
			transferFunc, &error);
		check_CL_error(error, "Create 2D Texture for Transferfunc Failed");

		// Create samplers for transfer function, linear interpolation and nearest interpolation 
		transferFuncSampler = clCreateSampler(m_Context, true, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR, &error);
		check_CL_error(error, "Create transferFuncSampler Failed");
		volumeSamplerLinear = clCreateSampler(m_Context, true, CL_ADDRESS_REPEAT, CL_FILTER_LINEAR, &error);
		check_CL_error(error, "Create volumeSamplerLinear Failed");
		volumeSamplerNearest = clCreateSampler(m_Context, true, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST, &error);
		check_CL_error(error, "Create volumeSamplerNearest Failed");
	}*/
}

int VolumeRender::OpenGLBufferInit()
{
	if (m_Pbo) {
		// delete old buffer
		clReleaseMemObject(m_PboCL);
		glDeleteBuffersARB(1, &m_Pbo);
	}

	// create pixel buffer object for display
	glGenBuffersARB(1, &m_Pbo);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_Pbo);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_RenderWidth * m_RenderHeight * sizeof(GLubyte) * 4, 0, GL_STREAM_DRAW_ARB);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	if (g_glInterop) 
	{
		// create OpenCL buffer from GL PBO
		m_PboCL = clCreateFromGLBuffer(m_Context, CL_MEM_WRITE_ONLY, m_Pbo, &error);
		check_CL_error(error, "create OpenCL buffer from GL PBO Failed");
	}
	else 
	{
		m_PboCL = clCreateBuffer(m_Context, CL_MEM_WRITE_ONLY, WIDTH * HEIGHT * sizeof(GLubyte)* 4, NULL, &error);
		check_CL_error(error, "create OpenCL buffer PBO Failed");
	}
	// init invViewMatrix
	d_invViewMatrix = clCreateBuffer(m_Context, CL_MEM_READ_ONLY, 12 * sizeof(float), 0, &error);
	check_CL_error(error, "create OpenCL buffer PBO Failed");
	return 1;
}

void VolumeRender::KernelInit()
{
	BuildKernel(VolumeKernelNames);
	m_Sliceobj->SliceOpenCLini(m_Context, m_Queue, m_Device);
	m_VoFilterObj->VolumeFilterOpenCLini(m_Context, m_Queue, m_Device);
}

void VolumeRender::RayCasting3DTexture(dim3 grid, dim3 threads, cl_mem pbocl, cl_uint gridSize[4], float density, float brightness, float transferOffset, float transferScale,
	cl_mem invMatrix, cl_mem d_volumearray, cl_mem transferfuncarray, cl_sampler volumeSampler, cl_sampler transfersampler)
{
	unsigned int k = RAYCASTING3DTex;
	unsigned int a = 0;
	int ImageW = gridSize[0];
	int ImageH = gridSize[1];
	short minvalue = m_Sliceobj->GetVolumeMinValue();
	short maxvalue = m_Sliceobj->GetVolumeMaxValue();
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&pbocl);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), gridSize);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &ImageW);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &ImageH);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_short), &maxvalue);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_short), &minvalue);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &density);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &brightness);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &transferOffset);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &transferScale);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&invMatrix);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), NULL);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), NULL);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), NULL);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&d_volumearray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&transferfuncarray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_sampler), &volumeSampler);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &transferFuncSampler);
	check_CL_error(error, "Configuring RayCasting3DTexture failed");

	//grid.x *= threads.x;
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, (size_t *)&grid, (size_t *)&threads, 0, 0, 0);
	check_CL_error(error, "launch_RayCasting3DTexture failed!");
#if 0
	int totalsize = WIDTH * HEIGHT * sizeof(UINT32);
	float* h_pboreadback = (float *)malloc(totalsize);
	int err = clEnqueueReadBuffer(m_Queue, pbocl, CL_TRUE, 0, totalsize, h_pboreadback, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	FILE *fp = NULL;
	fopen_s(&fp, "PBOreadbacktest.txt", "w+");
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			fprintf_s(fp, "%d ", h_pboreadback[i*WIDTH + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	//free(h_pboreadback);
#endif

#if 0
	totalsize = WIDTH * HEIGHT * sizeof(float);
	float* h_pboreadbackx = (float *)malloc(totalsize);
	float* h_pboreadbacky = (float *)malloc(totalsize);
	float* h_pboreadbackz = (float *)malloc(totalsize);
	err = clEnqueueReadBuffer(m_Queue, dtestoutx, CL_TRUE, 0, totalsize, h_pboreadbackx, 0, 0, 0);
	err = clEnqueueReadBuffer(m_Queue, dtestouty, CL_TRUE, 0, totalsize, h_pboreadbacky, 0, 0, 0);
	err = clEnqueueReadBuffer(m_Queue, dtestoutz, CL_TRUE, 0, totalsize, h_pboreadbackz, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	fp = NULL;
	fopen_s(&fp, "volumereadback.txt", "w+");
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			fprintf_s(fp, "%f  %f  %f, ", h_pboreadbackx[i*WIDTH + j], h_pboreadbacky[i*WIDTH + j], h_pboreadbackz[i*WIDTH + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	//free(h_pboreadback);
#endif
	//clReleaseMemObject(dtestoutx);
	//clReleaseMemObject(dtestouty);
	//clReleaseMemObject(dtestoutz);
}

void VolumeRender::RayCasting3DTextureBTF(dim3 grid, dim3 threads, cl_mem pbocl, cl_uint gridSize[4], float density, float brightness, float transferOffset, float transferScale,
	cl_mem invMatrix, cl_mem d_volumearray, cl_mem transferfuncarray, cl_sampler volumeSampler, cl_sampler transfersampler)
{
	unsigned int k = RAYCASTING3DTexBTF;
	unsigned int a = 0;
	int ImageW = gridSize[0];
	int ImageH = gridSize[1];
	short minvalue = m_Sliceobj->GetVolumeMinValue();
	short maxvalue = m_Sliceobj->GetVolumeMaxValue();
	//cl_mem dtestoutx = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, ImageH*ImageW*sizeof(float), 0, &error);
	//cl_mem dtestouty = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, ImageH*ImageW*sizeof(float), 0, &error);
	//cl_mem dtestoutz = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, ImageH*ImageW*sizeof(float), 0, &error);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&pbocl);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), gridSize);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &ImageW);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &ImageH);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_short), &maxvalue);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_short), &minvalue);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &density);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &brightness);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &transferOffset);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &transferScale);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&invMatrix);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), NULL);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), NULL);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), NULL);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&d_volumearray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&transferfuncarray);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_sampler), &volumeSampler);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &transferFuncSampler);
	check_CL_error(error, "Configuring RayCasting3DTexture failed");

	//grid.x *= threads.x;
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, (size_t *)&grid, (size_t *)&threads, 0, 0, 0);
	check_CL_error(error, "launch_RayCasting3DTexture failed!");
#if 0
	int totalsize = WIDTH * HEIGHT * sizeof(UINT32);
	float* h_pboreadback = (float *)malloc(totalsize);
	int err = clEnqueueReadBuffer(m_Queue, pbocl, CL_TRUE, 0, totalsize, h_pboreadback, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	FILE *fp = NULL;
	fopen_s(&fp, "PBOreadbacktest.txt", "w+");
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			fprintf_s(fp, "%d ", h_pboreadback[i*WIDTH + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	//free(h_pboreadback);
#endif

#if 0
	totalsize = WIDTH * HEIGHT * sizeof(float);
	float* h_pboreadbackx = (float *)malloc(totalsize);
	float* h_pboreadbacky = (float *)malloc(totalsize);
	float* h_pboreadbackz = (float *)malloc(totalsize);
	err = clEnqueueReadBuffer(m_Queue, dtestoutx, CL_TRUE, 0, totalsize, h_pboreadbackx, 0, 0, 0);
	err = clEnqueueReadBuffer(m_Queue, dtestouty, CL_TRUE, 0, totalsize, h_pboreadbacky, 0, 0, 0);
	err = clEnqueueReadBuffer(m_Queue, dtestoutz, CL_TRUE, 0, totalsize, h_pboreadbackz, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	fp = NULL;
	fopen_s(&fp, "volumereadback.txt", "w+");
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			fprintf_s(fp, "%f  %f  %f, ", h_pboreadbackx[i*WIDTH + j], h_pboreadbacky[i*WIDTH + j], h_pboreadbackz[i*WIDTH + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	//free(h_pboreadback);
#endif
	//clReleaseMemObject(dtestoutx);
	//clReleaseMemObject(dtestouty);
	//clReleaseMemObject(dtestoutz);
}

void VolumeRender::RayCasting3DMem(dim3 grid, dim3 threads, cl_mem pbocl, cl_uint gridSize[4], float density, float brightness, float transferOffset, float transferScale,
	cl_mem invMatrix, cl_mem volume, cl_mem transferbuf)
{
	unsigned int k = RAYCASTINGMEMORY;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&pbocl);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), gridSize);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &density);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &brightness);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &transferOffset);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(float), &transferScale);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&invMatrix);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&volume);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&transferbuf);
	check_CL_error(error, "Configuring RayCasting3DMem failed");

	grid.x *= threads.x;
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	check_CL_error(error, "launch_gRayCasting3DMem failed");
}

void VolumeRender::Texture3dTesting(dim3 grid, dim3 threads, cl_mem volumetex, cl_sampler volumeSampler, cl_mem d_out, cl_uint imageW, cl_uint imageH, cl_uint Nslice)
{
	unsigned int k = TESTRW3DTEXTURE;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&volumetex);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_sampler), &volumeSampler);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &d_out);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &imageW);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &imageH);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &Nslice);
	check_CL_error(error, "configuring Texture3dTesting failed");
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	check_CL_error(error, "launch_Texture3dTesting failed");
}

void VolumeRender::Texture2dTesting(dim3 grid, dim3 threads, cl_mem imagetex, cl_sampler transferSampler, cl_mem d_out, cl_uint length)
{
	unsigned int k = TESTRW2DTEXTURE;
	unsigned int a = 0;
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), (void *)&imagetex);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_sampler), (void *)&transferSampler);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &d_out);
	error |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &length);

	grid.x *= threads.x;
	error = clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	check_CL_error(error, "launch_Texture2dTesting failed");
}

void VolumeRender::Testing3DTex()
{
	dim3 local = { 256, 1, 1 };
	int totalpt = GridSize[0] * GridSize[1] * GridSize[2];
	dim3 grid(totalpt/local.x, 1, 1);
	cl_mem testdout = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, sizeof(short)*GridSize[0] * GridSize[1] * GridSize[2], 0, &error);
	grid.x *= local.x;
	Texture3dTesting(grid, local, d_volumeArray, volumeSamplerLinear, testdout, GridSize[0], GridSize[1], GridSize[2]);
#if 0
	short* h_volumereadback = (short *)malloc(totalpt* sizeof(short));
	int err = clEnqueueReadBuffer(m_Queue, testdout, CL_TRUE, 0, totalpt* sizeof(short), h_volumereadback, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	FILE *fp = NULL;
	fopen_s(&fp, "volumereadback.txt", "w+");
	for (int i = 0; i < totalpt / GridSize[0]; i++)
	{
		for (int j = 0; j < GridSize[0]; j++)
		{
			fprintf_s(fp, "%d ", h_volumereadback[i*GridSize[0] + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	free(h_volumereadback);
#endif
}

void VolumeRender::Testing2DTex()
{
	dim3 local = { 16, 1, 1 };
	int totalpt = 16;
	dim3 grid(totalpt / local.x, 1, 1);
	cl_mem testdout = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, sizeof(float)*totalpt, 0, &error);
	Texture2dTesting(grid, local, d_transferFuncArray, transferFuncSampler, testdout, 16);
#if 1
	float* h_volumereadback = (float *)malloc(totalpt* sizeof(float));
	int err = clEnqueueReadBuffer(m_Queue, testdout, CL_TRUE, 0, totalpt* sizeof(float), h_volumereadback, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	for (int i = 0; i < totalpt; i++)
	{
		printf("%f\n", h_volumereadback[i]);
	}
	free(h_volumereadback);
#endif
}

void VolumeRender::RayCastingProcessing()
{
	m_InvViewMatrix[0] = m_ModelView[0]; m_InvViewMatrix[1] = m_ModelView[4]; m_InvViewMatrix[2] = m_ModelView[8]; m_InvViewMatrix[3] = m_ModelView[12];
	m_InvViewMatrix[4] = m_ModelView[1]; m_InvViewMatrix[5] = m_ModelView[5]; m_InvViewMatrix[6] = m_ModelView[9]; m_InvViewMatrix[7] = m_ModelView[13];
	m_InvViewMatrix[8] = m_ModelView[2]; m_InvViewMatrix[9] = m_ModelView[6]; m_InvViewMatrix[10] = m_ModelView[10]; m_InvViewMatrix[11] = m_ModelView[14];
	error = clEnqueueWriteBuffer(m_Queue, d_invViewMatrix, CL_TRUE, 0, 12 * sizeof(float), m_InvViewMatrix, 0, 0, 0);
	check_CL_error(error, "Write InvViewMatrix to GPU Failed");
#if 1
	int totalsize = 12 * sizeof(float);
	float* h_pboreadback = (float *)malloc(totalsize);
	int err = clEnqueueReadBuffer(m_Queue, d_invViewMatrix, CL_TRUE, 0, totalsize, h_pboreadback, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	for (int i = 0; i < 12; i++)
	{
		printf("%f ", h_pboreadback[i]);
		if (i % 4 == 0)
		{
			printf("\n");
		}
	}
	printf("\n");
#endif
	dim3 local = { BLOCKX, BLOCKY, 1 };
	int dstwidth = m_RenderWidth;
	int dstheight = m_RenderHeight;
	int groupx = dstwidth / BLOCKX;
	int groupy = dstheight / BLOCKY;
	dim3 grid(dstwidth, dstheight, 1);
	
	if (g_glInterop) 
	{
		// Acquire PBO for OpenCL writing
		glFlush();
		error |= clEnqueueAcquireGLObjects(m_Queue, 1, &m_PboCL, 0, 0, 0);
	}
	RayCasting3DTextureBTF(grid, local, m_PboCL, GridSize, m_Density, m_Brightness, m_TransferOffset,
		m_TransferScale, d_invViewMatrix, d_volumeArray, d_transferFuncArray, volumeSamplerLinear, transferFuncSampler);
	if (g_glInterop)
	{
		// Transfer ownership of buffer back from CL to GL    
		error |= clEnqueueReleaseGLObjects(m_Queue, 1, &m_PboCL, 0, 0, 0);
		check_CL_error(error, "Error Occured while transfering from openCL to openGL");
		clFinish(m_Queue);
	}

#if 0
	int totalsize = WIDTH * HEIGHT * sizeof(UINT32);
	UINT32* h_pboreadback = (UINT32 *)malloc(totalsize);
	int err = clEnqueueReadBuffer(m_Queue, m_PboCL, CL_TRUE, 0, totalsize, h_pboreadback, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	FILE *fp = NULL;
	fopen_s(&fp, "volumereadback.txt", "w+");
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			fprintf_s(fp, "%d ", h_pboreadback[i*WIDTH + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	//free(h_pboreadback);
#endif
}

void VolumeRender::RayCastingRendering()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glRasterPos2i(0, 0);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_Pbo);
	glDrawPixels(m_RenderWidth, m_RenderHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
}

//compute
void VolumeRender::Volumecomputing()
{
	//Testing2DTex();
	//Testing3DTex();
	RayCastingProcessing();
}

//rendering
void VolumeRender::RenderProcess()
{
	RayCastingRendering();
}
