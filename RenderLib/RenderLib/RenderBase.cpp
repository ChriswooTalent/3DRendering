// RenderBase:  
// A Basic Class for 3D or 2D rendering
// The define of some Global Param here
//    
//  Created by Chriswoo on 2017-08-02.
//  Contact: wsscx01@aliyun.com
// 
#include "RenderBase.h"
#include "ImageBasicProcess.h"
#include "QuaternionProcess.h"
#include <fstream> 
#include <sstream>

using namespace QuaternionProcess;
//NDIDriveBay
extern "C" __declspec(dllimport) void InitNDIDriveBay(const char *err, int &errorCode);
extern "C" __declspec(dllimport) void GetNDIDriveBaySynRecord(void *pRecord, int elementsize, int recordSize, int &validsensornum, int *sensorindex, int &errorCode, const char *err);
extern "C" __declspec(dllimport) void ReleaseNDIDriveBayResource();

RenderBase::RenderBase()
{
	//OpenGL shader
	m_GLShader = 0;
	//OpenGL Texture
	m_Pixels = NULL;
	m_Pixels16 = NULL;
	//error message
	error_message = NULL;
	slicepbo = 0;
	m_SlicePboCL = 0;
	m_RegPboCL = 0;
	d_volume = 0;
	d_volumarraybase = 0;
	d_volumeBone = 0;
	d_volumemask = 0;
	d_upperzp = 0;
	d_xstartoffset = 0;
	d_ystartoffset = 0;
	m_Sliceobj = new Rotateslice();
	for (int k = 0; k < DEFAULTSENSORCOUNT; k++)
	{
		m_RegObj[k] = new CorRegisTration();
	}
	m_VoFilterObj = new VolumeFiler();
	m_HostVolume = NULL;
	for (int i = 0; i < 16; i++)
	{
		m_ModelView[i] = 0.0f;
	}
	for (int i = 0; i < 4; i++)
	{
		m_SkinColor[i] = 0.0f;
		m_BoneColor[i] = 0.0f;
		m_MaskColor[i] = 0.0f;
	}
	m_AnimateFlag = true;
	m_ComputeFlag = true;
	m_ComputeBMFlag = true;
	m_ComputeMaskFlag = true;
	m_SkinFilteringIterTimes = 0;
	m_BoneFilteringIterTimes = 0;
	m_MaskFilteringIterTimes = 0;
	m_SkinMeshFilteringFlag = 0;
	m_BoneMeshFilteringFlag = 0;
	m_MaskMeshFilteringFlag = 0;
	m_MAXIterTimes = 0;

	m_MaskStartNo = 0;
	m_MaskEndNo = 0;
	m_InterpSliceNum = 0;
	m_pad = 0;
	m_InterpVoxelZsize = 0.0f;
	m_InterpZstart = 0.0f;

	m_EyeCenterPt.x = 0.0f;
	m_EyeCenterPt.y = 0.0f;
	m_EyeCenterPt.z = 0.0f;
	m_EyeFoucusCenterPt.x = 0.0f;
	m_EyeFoucusCenterPt.y = 0.0f;
	m_EyeFoucusCenterPt.z = 0.0f;
	m_HeadDirection.x = 0.0f;
	m_HeadDirection.y = 0.0f;
	m_HeadDirection.z = 0.0f;

	//center offset
	m_XCenterOffset = 0.0f;
	m_YCenterOffset = 0.0f;
	m_ZCenterOffset = 0.0f;
	//zoom and scale
	m_ZoomAspect = 1.0f;
	m_ScaleParam = 1.0f;
	//Rendering bounds
	m_UpperBoundRenderingX = 0.0f;
	m_UpperBoundRenderingY = 0.0f;
	m_UpperBoundRenderingZ = 0.0f;
	m_BottomBoundRenderingX = 0.0f;
	m_BottomBoundRenderingY = 0.0f;
	m_BottomBoundRenderingZ = 0.0f;
	//wincenter
	m_WindowCenter = 0.0f;
	m_WindowWidth = 0.0f;
	//Grid offset
	m_XGridoffset = 0;
	m_YGridoffset = 0;
	m_ZGridoffset = 0;

	m_SkinRenderFlag = false;
	m_BoneRenderFlag = false;
	for (int i = 0; i < MAXNUMOFORGAN; i++)
	{
		m_HostVolumeOrgan[i] = NULL;
		m_OrganRenderFlag[i] = false;
	}
	m_SliceRenderFlag = false;
	m_PboSWflag = false;
	m_RegProcessRenderFlag = false;
	m_RegFinishRenderFlag = false;
	for (int i = 0; i < DEFAULTSENSORCOUNT; i++)
	{
		m_PunctureLineRenderFlag[i] = false;
	}

	//pbo resize flag
	m_PboResizeFlag = false;

	m_Count = 0.0f;
	m_PunCtCount = 0.0f;

	//NDI Record
	m_NDIConnectedFlag = false;
	m_NDIFinishedFlag = false;
	m_ValidSensorCount = 0;
	memset(m_OldRecord, 0, DEFAULTSENSORCOUNT * sizeof(m_OldRecord[0]));
	memset(m_CurRecord, 0, DEFAULTSENSORCOUNT * sizeof(m_CurRecord[0]));
	memset(m_SensorInd, 0, DEFAULTSENSORCOUNT * sizeof(int));

	//Registration Im buf
	for (int i = 0; i < MAXREGDIRECIMNUM; i++)
	{
		m_RegDirectionImBuf[i] = (UINT8 *)malloc(3*REGDIRECIMHEIGHT*REGDIRECIMWIDTH);
		memset(m_RegDirectionImBuf[i], 0, 3 * REGDIRECIMHEIGHT*REGDIRECIMWIDTH);
	}
	m_ProbeIconImBuf = (UINT8 *)malloc(3 * PROBEICONWIDTH*PROBEICONHEIGHT*sizeof(UINT8));
	memset(m_ProbeIconImBuf, 0, 3 * PROBEICONWIDTH*PROBEICONHEIGHT*sizeof(UINT8));
	m_WorkBuf = (UINT8 *)malloc(4 * WIDTH*HEIGHT);
	memset(m_WorkBuf, 0, 4 * WIDTH*HEIGHT);
	memset(m_RegHintFilePath, 0, 255 * sizeof(char));
	m_RegHintChangeTickCount = 0;
	m_RegHintFinishWaitTickCount = 0;
	m_RegHintIndex = 0;

	m_Texture[0] = 0;
	m_Texture[1] = 0;
	m_Texture[2] = 0;
	m_ProbeTexture = 0;

	//Registration Angle Info
	m_Rollstart = 0.0f;
	m_Elevationstart = 0.0f;
	m_Azimuthstart = 0.0f;
	m_OldRoll = 0.0f;
	m_OldElevation = 0.0f;
	m_OldAzimuth = 0.0f;
	m_CurRoll = 0.0f;
	m_CurElevation = 0.0f;
	m_CurAzimuth = 0.0f;
}

RenderBase::~RenderBase()
{
	if (m_Sliceobj)
	{
		delete m_Sliceobj;
		m_Sliceobj = NULL;
	}
	if (m_VoFilterObj)
	{
		delete m_VoFilterObj;
		m_VoFilterObj = NULL;
	}
	if (m_HostVolume)
	{
		free(m_HostVolume);
		m_HostVolume = NULL;
	}
	for (int i = 0; i < DEFAULTSENSORCOUNT; i++)
	{
		if (m_RegObj[i])
		{
			delete m_RegObj[i];
			m_RegObj[i] = NULL;
		}
	}
	for (int i = 0; i < MAXNUMOFORGAN; i++)
	{
		if (m_HostVolumeOrgan[i])
		{
			free(m_HostVolumeOrgan[i]);
			m_HostVolumeOrgan[i] = NULL;
		}
	}
	//Registration Im buf
	for (int i = 0; i < MAXREGDIRECIMNUM; i++)
	{
		if (m_RegDirectionImBuf[i])
		{
			free(m_RegDirectionImBuf[i]);
			m_RegDirectionImBuf[i] = NULL;
		}
	}
	//free work buffer
	if (m_ProbeIconImBuf)
	{
		free(m_ProbeIconImBuf);
	}
	if (m_WorkBuf)
	{
		free(m_WorkBuf);
	}
	if (d_volume) clReleaseMemObject(d_volume);
	if (d_volumeBone) clReleaseMemObject(d_volumeBone);
	if (d_volumemask) clReleaseMemObject(d_volumemask);
	if (d_upperzp) clReleaseMemObject(d_upperzp);
	if (d_xstartoffset) clReleaseMemObject(d_xstartoffset);
	if (d_ystartoffset) clReleaseMemObject(d_ystartoffset);
	OpenCLRelease();
	ReleaseNDIDriveBayResource();
	StopInternalThread();
}

