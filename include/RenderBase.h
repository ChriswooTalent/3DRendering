// RenderBase:  
// A Basic Class for 3D or 2D rendering
// The define of some Global Param here
//    
//  Created by Chriswoo on 2017-08-02.
//  Contact: wsscx01@aliyun.com
// 
#ifndef _RENDERBASE_H_
#define _RENDERBASE_H_

#include "stdafx.h"

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <memory.h>
#include <sys/timeb.h>

#include <io.h>
#include <iostream>
#include <fstream> 
#include <sstream>
#include <string>
#include <cmath>
#include <vector>

#include "DataTypes.h"
#include "Rotateslice.h"
#include "CorRegisTration.h"
#include "Volumefiltering.h"
#include "InternalThread.h"
#include "OpenCLBase.h"
using namespace std;

#ifdef RENDERLIB_EXPORTS  
#define RENDERLIB_API __declspec(dllexport)  
#else  
#define RENDERLIB_API __declspec(dllimport)  
#endif  

#define MAXNUM 512
#define MAXNUMOFORGAN 8
#define MAXREGDIRECIMNUM 8
#define REGDIRECIMWIDTH 128
#define REGDIRECIMHEIGHT 170
#define PROBEICONWIDTH  162
#define PROBEICONHEIGHT 192
#define DEFAULTSENSORCOUNT 4
#define CHANGE_THRESH 0.001745

// PrintLog
extern "C" __declspec(dllimport) void OpenLog();
extern "C" __declspec(dllimport) void CloseLog();
extern "C" __declspec(dllimport) void PrintLog(const char * sysmessage);
 
//绘制基类
class RENDERLIB_API RenderBase : public OpenCLBase, public InternalThread
{
public:
	RenderBase();
	~RenderBase();

	//virtual function
	//texture
	virtual void TextureInit();
	//opengl buffer
	virtual int OpenGLBufferInit();
	//compute
	virtual void Volumecomputing();
	//rendering
	virtual void RenderProcess();
	virtual void GLDataRendering(cl_float translate[4], cl_float clrotate[4], int orderflag);
	virtual void WGLDataRendering(int orderflag);
	//Rendering Feature
	virtual void SetRenderingFeature(RenderParam param);
	virtual void GetRenderingFeature(RenderParam &param);

	//Rendering TexSlice
	virtual void RenderTexSlice(GLuint tex2d, int orderflag);

	//Rendering Hint Flag
	virtual void RenderRegHintPBO(GLuint tex2d, int bufindex, int orderflag);

	//Rendering PunctureLine
	virtual void RenderingPunctureLine();

	//Rendering Probe sensor
	virtual void RenderingProbesensor(GLuint tex2d, int orderflag);

	//Reshape func
	virtual void ReshapeFunc(int w, int h, int texflag);

	//keyboard
	virtual void KeyBoard(unsigned char key);

	//init device
	virtual void InitNDIDevice();

	//animation
	virtual void animation();

	//volumedata
	void VolumeDataInit();

	//Registration Rendering
	void RegistrationProcessRendering(int orderflag);

	//openGL相关操作
	GLuint compileASMShader(GLenum program_type, const char *code);

	//openGL Texture
	void LoadGLU8Textures(UINT8 *imdata, GLuint tex2d, int width, int height);
	void LoadGLU8ColorTextures(UINT8 *imdata, GLuint tex2d, int width, int height, int channels);
	void LoadGLTextures(short *imdata, int width, int height);
	void Init2DTexture();
	void Init2DTexturePBO();
	void InitRegProcessHintPBO();
	void GetPBO();
	void GetRegProcessHintPBO();
	void GetRegProcessFinishedHintPBO();
	void ClearSlicePBO(cl_mem &slicepbo);

	//openGL Pbo
	//PBO
	void createPBO(GLuint &pbo, UINT32 size, cl_mem &pbo_cl, GLuint pboid);
	void deletePBO(GLuint pbo, cl_mem pbo_cl);

	//openGL Lights info
	void SetLightsForRendering();

	//Get Data from file
	void GetVolumeDataFromFile(const char *filename, short *volume, int totalsize, int gridx);

	//Get Data Mask from file
	void GetVolumeMaskFromFile(const char *filename, double *volume, int totalsize, int gridx);

	void SetOrganNum(int nums)
	{
		m_OrganNum = nums;
		//最多只允许绘制8个器官
		if (m_OrganNum >= MAXNUMOFORGAN)
		{
			m_OrganNum = MAXNUMOFORGAN;
		}
	}
	//ModelView
	void SetModelView(GLfloat modelView[16]);

	void SetZoomParam(GLfloat zoomaspect);

	void ZoomUp();

	void ZoomDown();

	void WaitForInitOfNDI();

	//NDI Sensor
	void GetNDIRecord(void *record)
	{
		memcpy(record, &m_CurRecord, sizeof(PositionAngleUnit));
	}

	//WindowSize
	void SetRenderingWindowSize(int width, int height)
	{
		m_RenderWidth = width;
		m_RenderHeight = height;
	}

	//computeflag
	void SetComputeFlag(bool flag)
	{
		m_ComputeFlag = flag;
	}

	//GridSize
	void SetGridSize(int x, int y, int z)
	{
		GridSize[0] = x;
		GridSize[1] = y;
		GridSize[2] = z;
		GridSize[3] = 0;
	}

	void GetGridSize(int &x, int &y, int &z)
	{
		x = GridSize[0];
		y = GridSize[1];
		z = GridSize[2];
	}

	//MaskGridSize
	void SetMaskGridSize(int x, int y, int z)
	{
		MaskGridSize[0] = x;
		MaskGridSize[1] = y;
		MaskGridSize[2] = z;
		MaskGridSize[3] = 0;
	}
	void GetMaskGridSize(int &x, int &y, int &z)
	{
		x = MaskGridSize[0];
		y = MaskGridSize[1];
		z = MaskGridSize[2];
	}

	//计算CT数据中心位置的世界坐标
	void CalcVolumeCenterPt();

	//CT info processing
	void SetDicomInfo(_DICOMInfo *dicomobj, int scnum)
	{
		m_Pixelspacex = dicomobj[0].PixelSpacing_X;
		m_Pixelspacey = dicomobj[0].PixelSpacing_Y;
		m_UpperLeftX = dicomobj[0].UpperLeft_X;
		m_UpperLeftY = dicomobj[0].UpperLeft_Y;
		m_Slicenum = scnum;
		for (int i = 0; i < scnum; i++)
		{
			m_UpperLeftZ[i] = dicomobj[i].UpperLeft_Z;
			m_UpperLeftXOffset[i] = dicomobj[i].UpperLeft_X - dicomobj[0].UpperLeft_X;
			m_UpperLeftYOffset[i] = dicomobj[i].UpperLeft_Y - dicomobj[0].UpperLeft_Y;
		}
		float sb = m_UpperLeftZ[scnum - 1] - m_UpperLeftZ[0];
		m_Pixelspacez = (m_UpperLeftZ[scnum - 1] - m_UpperLeftZ[0]) / (scnum-1);
		m_WindowCenter = dicomobj[0].WindowCenter;
		m_WindowWidth = dicomobj[0].WindowWidth;
		m_Sliceobj->SetCTCorInfo(m_Pixelspacex, m_Pixelspacey, m_UpperLeftX, m_UpperLeftY, m_UpperLeftZ, dicomobj[0].WindowCenter, dicomobj[0].WindowWidth, scnum);
	}

	void SetRoiInfo(float offx, float offy, float offz, float edx, float edy, float edz)
	{
		Offsetx = (UINT32)offx;
		Offsety = (UINT32)offy;
		Offsetz = (UINT32)offz;
		Endx = (UINT32)edx;
		Endy = (UINT32)edy;
		Endz = (UINT32)edz;
	}

	void SetMaskRoiInfo(float offx, float offy, float offz, float edx, float edy, float edz)
	{
		MaskOffsetx = (UINT32)offx;
		MaskOffsety = (UINT32)offy;
		MaskOffsetz = (UINT32)offz;
		MaskEndx = (UINT32)edx;
		MaskEndy = (UINT32)edy;
		MaskEndz = (UINT32)edz;
	}