int RenderBase::OpenGLBufferInit()
{
	return 1;
}

void RenderBase::TextureInit()
{
}

void RenderBase::Volumecomputing()
{
}

void RenderBase::RenderProcess()
{
}

void RenderBase::GLDataRendering(cl_float translate[4], cl_float clrotate[4], int orderflag)
{
}

void RenderBase::WGLDataRendering(int orderflag)
{
}

void RenderBase::SetRenderingFeature(RenderParam param)
{

}

void RenderBase::GetRenderingFeature(RenderParam &param)
{
}

void RenderBase::RenderTexSlice(GLuint tex2d, int orderflag)
{
}

void RenderBase::RenderRegHintPBO(GLuint tex2d, int bufindex, int orderflag)
{
}

void RenderBase::RenderingPunctureLine()
{
}

void RenderBase::RenderingProbesensor(GLuint tex2d, int orderflag)
{
}

void RenderBase::KeyBoard(unsigned char key)
{
	switch (key) {
		//zoom in
	case '+':
		m_ZoomAspect += 5.0f;
		m_ScaleParam += 0.1f;
		if (m_ScaleParam >= 2.0f)
		{
			m_ScaleParam = 2.0f;
		}
		break;
		//zoom out
	case '-':
		m_ZoomAspect -= 5.0f;
		m_ScaleParam -= 0.1f;
		if (m_ScaleParam <= 0.2f)
		{
			m_ScaleParam = 0.2f;
		}
		break;
		//open and close SkinRendering
	case '0':
		if (m_SkinRenderFlag == false)
		{
			m_SkinRenderFlag = true;
		}
		else if (m_SkinRenderFlag == true)
		{
			m_SkinRenderFlag = false;
		}
		break;
		//open and close BoneRendering
	case '1':
		if (m_BoneRenderFlag == false)
		{
			m_BoneRenderFlag = true;
		}
		else if (m_BoneRenderFlag == true)
		{
			m_BoneRenderFlag = false;
		}
		break;
		//open and close Mask1Rendering
	case '2':
		if (m_OrganRenderFlag[0] == false)
		{
			m_OrganRenderFlag[0] = true;
		}
		else if (m_OrganRenderFlag[0] == true)
		{
			m_OrganRenderFlag[0] = false;
		}
		break;
		//open and close Mask2Rendering
	case '3':
		if (m_SliceRenderFlag == false)
		{
			m_SliceRenderFlag = true;
		}
		else if (m_SliceRenderFlag == true)
		{
			m_SliceRenderFlag = false;
		}
		break;
		//open and close Mask3Rendering
	case '4':
		if (m_PboSWflag == false)
		{
			m_PboSWflag = true;
		}
		else if (m_PboSWflag == true)
		{
			m_PboSWflag = false;
		}
		break;
	case '5'://(turnning on and turnning off of the Puncline1)
		if (m_PunctureLineRenderFlag[0] == false)
		{
			m_PunctureLineRenderFlag[0] = true;
		}
		else if (m_PunctureLineRenderFlag[0] == true)
		{
			m_PunctureLineRenderFlag[0] = false;
		}
		break;
	case '6'://(Hint of the starting step of the Registration)
		WaitingForRegFlag();
		SetRegStartFlag(true);
		SetRegFinshedFlag(false);
		m_RegProcessRenderFlag = true;
		m_RegFinishRenderFlag = false;
		break;
	case '7'://(Finish the Registration Process)
		m_RegFinishRenderFlag = true;
		m_RegProcessRenderFlag = false;
		GetWorldRecordForReg();
		SetRegistrationPt1(m_WorldPt, m_RefPt1, 0);
		SetRegFinshedFlag(true);
		SetRegStartFlag(false);
		RegistrationSimple();
		m_RegObj[0]->StopInternalThread();
		break;
	case '8'://(turnning on and turnning off of the Puncline2)
		if (m_PunctureLineRenderFlag[1] == false)
		{
			m_PunctureLineRenderFlag[1] = true;
		}
		else if (m_PunctureLineRenderFlag[1] == true)
		{
			m_PunctureLineRenderFlag[1] = false;
		}
		break;
	case '9':
		float *buf = (float *)malloc(512 * 512 * sizeof(float));
		GetNomalizedSliceData(buf);
		FILE *fp = 0;
		fopen_s(&fp, "D:/matlab_rotation_test/SliceOut.dat", "wb+");
		fwrite(buf, sizeof(float), 512 * 512, fp);
		fclose(fp);
		break;
	}
}

void RenderBase::InitNDIDevice()
{
	InitNDIDriveBay(error_message, error);
	if (error == 0)
	{
		m_NDIConnectedFlag = true;
	}
	else
	{
		m_NDIConnectedFlag = false;
		PrintLog("Init NDI Device Failed");
	}
}

void RenderBase::animation()
{
}

GLuint RenderBase::compileASMShader(GLenum program_type, const char *code)
{
	GLuint program_id;
	glGenProgramsARB(1, &program_id);
	glBindProgramARB(program_type, program_id);
	glProgramStringARB(program_type, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(code), (GLubyte *)code);

	GLint error_pos;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);
	if ((int)error_pos != -1) {
		const GLubyte *error_string;
		error_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		//shrLog("Program error at position: %d\n%s\n", (int)error_pos, error_string);
		return 0;
	}
	return program_id;
}

void RenderBase::Init2DTexture()
{
	UINT8 *slicedata = 0;
	int slicewidth = 0;
	int sliceheight = 0;
	GetSliceInfo(slicewidth, sliceheight);
	slicedata = (UINT8 *)malloc(slicewidth*sliceheight*sizeof(UINT8));
	memset(slicedata, 0, slicewidth*sliceheight*sizeof(UINT8));
	GetSliceDataU8CPU(slicedata);

	LoadGLU8Textures(slicedata, m_Texture[1], slicewidth, sliceheight);
	free(slicedata);
}

void RenderBase::GetRegProcessFinishedHintPBO()
{
	int pbowidth = REGDIRECIMWIDTH;
	int pboheight = REGDIRECIMHEIGHT;
	InitImageBufAlpha(m_RegDirectionImBuf[7], pbowidth, pboheight);
	if (g_glInterop)
	{
		// Acquire PBO for OpenCL writing
		glFlush();
		error |= clEnqueueAcquireGLObjects(m_Queue, 1, &m_RegPboCL, 0, 0, 0);
	}
	clEnqueueWriteBuffer(m_Queue, m_RegPboCL, CL_TRUE, 0, sizeof(UINT8)*pbowidth * pboheight * 4, m_WorkBuf, 0, 0, 0);
	check_CL_error(error, "m_RegPboCL changed failed");
	if (g_glInterop)
	{
		// Transfer ownership of buffer back from CL to GL    
		error |= clEnqueueReleaseGLObjects(m_Queue, 1, &m_RegPboCL, 0, 0, 0);
		check_CL_error(error, "PBO Error Occured while transfering from openCL to openGL");
		clFinish(m_Queue);
	}
}

void RenderBase::GetRegProcessHintPBO()
{
	int pbowidth = REGDIRECIMWIDTH;
	int pboheight = REGDIRECIMHEIGHT;
	
	m_RegHintChangeTickCount ++;
	if (m_RegHintChangeTickCount == 10)
	{
		m_RegHintChangeTickCount = 0;
		m_RegHintIndex += 1;
		if (m_RegHintIndex == 6)
		{
			m_RegHintIndex = 0;
		}
	}
	int index = m_RegHintIndex;
	InitImageBufAlpha(m_RegDirectionImBuf[index], pbowidth, pboheight);
	if (g_glInterop)
	{
		// Acquire PBO for OpenCL writing
		glFlush();
		error |= clEnqueueAcquireGLObjects(m_Queue, 1, &m_RegPboCL, 0, 0, 0);
	}
	clEnqueueWriteBuffer(m_Queue, m_RegPboCL, CL_TRUE, 0, sizeof(UINT8)*pbowidth * pboheight * 4, m_WorkBuf, 0, 0, 0);
	check_CL_error(error, "m_RegPboCL changed failed");
	if (g_glInterop)
	{
		// Transfer ownership of buffer back from CL to GL    
		error |= clEnqueueReleaseGLObjects(m_Queue, 1, &m_RegPboCL, 0, 0, 0);
		check_CL_error(error, "PBO Error Occured while transfering from openCL to openGL");
		clFinish(m_Queue);
	}
}