	//Interopflag
	void SetGLInteropflag(cl_bool flag)
	{
		g_glInterop = flag;
	}

	cl_bool GetGLInteropFlag()
	{
		return g_glInterop;
	}

	void SetDpRenderingFlag(bool skinflag, bool boneflag, bool sliceflag, bool *punctureflag, bool *organflag);

	void SetPboSWFlag(bool flag)
	{
		m_PboSWflag = flag;
	}

	void SetComputeFlag(bool flag, bool flagbm)
	{
		m_ComputeFlag = flag;
		m_ComputeBMFlag = flagbm;
	}

	void GetComputeFlag(bool &flag, bool &flagbm)
	{
		flag = m_ComputeFlag;
		flagbm = m_ComputeBMFlag;
	}

	void SetStartSliceNo(int val)
	{
		m_MaskStartNo = val;
	}
	void SetDicomData(float *dicomdata);
	void SetOrganData(float *maskdata);
	void SetEndSliceNo(int val)
	{
		m_MaskEndNo = val;
	}

	void SetInterpVoxelInfo(int slicenum, float voxelzsize)
	{
		m_InterpSliceNum = slicenum;
		m_InterpVoxelZsize = voxelzsize;
	}

	void SetMeshFilteringParam(int skiniter, int boneiter, int maskIter, bool skinfilterflag, bool bonefilterflag, bool maskfilterflag)
	{
		m_SkinFilteringIterTimes = skiniter;
		m_BoneFilteringIterTimes = boneiter;
		m_MaskFilteringIterTimes = maskIter;
		m_SkinMeshFilteringFlag = skinfilterflag;
		m_BoneMeshFilteringFlag = bonefilterflag;
		m_MaskMeshFilteringFlag = maskfilterflag;
		m_MAXIterTimes = MAX(MAX(m_SkinFilteringIterTimes, m_BoneFilteringIterTimes), m_MaskFilteringIterTimes);
	}

	void SetRenderingColors(float *skincolor, float *bonecolor, float *maskcolor, float *punclinecolor, float *probecolor, int alen)
	{
		assert(alen == 4);
		memcpy(m_SkinColor, skincolor, alen*sizeof(float));
		memcpy(m_BoneColor, bonecolor, alen*sizeof(float));
		memcpy(m_MaskColor, maskcolor, alen*sizeof(float));
		memcpy(m_ProbeColor, probecolor, alen*sizeof(float));
		for (int i = 0; i < DEFAULTSENSORCOUNT; i++)
		{
			memcpy(m_PuncLineColor[i], &punclinecolor[i*DEFAULTSENSORCOUNT], alen*sizeof(float));
		}
	}

	//以下函数基类中只做声明不做实现，继承时实现
	//Get coefs of the cut plane
	void GetCoefsOfCutplane(float &pa, float &pb, float &pc, float &pd);

	//调用rotate中函数，获取切片4个边界上的顶点在CT数据中的真实坐标(外部调用接口)
	void GetRealBoundCorOfSlice(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt, int flag);

	//调用rotate中函数，获取切片4个边界上的顶点在CT数据中的标准坐标(外部调用接口)
	void GetStdBoundCorOfSlice(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt, int flag);

	//Get the size of the slice
	void GetSliceInfo(int &width, int &height);
	void GetSliceRotationMat(float *rmat);

	//Get the SliceData On GPU
	void GetSliceDataU8GPU(cl_mem &sliceu8gpu);
	void GetSlicePBO(cl_mem &slicepbo, bool resizeflag);

	//Get the SliceData On CPU
	void GetNomalizedSliceData(float *normslice);

	//Get Final SliceImage
	void GetSliceDataU8CPU(UINT8 *sliceu8);

	//NDI Device
	void GetRealTimeNDIRecord();
	void GetWorldRecordForReg();

	//Registration Hint
	void LoadProbeICon();
	void InitRegDirectionImBuffer();
	void GetHintOriginFlagCor(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt);
	void GetHintFlagCor(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt);
	void SetHintDataPath(const char *path);
	void InitImageBufAlpha(UINT8 *buf, int width, int height);