void RenderBase::GetPBO()
{
	int pbowidth = m_Sliceobj->GetDstWidth();
	int pboheight = m_Sliceobj->GetDstHeight();
	if ((WIDTH == pbowidth) && (HEIGHT == pboheight))
	{
		m_PboResizeFlag = false;
	}
	else
	{
		m_PboResizeFlag = true;
	}
	if (g_glInterop)
	{
		// Acquire PBO for OpenCL writing
		glFlush();
		error |= clEnqueueAcquireGLObjects(m_Queue, 1, &m_SlicePboCL, 0, 0, 0);
	}

	ClearSlicePBO(m_SlicePboCL);
	GetSlicePBO(m_SlicePboCL, m_PboResizeFlag);
#if 0
	UINT32* h_pbo = (UINT32 *)malloc(WIDTH*HEIGHT*sizeof(UINT32));
	int err = clEnqueueReadBuffer(m_Queue, m_SlicePboCL, CL_TRUE, 0, WIDTH*HEIGHT*sizeof(UINT32), h_pbo, 0, 0, 0);
	if (err != 0)
	{
		return;
	}
	FILE *fp = NULL;
	fopen_s(&fp, "pbo_test.txt", "w+");
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			fprintf_s(fp, "%d ", h_pbo[i*GridSize[0] + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	free(h_pbo);
#endif
	if (g_glInterop)
	{
		// Transfer ownership of buffer back from CL to GL    
		error |= clEnqueueReleaseGLObjects(m_Queue, 1, &m_SlicePboCL, 0, 0, 0);
		check_CL_error(error, "PBO Error Occured while transfering from openCL to openGL");
		clFinish(m_Queue);
	}
}

void RenderBase::Init2DTexturePBO()
{
	createPBO(slicepbo, WIDTH*HEIGHT*sizeof(GLubyte)* 4, m_SlicePboCL, 1);
	PrintLog("createPBO end!");
	glGenTextures(1, &m_Texture[0]);
	glBindTexture(GL_TEXTURE_2D, m_Texture[0]);

	// Step2 设定wrap参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderBase::InitRegProcessHintPBO()
{
	createPBO(regpbo, REGDIRECIMWIDTH*REGDIRECIMHEIGHT*sizeof(GLubyte) * 4, m_RegPboCL, 2);
	glGenTextures(1, &m_Texture[2]);
	glBindTexture(GL_TEXTURE_2D, m_Texture[2]);

	// Step2 设定wrap参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, REGDIRECIMWIDTH, REGDIRECIMHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderBase::LoadGLU8ColorTextures(UINT8 *imdata, GLuint tex2d, int width, int height, int channels)
{
	GLenum fms[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
	if (imdata == NULL)
	{
		check_CL_error(-1, "LoadGLU8ColorTextures data is NULL");
	}
	int pixellength = width*height*channels;
	m_Pixels = (GLubyte *)malloc(pixellength*sizeof(GLubyte));
	memcpy(m_Pixels, imdata, pixellength*sizeof(GLubyte));

	if (imdata)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_2D, tex2d);
		// Step2 设定wrap参数
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


		// Step3 设定filter参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, fms[channels], width, height, 0, fms[channels], GL_UNSIGNED_BYTE, m_Pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	free(m_Pixels);
}

void RenderBase::LoadGLU8Textures(UINT8 *imdata, GLuint tex2d, int width, int height)
{
	if (imdata == NULL)
	{
		return;
	}
	int pixellength = width*height;
	m_Pixels = (GLubyte *)malloc(pixellength*sizeof(GLubyte));
	memcpy(m_Pixels, imdata, pixellength*sizeof(GLubyte));
#if 0
	FILE *fp = 0;
	fopen_s(&fp, "texture_matlab.txt", "w+");
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			fprintf(fp, "%d ", imdata[i*width + j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
#endif

	if (imdata)
	{
		glGenTextures(1, &tex2d);
		glBindTexture(GL_TEXTURE_2D, tex2d);
		// Step2 设定wrap参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


		// Step3 设定filter参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_Pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	free(m_Pixels);
}

void RenderBase::LoadGLTextures(short *imdata, int width, int height)
{
	if (imdata == NULL)
	{
		return;
	}
	int pixellength = width*height;
	m_Pixels16 = (GLshort *)malloc(pixellength*sizeof(GLshort));
	memcpy(m_Pixels16, imdata, pixellength*sizeof(GLshort));
#if 0
	FILE *fp = 0;
	fopen_s(&fp, "texture_matlab.txt", "w+");
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			fprintf(fp, "%d ", imdata[i*width + j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
#endif

	if (imdata)
	{
		glGenTextures(1, &m_Texture[0]);
		glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_LUMINANCE, GL_SHORT, m_Pixels16);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	free(m_Pixels16);
}

void RenderBase::createPBO(GLuint &pbo, UINT32 size, cl_mem &pbo_cl, GLuint pboid)
{
	if (pbo) {
		// delete old buffer
		clReleaseMemObject(pbo_cl);
		glDeleteBuffersARB(1, &pbo);
	}

	// create pixel buffer object for display
	glGenBuffers(pboid, &pbo);
	//glGenBuffersARB(pboid, &pbo);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, size, 0, GL_STREAM_DRAW_ARB);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	PrintLog("glBindBufferARB end!");
	if (g_glInterop)
	{
		// create OpenCL buffer from GL PBO
		pbo_cl = clCreateFromGLBuffer(m_Context, CL_MEM_WRITE_ONLY, pbo, &error);
		check_CL_error(error, "create OpenCL buffer from GL PBO Failed");
	}
	else
	{
		pbo_cl = clCreateBuffer(m_Context, CL_MEM_WRITE_ONLY, size, NULL, &error);
		check_CL_error(error, "create OpenCL buffer PBO Failed");
	}
}

void RenderBase::deletePBO(GLuint pbo, cl_mem pbo_cl)
{
	// release res
	if (pbo)
	{
		// delete old buffer
		clReleaseMemObject(pbo_cl);
		glDeleteBuffersARB(1, &pbo);
		pbo = 0;
	}
}

void RenderBase::SetDpRenderingFlag(bool skinflag, bool boneflag, bool sliceflag, bool *punctureflag, bool *organflag)
{
	m_SkinRenderFlag = skinflag;
	m_BoneRenderFlag = boneflag;
	m_SliceRenderFlag = sliceflag;
	for (int i = 0; i < DEFAULTSENSORCOUNT; i++)
	{
		m_PunctureLineRenderFlag[i] = punctureflag[i];
	}
	for (int i = 0; i < m_OrganNum; i++)
	{
		m_OrganRenderFlag[i] = organflag[i];
	}
}

//设定4个光源，前后左右4个
void RenderBase::SetLightsForRendering()
{
	float lightPos0[] = { 0.0f, 0.0f, 10.0f, 0.0f };
	float lightPos1[] = { 0.0f, 0.0f, -10.0f, 0.0f };
	float lightPos2[] = { 10.0f, 0.0f, 0.0f, 0.0f };
	float lightPos3[] = { -10.0f, 0.0f, 0.0f, 0.0f };

	float lightblack[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float lightwhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float lightambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	float lightdiffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float lightspecular[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightwhite);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightwhite);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightspecular);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, lightwhite);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightwhite);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightspecular);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);

	glLightfv(GL_LIGHT2, GL_AMBIENT, lightambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightdiffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, lightspecular);
	glLightfv(GL_LIGHT2, GL_POSITION, lightPos2);

	glLightfv(GL_LIGHT3, GL_AMBIENT, lightambient);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, lightdiffuse);
	glLightfv(GL_LIGHT3, GL_SPECULAR, lightspecular);
	glLightfv(GL_LIGHT3, GL_POSITION, lightPos3);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_LIGHT3);
}

void RenderBase::ReshapeFunc(int w, int h, int texflag)
{
	float zaspect = m_ZoomAspect;
	if (texflag == 1)
	{
		glViewport(0, 0, w, h);
	}
	else
	{
		GLfloat fAspect = 0.0f;
		GLfloat fHalfWorldSize = (1.414f / 2);
		glViewport(0, 0, w, h);
		glMatrixMode(GL_PROJECTION);//先投影
		glLoadIdentity();
		if (w <= h)
		{
			fAspect = (GLfloat)h / (GLfloat)w;
			glOrtho(-fHalfWorldSize*zaspect, fHalfWorldSize*zaspect, -fHalfWorldSize*fAspect*zaspect,
				fHalfWorldSize*fAspect*zaspect, -5 * fHalfWorldSize*zaspect, 5 * fHalfWorldSize*zaspect);
		}
		else
		{
			fAspect = (GLfloat)w / (GLfloat)h;
			glOrtho((-fHalfWorldSize)*fAspect*zaspect, fHalfWorldSize*fAspect*zaspect, -fHalfWorldSize*zaspect,
				fHalfWorldSize*zaspect, -5 * fHalfWorldSize*zaspect, 5 * fHalfWorldSize*zaspect);
		}
		gluLookAt(m_EyeCenterPt.x, m_EyeCenterPt.y, m_EyeCenterPt.z, m_EyeFoucusCenterPt.x, m_EyeFoucusCenterPt.y, m_EyeFoucusCenterPt.z,
			m_HeadDirection.x, m_HeadDirection.y, m_HeadDirection.z);
	}
}

void RenderBase::WaitForInitOfNDI()
{
	WaitForSingleObject(m_NotifyEvent, INFINITE);
}

void RenderBase::GetWorldRecordForReg()
{
	PositionAngleUnit *pRecord = m_CurRecord;
	if (m_NDIConnectedFlag)
	{
		GetNDIDriveBaySynRecord(pRecord, sizeof(PositionAngleUnit), 1, m_ValidSensorCount, m_SensorInd, error, error_message);
		check_CL_error(error, error_message);
		if (m_ValidSensorCount > 0)
		{
			float x_onplane = pRecord[0].x;
			float y_onplane = pRecord[0].y;
			float z_onplane = pRecord[0].z;
			m_WorldPt.x = x_onplane;
			m_WorldPt.y = y_onplane;
			m_WorldPt.z = z_onplane;
			m_Rollstart = pRecord[0].r;
			m_Elevationstart = pRecord[0].e;
			m_Azimuthstart = pRecord[0].a;
		}
	}
}

void RenderBase::GetRealTimeNDIRecord()
{
	Quaternion QuNionTemp[DEFAULTSENSORCOUNT];
	Quaternion RegQuaternion;
	Quaternion TestQuaternion;
	Quaternion TempResQuaternion;
	PositionAngleUnit *pRecord = m_CurRecord;
	float count = 0.0f;
	vector3d axisvectest;
	axisvectest.fX = 0.0f;
	axisvectest.fY = 0.0f;
	axisvectest.fZ = 1.0f;
	error = 0;
	float Regrmat[9] = { 0.0f };
	Regrmat[0] = 1.0f;
	Regrmat[4] = 1.0f;
	Regrmat[8] = 1.0f;
	float tvec[3] = { 0.0f };
	vector3d originpt;
	vector3d rotatedpt;
	vector3d transformpt;
	vector3d transvec3d;
	if (m_NDIConnectedFlag)
	{
		GetNDIDriveBaySynRecord(pRecord, sizeof(PositionAngleUnit), 1, m_ValidSensorCount, m_SensorInd, error, error_message);
		check_CL_error(error, error_message);
		if (m_ValidSensorCount > 0)
		{
			if (m_ValidSensorCount > DEFAULTSENSORCOUNT)
			{
				m_ValidSensorCount = DEFAULTSENSORCOUNT;
			}
			for (int kn = 0;  kn < m_ValidSensorCount; kn ++)
			{
				//use the Regrmat and the tvec of the first Registration object
				if (0 == kn)
				{
					//m_RegObj[kn]->GetRegistrationCorInfo(Regrmat, tvec);
					m_RegObj[kn]->GetRegistrationQCorInfo(RegQuaternion, tvec);
				}
				else
				{
					m_RegObj[kn]->SetRegistrationQCorInfo(RegQuaternion, tvec);
					//m_RegObj[kn]->SetRegistrationCorInfo(Regrmat, tvec);
				}
				float x_onplane = pRecord[kn].x;
				float y_onplane = pRecord[kn].y;
				float z_onplane = pRecord[kn].z;
				originpt.fX = x_onplane;
				originpt.fY = y_onplane;
				originpt.fZ = z_onplane;
				transvec3d.fX = tvec[0];
				transvec3d.fY = tvec[1];
				transvec3d.fZ = tvec[2];
				//transformpt = m_Sliceobj->matrix_mult(Regrmat, originpt) + transvec3d;
				Quaternion tempquater(0.0f, originpt.fX, originpt.fY, originpt.fZ);
				tempquater = RegQuaternion.GetQRotationResult(tempquater);
				rotatedpt.fX = tempquater.GetQuaternionX();
				rotatedpt.fY = tempquater.GetQuaternionY();
				rotatedpt.fZ = tempquater.GetQuaternionZ();
				transformpt = rotatedpt + transvec3d;

				float azimuth = (pRecord[kn].e - m_Elevationstart)*PI / 180.0;//z angle
				float elevation = (pRecord[kn].r - m_Rollstart)*PI / 180.0;//y angle
				float roll = (pRecord[kn].a - m_Azimuthstart)*PI / 180;//x angle

				/*TestQuaternion.FromEuler(azimuth, roll, elevation, 1);
				TempResQuaternion = RegQuaternion.GetQRotationResult(TestQuaternion);
				float xangletest = 0.0f;
				float yangletest = 0.0f;
				float zangletest = 0.0f;
				TempResQuaternion.GetXYZAxisAngleByQuaternion(xangletest, yangletest, zangletest);*/
				if (fabs(azimuth - m_OldAzimuth) >= CHANGE_THRESH)
				{
					m_OldAzimuth = azimuth;
					m_CurAzimuth = azimuth;
				}
				else
				{
					m_CurAzimuth = m_OldAzimuth;
				}
				if (fabs(elevation - m_OldElevation) >= CHANGE_THRESH)
				{
					m_OldElevation = elevation;
					m_CurElevation = elevation;
				}
				else
				{
					m_CurElevation = m_OldElevation;
				}
				if (fabs(roll - m_OldRoll) >= CHANGE_THRESH)
				{
					m_OldRoll = roll;
					m_CurRoll = roll;
				}
				else
				{
					m_CurRoll = m_OldRoll;
				}
				if (kn == 0)
				{
					m_Sliceobj->InitSlicePlaneInfo(axisvectest, transformpt.fX, transformpt.fY, transformpt.fZ, m_CurAzimuth, m_OldRoll, m_OldElevation, Regrmat);
					m_Sliceobj->GetDrawingSensorPositionReal();

					Point3f pt_sensor = m_Sliceobj->GetWorldSensorPosition();
					float rmat[9] = { 0.0f };
					/*m_Sliceobj->GetRotationMat(rmat);
					m_RegObj[kn]->SetSensorRotationMat(rmat);
					m_RegObj[kn]->SetSensorPoint(pt_sensor);*/
					m_Sliceobj->GetRotationQuaternion(QuNionTemp[kn]);
					m_RegObj[kn]->SetRotationQuaternion(QuNionTemp[kn]);
					m_RegObj[kn]->SetSensorPoint(pt_sensor);
					printf("\n");
					printf("X ANGLE is %f\n", roll);
					printf("Y ANGLE is %f\n", elevation);
					printf("Z ANGLE is %f\n", azimuth);
				}
				else
				{
					float rmatemp[9] = { 0.0f };
					float rmat[9] = { 0.0f };
					Point3f pt_sensor;
					pt_sensor.x = transformpt.fX;
					pt_sensor.y = transformpt.fY;
					pt_sensor.z = transformpt.fZ;
					/*m_Sliceobj->GetRotationMat(m_CurAzimuth, m_OldRoll, m_OldElevation, rmat);
					m_RegObj[kn]->SetSensorRotationMat(rmat);
					m_RegObj[kn]->SetSensorPoint(pt_sensor);*/
					QuNionTemp[kn].FromEuler(azimuth, roll, elevation, 1);
					m_RegObj[kn]->SetRotationQuaternion(QuNionTemp[kn]);
					m_RegObj[kn]->SetSensorPoint(pt_sensor);
				}
				memcpy(&m_OldRecord[0], &m_CurRecord[0], sizeof(PositionAngleUnit));
			}
		}
	}
	else
	{
		m_ValidSensorCount = 2;
		float x_center = 0.0f;
		float y_center = 0.0f;
		float z_center = 0.0f;
		float azimuth = 0.0f;
		float elevation = 0.0f;
		float roll = 0.0f;
		//m_Count = 90.0f;
		for (int kn = 0; kn < m_ValidSensorCount; kn++)
		{
			if (kn == 0)
			{
				printf("probe punc:m_Count is %f\n", m_Count);
				x_center = 255.0f;
				y_center = 255.0f;
				z_center = (double)GridSize[2] / 2;
				azimuth = (-0.0f*m_Count - m_Azimuthstart)*PI / 180.0;
				elevation = (0.0f - m_Elevationstart)*PI / 180.0;
				roll = (-1.0f*m_Count - m_Rollstart)*PI / 180;

				/*azimuth = 0.0f*PI / 180.0f;
				elevation = 30.0f*PI / 180.0f;
				roll = 0.0f*PI / 180.0f;
				RegQuaternion.FromEuler(0.0f, PI / 2, PI, 1);
				TestQuaternion.FromEuler(azimuth, roll, elevation, 1);
				TempResQuaternion = RegQuaternion.GetQRotationResult(TestQuaternion);
				float xangletest = 0.0f;
				float yangletest = 0.0f;
				float zangletest = 0.0f;
				TempResQuaternion.GetXYZAxisAngleByQuaternion(xangletest, yangletest, zangletest);
				printf("xangletest is %f\n", xangletest);
				printf("yangletest is %f\n", yangletest);
				printf("zangletest is %f\n", zangletest);*/
				if (fabs(azimuth - m_OldAzimuth) >= CHANGE_THRESH)
				{
					m_OldAzimuth = azimuth;
					m_CurAzimuth = azimuth;
				}
				else
				{
					m_CurAzimuth = m_OldAzimuth;
				}
				if (fabs(elevation - m_OldElevation) >= CHANGE_THRESH)
				{
					m_OldElevation = elevation;
					m_CurElevation = elevation;
				}
				else
				{
					m_CurElevation = m_OldElevation;
				}
				if (fabs(roll - m_OldRoll) >= CHANGE_THRESH)
				{
					m_OldRoll = roll;
					m_CurRoll = roll;
				}
				else
				{
					m_CurRoll = m_OldRoll;
				}
				m_Sliceobj->InitSlicePlaneInfo(axisvectest, x_center*m_Pixelspacex, y_center*m_Pixelspacey, z_center*m_Pixelspacez, m_CurAzimuth, m_CurRoll, m_CurElevation, Regrmat);
				m_Sliceobj->GetDrawingSensorPositionDown();
				Point3f pt_sensor = m_Sliceobj->GetWorldSensorPosition();
				m_Sliceobj->GetRotationQuaternion(QuNionTemp[kn]);
				m_RegObj[kn]->SetRotationQuaternion(QuNionTemp[kn]);
				m_RegObj[kn]->SetSensorPoint(pt_sensor);
				m_Count = m_Count + 0.2f;
				if (m_Count >= 180)
				{
					m_Count = 0.0;
				}
			}
			else if (kn == 1)
			{
				printf("needle punc:m_PunCtCount is %f\n", m_PunCtCount);
				x_center = 250.0f;
				y_center = 380.0f;
				z_center = 50.0f;
				azimuth = 1.0f*m_PunCtCount*PI / 180.0;
				elevation = 0.0f*PI / 180.0;
				roll = 0.0f*PI / 180;
				m_PunCtCount = m_PunCtCount + 0.1f;
				if (m_PunCtCount >= 360)
				{
					m_PunCtCount = 0.0f;
				}
				Point3f pt_sensor;
				pt_sensor.x = x_center;
				pt_sensor.y = y_center;
				pt_sensor.z = z_center;
                
				QuNionTemp[kn].FromEuler(azimuth, roll, elevation, 1);
				m_RegObj[kn]->SetRotationQuaternion(QuNionTemp[kn]);
				m_RegObj[kn]->SetSensorPoint(pt_sensor);
			}
			m_CurRecord[kn].x = x_center;
			m_CurRecord[kn].y = y_center;
			m_CurRecord[kn].z = z_center;
			m_CurRecord[kn].a = azimuth;
			m_CurRecord[kn].e = elevation;
			m_CurRecord[kn].r = roll;
		}
	}
}

void RenderBase::GetRegistrationCorInfo(float *rmat, float *tvec)
{
	
}

void RenderBase::InitImageBufAlpha(UINT8 *buf, int width, int height)
{
	memset(m_WorkBuf, 0, sizeof(UINT8)*WIDTH*HEIGHT * 4);
	for (int i = 0; i < height; i++)
	{
		UINT8 *tempbuf = &buf[i*width * 3];
		UINT8 *tempdstbuf = &m_WorkBuf[i*width * 4];
		for (int j = 0; j < width; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				tempdstbuf[4 * j + k] = tempbuf[3 * j + k];
			}
			tempdstbuf[4 * j + 3] = 224;
		}
	}
}

void RenderBase::LoadProbeICon()
{
	char imfilename[256] = { 0 };
	int channels = 3;
	sprintf(imfilename, "%sProbeIcon.dat", m_RegHintFilePath);
	LoadImageFromDat(imfilename, m_ProbeIconImBuf, PROBEICONWIDTH, PROBEICONHEIGHT, channels);
#if 0
	FILE *fp = 0;
	fopen_s(&fp, "D:/Code_local/project_carbon/Configuration/probe_matlab.txt", "w+");
	for (int i = 0; i < PROBEICONHEIGHT; i++)
	{
		for (int j = 0; j < PROBEICONWIDTH; j++)
		{
			for (int k = 0; k < channels; k++)
			{
				fprintf(fp, "%d ", m_ProbeIconImBuf[i*PROBEICONWIDTH*channels + channels*j + k]);
			}
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
#endif
}

void RenderBase::InitRegDirectionImBuffer()
{
	char imfilename[256] = {0};
	int channels = 3;
	for (int i = 0; i < MAXREGDIRECIMNUM-1; i++)
	{
		memset(imfilename, 0, 256 * sizeof(char));
		sprintf(imfilename, "%sdirection%d.dat", m_RegHintFilePath, (i + 1));
		LoadImageFromDat(imfilename, m_RegDirectionImBuf[i], REGDIRECIMWIDTH, REGDIRECIMHEIGHT, channels);
	}
	memset(imfilename, 0, 256 * sizeof(char));
	sprintf(imfilename, "%sdirection%d.dat", m_RegHintFilePath, MAXREGDIRECIMNUM);
	LoadImageFromDat(imfilename, m_RegDirectionImBuf[MAXREGDIRECIMNUM - 1], REGDIRECIMWIDTH, REGDIRECIMHEIGHT, channels);
}

void RenderBase::RegistrationProcessRendering(int orderflag)
{
	//m_RegFinishRenderFlag = false;
	if (m_RegProcessRenderFlag)
	{
		RenderRegHintPBO(m_Texture[2], 0, orderflag);
	}
	if (m_RegFinishRenderFlag)
	{
		m_RegHintFinishWaitTickCount++;
		RenderRegHintPBO(m_Texture[2], 1, orderflag);
		if (m_RegHintFinishWaitTickCount == 100)
		{
			m_RegFinishRenderFlag = false;
			m_RegHintFinishWaitTickCount = 0;
		}
	}
}

void RenderBase::GetHintOriginFlagCor(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt)
{
	vector3d refptworld;
	vector3d refptworldChanged;
	refptworld.fX = m_RefPt1.x + m_UpperLeftX + m_XCenterOffset;
	refptworld.fY = m_RefPt1.y + m_UpperLeftY + m_YCenterOffset;
	refptworld.fZ = m_RefPt1.z + m_UpperLeftZ[0] + m_ZCenterOffset;
	refptworldChanged = refptworld;
	float SensorPosition1 = 0.0f;
	float SensorPosition2 = 0.0f;
	float SensorFUL1 = 0.0f;
	float SensorFUL2 = 0.0f;
	float SensorFUR1 = 0.0f;
	float SensorFUR2 = 0.0f;
	float SensorFDL1 = 0.0f;
	float SensorFDL2 = 0.0f;
	float SensorFDR1 = 0.0f;
	float SensorFDR2 = 0.0f;
	float SensorBUL1 = 0.0f;
	float SensorBUL2 = 0.0f;
	float SensorBUR1 = 0.0f;
	float SensorBUR2 = 0.0f;
	float SensorBDL1 = 0.0f;
	float SensorBDL2 = 0.0f;
	float SensorBDR1 = 0.0f;
	float SensorBDR2 = 0.0f;

	SensorPosition1 = refptworldChanged.fX;
	SensorPosition2 = refptworldChanged.fY+83.0f;

	SensorFUL1 = SensorPosition1 - 32.0f;
	SensorFUL2 = SensorPosition2 - 43.0f;

	SensorFUR1 = SensorPosition1 + 32.0f;
	SensorFUR2 = SensorPosition2 - 43.0f;

	SensorFDL1 = SensorPosition1 - 32.0f;
	SensorFDL2 = SensorPosition2 + 43.0f;

	SensorFDR1 = SensorPosition1 + 32.0f;
	SensorFDR2 = SensorPosition2 + 43.0f;

	tlpt.x = SensorFUL1;
	tlpt.y = SensorFUL2;
	tlpt.z = refptworldChanged.fZ;

	trpt.x = SensorFUR1;
	trpt.y = SensorFUR2;
	trpt.z = refptworldChanged.fZ;

	blpt.x = SensorFDL1;
	blpt.y = SensorFDL2;
	blpt.z = refptworldChanged.fZ;

	brpt.x = SensorFDR1;
	brpt.y = SensorFDR2;
	brpt.z = refptworldChanged.fZ;
}

void RenderBase::GetHintFlagCor(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt)
{
	vector3d refptworld;
	vector3d refptworldChanged;
	refptworld.fX = m_RefPt1.x + m_UpperLeftX + m_XCenterOffset;
	refptworld.fY = m_RefPt1.y + m_UpperLeftY + m_YCenterOffset;
	refptworld.fZ = m_RefPt1.z + m_UpperLeftZ[0] + m_ZCenterOffset;
	refptworldChanged = m_Sliceobj->Camtransmatrix_mult(m_ModelView, refptworld);

	float SensorPosition1 = 0.0f;
	float SensorPosition2 = 0.0f;
	float SensorFUL1 = 0.0f;
	float SensorFUL2 = 0.0f;
	float SensorFUR1 = 0.0f;
	float SensorFUR2 = 0.0f;
	float SensorFDL1 = 0.0f;
	float SensorFDL2 = 0.0f;
	float SensorFDR1 = 0.0f;
	float SensorFDR2 = 0.0f;
	float SensorBUL1 = 0.0f;
	float SensorBUL2 = 0.0f;
	float SensorBUR1 = 0.0f;
	float SensorBUR2 = 0.0f;
	float SensorBDL1 = 0.0f;
	float SensorBDL2 = 0.0f;
	float SensorBDR1 = 0.0f;
	float SensorBDR2 = 0.0f;

	SensorPosition1 = refptworldChanged.fX;
	SensorPosition2 = refptworldChanged.fY;

	SensorFUL1 = SensorPosition1 - 15.0f;
	SensorFUL2 = SensorPosition2 - 30.0f;

	SensorFUR1 = SensorPosition1 + 15.0f;
	SensorFUR2 = SensorPosition2 - 30.0f;

	SensorFDL1 = SensorPosition1 - 15.0f;
	SensorFDL2 = SensorPosition2 + 30.0f;

	SensorFDR1 = SensorPosition1 + 15.0f;
	SensorFDR2 = SensorPosition2 + 30.0f;

	tlpt.x = SensorFUL1;
	tlpt.y = SensorFUL2;
	tlpt.z = refptworldChanged.fZ;

	trpt.x = SensorFUR1;
	trpt.y = SensorFUR2;
	trpt.z = refptworldChanged.fZ;

	blpt.x = SensorFDL1;
	blpt.y = SensorFDL2;
	blpt.z = refptworldChanged.fZ;

	brpt.x = SensorFDR1;
	brpt.y = SensorFDR2;
	brpt.z = refptworldChanged.fZ;
}

void RenderBase::SetHintDataPath(const char *path)
{
	memcpy(m_RegHintFilePath, path, 255 * sizeof(char));
}

void RenderBase::InitRegRefPts(Point3f refpt1, Point3f refpt2, Point3f refpt3, Point3f refpt4)
{
	m_RefPt1 = refpt1;
	m_RefPt2 = refpt2;
	m_RefPt3 = refpt3;
	m_RefPt4 = refpt4;

	m_RefPt1.x = m_RefPt1.x*m_Pixelspacex;
	m_RefPt1.y = m_RefPt1.y*m_Pixelspacey;
	m_RefPt1.z = m_RefPt1.z*m_Pixelspacez;

	m_RefPt2.x = m_RefPt2.x*m_Pixelspacex;
	m_RefPt2.y = m_RefPt2.y*m_Pixelspacey;
	m_RefPt2.z = m_RefPt2.z*m_Pixelspacez;

	m_RefPt3.x = m_RefPt3.x*m_Pixelspacex;
	m_RefPt3.y = m_RefPt3.y*m_Pixelspacey;
	m_RefPt3.z = m_RefPt3.z*m_Pixelspacez;

	m_RefPt4.x = m_RefPt3.x*m_Pixelspacex;
	m_RefPt4.y = m_RefPt3.y*m_Pixelspacey;
	m_RefPt4.z = m_RefPt3.z*m_Pixelspacez;
}

void RenderBase::WaitingForRegFlag()
{
	m_RegObj[0]->WaitingForRegFlag();
}

void RenderBase::InitPunctureLine(float ang, float plength, float refdis, float puncradius, float axis[3], int index)
{
	Quaternion TempQuaternion;
	m_RegObj[index]->SetPuncLineAngle(ang);
	m_RegObj[index]->SetRefDistance(refdis);
	m_RegObj[index]->SetPuncLineLength(plength);
	m_RegObj[index]->SetPuncNeedleRadius(puncradius);
	m_RegObj[index]->SetAxis(axis);
	vector3d basepuncvec(0.0f, 0.0f, 0.0f);
	if (axis[0] == 1.0)
	{
		basepuncvec.fX = cos(ang*PI / 180.0f);
		basepuncvec.fY = sin(ang*PI / 180.0f);
		basepuncvec.fZ = 0.0f;
		TempQuaternion.FromEuler(ang*PI / 180.0f, 0.0f, 0.0f, 1);
		m_RegObj[index]->SetRefAxisRotationQuaternion(TempQuaternion);
		//float refaxismat[9] = { 0.0f };
		//m_Sliceobj->GetRotationMat(ang*PI / 180.0f, 0.0f, 0.0f, refaxismat);
		//m_RegObj[index]->SetRefAxisRotationMat(refaxismat);
	}
	else if (axis[1] == 1.0)
	{
		basepuncvec.fX = sin(ang*PI / 180.0f);
		basepuncvec.fY = cos(ang*PI / 180.0f);
		basepuncvec.fZ = 0.0f;
		TempQuaternion.FromEuler(ang*PI / 180.0f, 0, 0.0f, 1);
		m_RegObj[index]->SetRefAxisRotationQuaternion(TempQuaternion);
		//float refaxismat[9] = { 0.0f };
		//m_Sliceobj->GetRotationMat(ang*PI / 180.0f, 0, 0.0f, refaxismat);
		//m_RegObj[index]->SetRefAxisRotationMat(refaxismat);
	}
	else
	{
		basepuncvec.fX = 0.0f;
		basepuncvec.fY = sin(ang*PI / 180.0f);
		basepuncvec.fZ = cos(ang*PI / 180.0f);
		TempQuaternion.FromEuler(0.0f, ang*PI / 180.0f, 0.0f, 1);
		m_RegObj[index]->SetRefAxisRotationQuaternion(TempQuaternion);
		//float refaxismat[9] = { 0.0f };
		//m_Sliceobj->GetRotationMat(0.0f, ang*PI / 180.0f, 0.0f, refaxismat);
		//m_RegObj[index]->SetRefAxisRotationMat(refaxismat);
	}

	if (index != 0)
	{
		m_RegObj[index]->SetBasePuncvec(basepuncvec, true);
	}
	else
	{
		m_RegObj[index]->SetBasePuncvec(basepuncvec, false);
	}
}

void RenderBase::Registration()
{
	if (m_RegObj[0]->GetRegFinshedFlag())
	{
		m_RegObj[0]->Registration();
	}
}

void RenderBase::RegistrationSimple()
{
	if (m_RegObj[0]->GetRegFinshedFlag())
	{
		m_RegObj[0]->RegistrationSimple();
		//m_RegObj[0]->RegistrationQuaternionSimple();
	}
}

void RenderBase::SetRegStartFlag(bool flag)
{
	m_RegObj[0]->SetRegStartFlag(flag);
}

void RenderBase::SetRegFinshedFlag(bool flag)
{
	m_RegObj[0]->SetRegFinshedFlag(flag);
}

bool RenderBase::GetRegStartFlag()
{
	return m_RegObj[0]->GetRegStartFlag();
}

bool RenderBase::GetRegFinshedFlag()
{
	return m_RegObj[0]->GetRegFinshedFlag();
}

void RenderBase::SetRegistrationPt1(Point3f pt, Point3f pt_match, int ind)
{
	m_RegObj[ind]->SetRegistrationPt1(pt, pt_match);
}
void RenderBase::SetRegistrationPt2(Point3f pt, Point3f pt_match, int ind)
{
	m_RegObj[ind]->SetRegistrationPt1(pt, pt_match);
}
void RenderBase::SetRegistrationPt3(Point3f pt, Point3f pt_match, int ind)
{
	m_RegObj[ind]->SetRegistrationPt1(pt, pt_match);
}
void RenderBase::SetRegistrationPt4(Point3f pt, Point3f pt_match, int ind)
{
	m_RegObj[ind]->SetRegistrationPt1(pt, pt_match);
}

void RenderBase::SetRegistrationPt(int step, int ind)
{
	if (step == REGStep1)
	{
		Point3f pt_temp;
		pt_temp.x = m_CurRecord[0].x;
		pt_temp.y = m_CurRecord[0].y;
		pt_temp.z = m_CurRecord[0].z;
		pt_temp.indsort = 0;
		m_RegObj[ind]->SetRegistrationPt1(pt_temp, m_RefPt1);
	}
	else  if (step == REGStep2)
	{
		Point3f pt_temp;
		pt_temp.x = m_CurRecord[0].x;
		pt_temp.y = m_CurRecord[0].y;
		pt_temp.z = m_CurRecord[0].z;
		pt_temp.indsort = 0;
		m_RegObj[ind]->SetRegistrationPt1(pt_temp, m_RefPt2);
	}
	else  if (step == REGStep3)
	{
		Point3f pt_temp;
		pt_temp.x = m_CurRecord[0].x;
		pt_temp.y = m_CurRecord[0].y;
		pt_temp.z = m_CurRecord[0].z;
		pt_temp.indsort = 0;
		m_RegObj[ind]->SetRegistrationPt1(pt_temp, m_RefPt3);
	}
	else  if (step == REGStep4)
	{
		Point3f pt_temp;
		pt_temp.x = m_CurRecord[0].x;
		pt_temp.y = m_CurRecord[0].y;
		pt_temp.z = m_CurRecord[0].z;
		pt_temp.indsort = 0;
		m_RegObj[ind]->SetRegistrationPt1(pt_temp, m_RefPt4);
	}
}

DWORD RenderBase::InternalThreadEntry()
{
	SetEvent(m_NotifyEvent);
	WaitForSingleObject(m_NotifyEvent, INFINITE);
	InitNDIDriveBay(error_message, error);

	if (error == 0)
	{
		m_NDIConnectedFlag = true;
	}
	else
	{
		m_NDIConnectedFlag = false;
		PrintLog("Init NDI Device Failed");
	}
	m_NDIFinishedFlag = true;
	SetEvent(m_NotifyEvent);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Load raw data from disk
////////////////////////////////////////////////////////////////////////////////
void RenderBase::GetVolumeDataFromFile(const char *filename, short *volume, int totalsize, int gridx)
{
	FILE *fpbintest = fopen(filename, "rb");
	if (!fpbintest)
	{
		fprintf(stderr, "Error opening file '%s'\n", filename);
		return;
	}
	int flag = fread(volume, sizeof(short), totalsize / sizeof(short), fpbintest);

	fclose(fpbintest);
}

void RenderBase::GetVolumeMaskFromFile(const char *filename, double *volume, int totalsize, int gridx)
{
	FILE *fpbintest = fopen(filename, "rb");
	if (!fpbintest)
	{
		fprintf(stderr, "Error opening file '%s'\n", filename);
		return;
	}
	int flag = fread(volume, sizeof(double), totalsize / sizeof(double), fpbintest);

	fclose(fpbintest);
}

void RenderBase::SetModelView(GLfloat modelView[16])
{
	for (int i = 0; i < 16; i++)
	{
		m_ModelView[i] = modelView[i];
	}
}

//set zoom param
void RenderBase::SetZoomParam(GLfloat zoomaspect)
{
	m_ZoomAspect = zoomaspect;
}

void RenderBase::ZoomUp()
{
	m_ScaleParam += 0.1f;
	if (m_ScaleParam >= 2.0f)
	{
		m_ScaleParam = 2.0f;
	}
}

void RenderBase::ZoomDown()
{
	m_ScaleParam -= 0.1f;
	if (m_ScaleParam <= 0.2f)
	{
		m_ScaleParam = 0.2f;
	}
}

void RenderBase::CalcVolumeCenterPt()
{
	m_XCenterOffset = -1.0f * (m_UpperLeftX + (float)(WIDTH / 2)*m_Pixelspacex);
	m_YCenterOffset = -1.0f * (m_UpperLeftY + (float)(HEIGHT / 2)*m_Pixelspacey);
	m_ZCenterOffset = -1.0f * (m_UpperLeftZ[0] + (float)(m_Slicenum / 2)*m_Pixelspacez);

	float xbound1 = 0.0f;
	float xbound2 = (float)(WIDTH)*m_Pixelspacex;

	float ybound1 = 0.0f;
	float ybound2 = (float)(HEIGHT)*m_Pixelspacey;

	float zbound1 = 0.0f;
	float zbound2 = (float)(m_Slicenum)*m_Pixelspacez;

	m_UpperBoundRenderingX = std::fmax(xbound1, xbound2);
	m_BottomBoundRenderingX = std::fmin(xbound1, xbound2);

	m_UpperBoundRenderingY = std::fmax(ybound1, ybound2);
	m_BottomBoundRenderingY = std::fmin(ybound1, ybound2);

	m_UpperBoundRenderingZ = std::fmax(zbound1, zbound2);
	m_BottomBoundRenderingZ = std::fmin(zbound1, zbound2);

	m_EyeFoucusCenterPt.x = 0;
	m_EyeFoucusCenterPt.y = 0;
	m_EyeFoucusCenterPt.z = 0;

	m_EyeCenterPt.x = m_EyeFoucusCenterPt.x;
	m_EyeCenterPt.y = m_EyeFoucusCenterPt.y;
	m_EyeCenterPt.z = m_EyeFoucusCenterPt.z + 10;

	m_HeadDirection.x = 0.0f;
	m_HeadDirection.y = 1.0f;
	m_HeadDirection.z = 0.0f;
	for (int i = 0; i < DEFAULTSENSORCOUNT; i++)
	{
		m_RegObj[i]->InitWorlCorInformation(m_Pixelspacex, m_Pixelspacey, m_Pixelspacez, m_UpperLeftX, m_UpperLeftY, m_UpperLeftZ[0], m_XCenterOffset, m_YCenterOffset, m_ZCenterOffset);
	}
	m_Sliceobj->SetSliceBounds(m_UpperBoundRenderingX, m_UpperBoundRenderingY, m_UpperBoundRenderingZ, m_BottomBoundRenderingX, m_BottomBoundRenderingY, m_BottomBoundRenderingZ);
}

void RenderBase::SetDicomData(float *dicomdata)
{
	int size = GridSize[0] * GridSize[1] * GridSize[2] * sizeof(float);
	m_HostVolume = (float *)malloc(size);
	memcpy(m_HostVolume, dicomdata, size);
}

void RenderBase::SetOrganData(float *maskdata)
{
	m_InterpZstart = m_UpperLeftZ[m_MaskStartNo] - m_InterpVoxelZsize;
	m_pad = 1;
	m_InterpSliceNum = m_InterpSliceNum + 2 * m_pad;
	int padmasksize = GridSize[0] * GridSize[1] * m_InterpSliceNum * sizeof(float);
	int masksize = GridSize[0] * GridSize[1] * (m_InterpSliceNum - 2) * sizeof(float);
	m_Sliceobj->InitSlice(GridSize[0], GridSize[1], GridSize[2]);
	m_Sliceobj->CalcMaxMinValueOfVolume(m_HostVolume);
	float minvalue = m_Sliceobj->GetVolumeMinValue();
	float maxvalue = m_Sliceobj->GetVolumeMaxValue();
	for (int i = 0; i < m_OrganNum; i++)
	{
		m_HostVolumeOrgan[i] = (float *)malloc(padmasksize);
		memset(m_HostVolumeOrgan[i], 0, masksize);
		int offset = i*padmasksize;
		float *temp = m_HostVolumeOrgan[i];
		for (int j = 0; j < GridSize[0] * GridSize[1] * m_pad; j++)
		{
			temp[j] = minvalue;
		}
		for (int j = GridSize[0] * GridSize[1]; j < GridSize[0] * GridSize[1] + GridSize[0] * GridSize[1] * (m_InterpSliceNum - 2 * m_pad); j++)
		{
			temp[j] = maskdata[offset + j - GridSize[0] * GridSize[1]];
		}
		for (int j = GridSize[0] * GridSize[1] * (m_InterpSliceNum - m_pad); j < GridSize[0] * GridSize[1] * m_InterpSliceNum; j++)
		{
			temp[j] = minvalue;
		}
		/*FILE *fmaskdatanormalized = NULL;
		fopen_s(&fmaskdatanormalized, "D:/Code_local/DICOM Reader/marchingcube_matlab/maskdatanormalized.dat", "wb+");
		fwrite(m_HostVolumeOrgan[i], sizeof(float), padmasksize/4, fmaskdatanormalized);
		fclose(fmaskdatanormalized);*/
    }
}

void RenderBase::VolumeDataInit()
{
	//set interp
	SetGLInteropflag(true);
#ifdef SPACE_FILTERING
	m_VoFilterObj->InitVolumeFilter(GridSize[0], GridSize[1], GridSize[2], 0.40f);
#endif

	VoxelSize[0] = m_Pixelspacex;
	VoxelSize[1] = m_Pixelspacey;
	VoxelSize[2] = (m_UpperLeftZ[GridSize[2] - 1] - m_UpperLeftZ[0]) / GridSize[2];
	MaskVoxelSize[0] = m_Pixelspacex;
	MaskVoxelSize[1] = m_Pixelspacex;
	MaskVoxelSize[2] = m_InterpVoxelZsize;
	MaskGridSize[2] = m_InterpSliceNum;
	MaskEndz = m_InterpSliceNum;

	//volume device mem
	//d_volume
	d_volume = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, sizeof(float)*GridSize[0] * GridSize[1] * GridSize[2], 0, &error);
#ifdef MEMST
	int buffer_size = sizeof(short)*GridSize[0] * GridSize[1] * GridSize[2];
	memsumoutside += (float)buffer_size / (1024.0f*1024.0f);
	printf("memsumoutside is %f\n", memsumoutside);
#endif
	check_CL_error(error, "d_volume created failed!");
	clEnqueueWriteBuffer(m_Queue, d_volume, CL_TRUE, 0, sizeof(float)*GridSize[0] * GridSize[1] * GridSize[2], m_HostVolume, 0, 0, 0);
	check_CL_error(error, "d_volume init failed");

	d_volumemask = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, sizeof(float)*GridSize[0] * GridSize[1] * m_InterpSliceNum, 0, &error);
	check_CL_error(error, "d_volumemask created failed!");
#ifdef SPACE_FILTERING
	//d_volumeArray
	cl_image_format volume_formatbase;
	volume_formatbase.image_channel_order = CL_RGBA;
	volume_formatbase.image_channel_data_type = CL_SNORM_INT16;
	d_volumarraybase = clCreateImage3D(m_Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &volume_formatbase,
		GridSize[0], GridSize[1], m_InterpSliceNum,
		(GridSize[0] * sizeof(short)* 4), (GridSize[0] * GridSize[1] * sizeof(short)* 4),
		m_HostVolumeOrganArray[0], &error);
	check_CL_error(error, "Create 3D Texture base Failed");
	SamplerLinear = clCreateSampler(m_Context, true, CL_ADDRESS_REPEAT, CL_FILTER_LINEAR, &error);
#endif

	clEnqueueWriteBuffer(m_Queue, d_volumemask, CL_TRUE, 0, sizeof(float)*GridSize[0] * GridSize[1] * m_InterpSliceNum, m_HostVolumeOrgan[0], 0, 0, 0);
	check_CL_error(error, "d_volume mask init failed");

	unsigned int startoffsetarraysize = sizeof(float) * GridSize[2];
	int upleftznum = sizeof(m_UpperLeftZ);

	d_upperzp = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, upleftznum, 0, &error);
	check_CL_error(error, "d_upperzp created failed!");
	error = clEnqueueWriteBuffer(m_Queue, d_upperzp, CL_TRUE, 0, upleftznum, m_UpperLeftZ, 0, 0, NULL);
	check_CL_error(error, "d_upperzp init failed");

	d_xstartoffset = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, startoffsetarraysize, 0, &error);
	error = clEnqueueWriteBuffer(m_Queue, d_xstartoffset, CL_TRUE, 0, startoffsetarraysize, m_UpperLeftXOffset, 0, 0, NULL);
	check_CL_error(error, "d_xstartoffset init failed");
	d_ystartoffset = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, startoffsetarraysize, 0, &error);
	error = clEnqueueWriteBuffer(m_Queue, d_ystartoffset, CL_TRUE, 0, startoffsetarraysize, m_UpperLeftYOffset, 0, 0, NULL);
	check_CL_error(error, "d_ystartoffset init failed");
	
	Init2DTexturePBO();
	InitRegDirectionImBuffer();
	InitRegProcessHintPBO();
	LoadProbeICon();
	PrintLog("LoadProbeICon done!");
	//calc center pt
	CalcVolumeCenterPt();
}

void RenderBase::GetSliceInfo(int &width, int &height)
{
	width = m_Sliceobj->GetDstWidth();
	height = m_Sliceobj->GetDstHeight();
}

void RenderBase::GetSliceRotationMat(float *rmat)
{
	m_Sliceobj->GetRotationMat(rmat);
}

void RenderBase::GetNomalizedSliceData(float *normslice)
{
	m_Sliceobj->GetSliceNormalized(normslice);
}

void RenderBase::GetSliceDataU8CPU(UINT8 *slicecpu)
{
	m_Sliceobj->GetSliceDataU8(slicecpu);
}

void RenderBase::GetSliceDataU8GPU(cl_mem &sliceu8gpu)
{
	m_Sliceobj->GetSliceDataU8GPU(sliceu8gpu);
}

void RenderBase::ClearSlicePBO(cl_mem &slicepbo)
{
	m_Sliceobj->ClearSlicePbo(slicepbo);
}

void RenderBase::GetSlicePBO(cl_mem &slicepbo, bool resizeflag)
{
	m_Sliceobj->GetSliceDataPBO(slicepbo, resizeflag);
}

void RenderBase::GetCoefsOfCutplane(float &pa, float &pb, float &pc, float &pd)
{
	m_Sliceobj->GetPlaneCoef(pa, pb, pc, pd);
}

//调用rotate中函数，获取切片4个边界上的顶点在CT数据中的真实坐标(外部调用接口)
void RenderBase::GetRealBoundCorOfSlice(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt, int flag)
{
	m_Sliceobj->GetRealBoundCorOfSlice(tlpt, trpt, blpt, brpt, flag);
}

//调用rotate中函数，获取切片4个边界上的顶点在CT数据中的标准坐标(外部调用接口)
void RenderBase::GetStdBoundCorOfSlice(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt, int flag)
{
	m_Sliceobj->GetStdBoundCorOfSlice(tlpt, trpt, blpt, brpt, flag);
}