	//Registration Process
	void InitRegistrationStartAngle();
	void InitRegRefPts(Point3f refpt1, Point3f refpt2, Point3f refpt3, Point3f refpt4);
	void SetRegistrationPt1(Point3f pt, Point3f pt_match, int ind);
	void SetRegistrationPt2(Point3f pt, Point3f pt_match, int ind);
	void SetRegistrationPt3(Point3f pt, Point3f pt_match, int ind);
	void SetRegistrationPt4(Point3f pt, Point3f pt_match, int ind);
	void InitPunctureLine(float ang, float plength, float refdis, float puncradius, float axis[3], int ind);
	void Registration();
	void RegistrationSimple();
	void SetRegStartFlag(bool flag);
	void SetRegFinshedFlag(bool flag);
	bool GetRegStartFlag();
	bool GetRegFinshedFlag();
	void SetRegistrationPt(int step, int ind);
	void GetRegistrationCorInfo(float *rmat, float *tvec);
	//Waiting for the registration flag
	void WaitingForRegFlag();

protected:
	virtual DWORD InternalThreadEntry();

protected:
	//Rendering Color
	GLfloat m_SkinColor[4];
	GLfloat m_BoneColor[4];
	GLfloat m_MaskColor[4];
	GLfloat m_PuncLineColor[DEFAULTSENSORCOUNT][4];
	GLfloat m_ProbeColor[4];
	//openGL modelview
	GLfloat m_ModelView[16];
	//openGL zoom param
	GLfloat m_ZoomAspect;
	GLfloat m_ScaleParam;
	//openGL Shader
	GLint m_GLShader;
	//openGL Texture
	GLuint m_Texture[3];
	GLuint m_ProbeTexture;
	GLubyte *m_Pixels;
	GLshort *m_Pixels16;
	//openGL Window Size
	int m_RenderWidth;
	int m_RenderHeight;
	//Volume Data Center cor
	Point3 m_EyeCenterPt;
	Point3 m_EyeFoucusCenterPt;
	Point3 m_HeadDirection;
	//volumedatafilename
	string volumedatafile;
	string volumedatamaskfile;
	//gridsize
	cl_uint GridSize[4];
	cl_uint MaskGridSize[4];
	cl_float VoxelSize[4];
	cl_float MaskVoxelSize[4];

	//error message
	char *error_message;
	bool m_NDIConnectedFlag;
	bool m_NDIFinishedFlag;
	int m_ValidSensorCount;
	int m_SensorInd[32];

	float m_Count;
	float m_PunCtCount;

	//slice pbo
	GLuint slicepbo;
	GLuint regpbo;

	//Registration Angle Info
	float m_Rollstart;
	float m_Elevationstart;
	float m_Azimuthstart;
	float m_OldRoll;
	float m_OldElevation;
	float m_OldAzimuth;
	float m_CurRoll;
	float m_CurElevation;
	float m_CurAzimuth;
	//Registration Ref pt
	Point3f m_WorldPt;
	Point3f m_RefPt1;
	Point3f m_RefPt2;
	Point3f m_RefPt3;
	Point3f m_RefPt4;

	//pbo device
	cl_mem m_SlicePboCL;
	cl_mem m_RegPboCL;
	bool m_PboResizeFlag;

	// x y start offset
	cl_mem d_xstartoffset;
	cl_mem d_ystartoffset;
	// OpenCL Mem
	cl_mem d_upperzp;
	cl_mem d_volumarraybase;
	cl_mem d_volume;
	cl_mem d_volumeBone;
	cl_mem d_volumemask;

	//Tex sampler
	cl_sampler SamplerLinear;

	// Kernel running time
	cl_ulong startTime, endTime;
	cl_double kernelExecTimeNs;

	// OpenCL event
	cl_event ndrEvt;

	//ct image related info
	int m_Slicenum;
	float m_Pixelspacex;
	float m_Pixelspacey;
	float m_Pixelspacez;
	float m_UpperLeftX;
	float m_UpperLeftY;
	float m_WindowCenter;
	float m_WindowWidth;
	float m_UpperLeftXOffset[MAXNUM];
	float m_UpperLeftYOffset[MAXNUM];
	float m_UpperLeftZ[MAXNUM];

	cl_bool g_glInterop;

	//不同不为绘制开关
	bool m_SkinRenderFlag;
	bool m_BoneRenderFlag;
	bool m_OrganRenderFlag[MAXNUMOFORGAN];
	bool m_SliceRenderFlag;
	bool m_PunctureLineRenderFlag[DEFAULTSENSORCOUNT];
	bool m_RegProcessRenderFlag;
	bool m_RegFinishRenderFlag;

	//pbo开关
	bool m_PboSWflag;

	//不同器官的配置参数变量，包括MaskRoiFileName以及器官数量参数
	int m_OrganNum;
	vector<string> m_MaskFileNames;

	//给定roi区域的起始和结束坐标
	UINT32 Offsetx;
	UINT32 Offsety;
	UINT32 Offsetz;
	UINT32 Endx;
	UINT32 Endy;
	UINT32 Endz;

	UINT32 MaskOffsetx;
	UINT32 MaskOffsety;
	UINT32 MaskOffsetz;
	UINT32 MaskEndx;
	UINT32 MaskEndy;
	UINT32 MaskEndz;

	//the upper bound and bottom bound of the rendering range
	float m_UpperBoundRenderingX;
	float m_UpperBoundRenderingY;
	float m_UpperBoundRenderingZ;
	float m_BottomBoundRenderingX;
	float m_BottomBoundRenderingY;
	float m_BottomBoundRenderingZ;

	//定义几个偏移量，将体数据的中心平移到（0,0,0）处
	float m_XCenterOffset;
	float m_YCenterOffset;
	float m_ZCenterOffset;

	//定义几个偏移量，定义mask数据的roi区间偏移
	UINT32 m_XGridoffset;
	UINT32 m_YGridoffset;
	UINT32 m_ZGridoffset;

	int m_MaskStartNo;
	int m_MaskEndNo;
	int m_InterpSliceNum;
	int m_pad;
	float m_InterpVoxelZsize;
	float m_InterpZstart;

	//MeshFiltering(网格平滑)
	int m_MAXIterTimes;
	int m_SkinFilteringIterTimes;
	int m_BoneFilteringIterTimes;
	int m_MaskFilteringIterTimes;
	bool m_SkinMeshFilteringFlag;
	bool m_BoneMeshFilteringFlag;
	bool m_MaskMeshFilteringFlag;

	//rendering flags
	bool m_AnimateFlag;
	bool m_ComputeFlag;
	bool m_ComputeBMFlag;
	bool m_ComputeMaskFlag;

	//GPU group info
	// GPU dim 1D
	size_t GlobalWorkSize;
	size_t LocalWorkSize;

	// GPU dim 2D
	size_t _GlobalWorkSize[2];  // global image size
	size_t _LocalWorkSize[2];   // thread num in one work group

	//CorRegisTrationObj
	CorRegisTration *m_RegObj[DEFAULTSENSORCOUNT];
	//RotateSliceObj
	Rotateslice *m_Sliceobj;
	//VolumeFilteringObj
	VolumeFiler *m_VoFilterObj;

	//volume mem on host
	float *m_HostVolume;
	float *m_HostVolumeOrgan[MAXNUMOFORGAN];

	//Registration Direction Image Buffer
	UINT8 *m_RegDirectionImBuf[MAXREGDIRECIMNUM];
	UINT8 *m_ProbeIconImBuf;
	UINT8 *m_WorkBuf;
	char m_RegHintFilePath[255];
	int  m_RegHintChangeTickCount;
	int  m_RegHintFinishWaitTickCount;
	int  m_RegHintIndex;

	PositionAngleUnit m_CurRecord[DEFAULTSENSORCOUNT];
	PositionAngleUnit m_OldRecord[DEFAULTSENSORCOUNT];
};
#endif // !_RENDERBASE_H_
