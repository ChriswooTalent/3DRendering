// Rotateslice:  
// Get the slice image of the volumedata from any angle  
//    
//  Created by Chriswoo on 2017-08-02.
//  Contact: wsscx01@aliyun.com
// 
#include "stdafx.h"
#include "Rotateslice.h"
#include "ScanApple.h"
#include <iostream> 
#include <fstream> 
#include <sstream>

using namespace std;
 
enum SliceKernelMethods
{
	SLICEYZX = 0,
	SLICEXZY = 1,
	SLICEXYZ = 2,
	GETSLICEPBO = 3,
	CopySlice = 4,
	GETRESIZESLICEPBO = 5,
	CLEARPBOBUF = 6,
	CLEARSLICEPBO = 7,
	GETWORLDSLICECORXYZ = 8
};
 
static const char* SliceKernelNames[] =
{
	"InterpSliceYZX",
	"InterpSliceXZY",
	"InterpSliceXYZ",
	"GetSlicePBO",
	"GetSliceDirectly",
	"GetRESIZESLICEPBO",
	"ClearPBOBuffer",
	"ClearSlicePbo",
	"GetWorldSliceCorXYZ"
};

Rotateslice::Rotateslice()
{
	m_Width = WIDTH;
	m_Height = HEIGHT;
	m_Slicenum = NSLICE;
	m_MaxValue = 0;
	m_MinValue = 0;
	m_DstWidth = 0;
	m_DstHeight = 0;
	m_VHeight = 0;
	m_Zind = 0;
	m_KernelCount = sizeof(SliceKernelNames) / sizeof(char *);
	m_kernelFile = (char *)SliceKernelFile;
	m_CtPixelSpacex = 0.0f;
	m_CtPixelSpacey = 0.0f;
	m_CtPixelSpacez = 0.0f;
	m_CtUpperLeft_X = 0.0f;
	m_CtUpperLeft_Y = 0.0f;
	m_CtWinCenter = 0.0f;
	m_CtWinWidth = 0.0f;
	vmin = 0;
	vmax = 0;
	m_Xmid = 0.0f;
	m_Ymid = 0.0f;
	m_Zmid = 0.0f;
	m_XaxisAngle = 0.0f;
	m_YaxisAngle = 0.0f;
	m_ZaxisAngle = 0.0f;
	m_Normvecx = 0.0f;
	m_Normvecy = 0.0f;
	m_Normvecz = 0.0f;
	memset(dstplane, 0, WIDTH*HEIGHT*sizeof(short));
	m_HSliceImageU8 = NULL;
	m_DSliceImageU8 = 0;
	m_DSliceZero = 0;
	m_DSliceZeroScan = 0;
	//Rendering bounds
	m_UpperBoundRenderingX = 0.0f;
	m_UpperBoundRenderingY = 0.0f;
	m_UpperBoundRenderingZ = 0.0f;
	m_BottomBoundRenderingX = 0.0f;
	m_BottomBoundRenderingY = 0.0f;
	m_BottomBoundRenderingZ = 0.0f;
	//interp and copy flag
	m_Interpflag = 0;
	m_Directcopyflag = 0;
	//init gridsize
	for (int i = 0; i < 4; i++)
	{
		GridSize[i] = 0;
		Resizeratio[i] = 0.0f;
		VoxelSize[i] = 0.0f;
	}

	//Rotation Mat
	for (int i = 0; i < 9; i++)
	{
		m_RMat[i] = 0.0f;
		m_TempRMat[i] = 0.0f;
	}
	m_RMat[0] = 1.0f;
	m_RMat[4] = 1.0f;
	m_RMat[8] = 1.0f;
	m_TempRMat[0] = 1.0f;
	m_TempRMat[4] = 1.0f;
	m_TempRMat[8] = 1.0f;
	m_ProbeUpdirectionFlag = false;
	m_ProbeLRDirectionOutFlag = 0;
}

Rotateslice::~Rotateslice()
{
	if (m_HSliceImageU8)
	{
		free(m_HSliceImageU8);
		m_HSliceImageU8 = NULL;
	}
	if (m_HSliceImagefloat)
	{
		free(m_HSliceImagefloat);
		m_HSliceImagefloat = NULL;
	}
}

void Rotateslice::KernelInit()
{
}


void Rotateslice::SliceOpenCLini(cl_context cxGPUContext, cl_command_queue cqCommandQueue, cl_device_id device)
{
	m_Context = cxGPUContext;
	m_Queue = cqCommandQueue;
	m_Device = device;
	BuildKernel(SliceKernelNames);
}

void Rotateslice::RotateSliceOpenCLRelease()
{
	if (m_DSliceImageU8)
		clReleaseMemObject(m_DSliceImageU8);
	if (m_DSliceImagefloat)
		clReleaseMemObject(m_DSliceImagefloat);
	if (m_DSliceZero)
		clReleaseMemObject(m_DSliceZero);
	if (m_DSliceZeroScan)
		clReleaseMemObject(m_DSliceZeroScan);
}

void Rotateslice::InitSlice(int width, int height, int slicenum)
{
	GridSize[0] = width;
	GridSize[1] = height;
	GridSize[2] = slicenum;
	GridSize[3] = 0;
	m_Width = GridSize[0];
	m_Height = GridSize[1];
	m_Slicenum = GridSize[2];
	CalcInterpGridSize();
	int volumesize = GridSize[0] * GridSize[1] * GridSize[2] * sizeof(float);
	int volumesizeuchar = GridSize[0] * GridSize[1] * GridSize[2] * sizeof(UINT8);
	int imagesizeu8 = WIDTH*HEIGHT*sizeof(UINT8);
	int imagesizefloat = WIDTH*HEIGHT*sizeof(float);

	m_HSliceImageU8 = (UINT8 *)malloc(imagesizeu8);
	m_HSliceImagefloat = (float *)malloc(imagesizefloat);
	//slice device mem;
	m_DSliceImageU8 = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, imagesizeu8, 0, &error);
	check_CL_error(error, "SliceImageU8 created failed!");
	m_DSliceImagefloat = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, imagesizefloat, 0, &error);
	check_CL_error(error, "SliceImagefloat created failed!");
	m_DSliceZero = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, imagesizefloat, 0, &error);
	check_CL_error(error, "m_DSliceZero created failed!");
	m_DSliceZeroScan = clCreateBuffer(m_Context, CL_MEM_READ_WRITE, imagesizefloat, 0, &error);
	check_CL_error(error, "m_DSliceZeroScan created failed!");
}

void Rotateslice::GetRotationMat(float *rmat)
{
	memcpy(rmat, m_RMat, 9 * sizeof(float));
}

void Rotateslice::GetRotationMat(float azimuth, float roll, float elevation, float *RMat)
{
	float Rmatx[9] = { 0.0f };
	float Rmaty[9] = { 0.0f };
	float Rmatz[9] = { 0.0f };
	float RmatTemp[9] = { 0.0f };

	Rmatx[0] = 1.0f;
	Rmatx[4] = cos(roll);
	Rmatx[5] = sin(roll);
	Rmatx[7] = -sin(roll);
	Rmatx[8] = cos(roll);

	Rmaty[0] = cos(elevation);
	Rmaty[2] = -sin(elevation);
	Rmaty[4] = 1.0f;
	Rmaty[6] = sin(elevation);
	Rmaty[8] = cos(elevation);

	Rmatz[0] = cos(azimuth);
	Rmatz[1] = sin(azimuth);
	Rmatz[3] = -sin(azimuth);
	Rmatz[4] = cos(azimuth);
	Rmatz[8] = 1.0f;

	matrix_matrix_mult(Rmatx, Rmaty, RmatTemp);
	matrix_matrix_mult(RmatTemp, Rmatz, RMat);
	/*matrix_matrix_mult(Rmatz, Rmaty, RmatTemp);
	matrix_matrix_mult(RmatTemp, Rmatx, RMat);*/
}

void Rotateslice::GetRotationMatDirectly(float azimuth, float roll, float elevation, float *RMat)
{
	float RmatTemp[9] = { 0.0f };
	float SINX = sin(roll);
	float COSX = cos(roll);
	float SINY = sin(elevation);
	float COSY = cos(elevation);
	float SINZ = sin(azimuth);
	float COSZ = cos(azimuth);

	RmatTemp[0] = COSY*COSZ;
	RmatTemp[1] = COSY*SINZ;
	RmatTemp[2] = -SINY;
	RmatTemp[3] = -COSX*SINZ + SINX*SINY*COSZ;
	RmatTemp[4] = COSX*COSZ + SINX*SINY*SINZ;
	RmatTemp[5] = SINX*COSY;
	RmatTemp[6] = SINX*SINZ + COSX*SINY*COSZ;
	RmatTemp[7] = -SINX*COSZ + COSX*SINY*SINZ;
	RmatTemp[8] = COSX*COSY;

	memcpy(RMat, RmatTemp, 9 * sizeof(float));
}

void Rotateslice::matrix_matrix_mult(float *m1, float *m2, float *result)
{
	register int i, k;

	for (i = 0; i < 3; i++)
	{
		for (k = 0; k < 3; k++)
		{
			result[i*3+k] = m1[i*3] * m2[k] + m1[i*3+1] * m2[3+k] + m1[i*3+2] * m2[6+k];
		}
	}
}

vector3d Rotateslice::matrix_mult(float *mat, vector3d a)
{
	float v0 = a.fX;
	float v1 = a.fY;
	float v2 = a.fZ;
	vector3d mulresult;

	mulresult.fX = mat[0] * v0 + mat[1] * v1 + mat[2] * v2;
	mulresult.fY = mat[3] * v0 + mat[4] * v1 + mat[5] * v2;
	mulresult.fZ = mat[6] * v0 + mat[7] * v1 + mat[8] * v2;
	return mulresult;
}

vector3d Rotateslice::Camtransmatrix_mult(float *mat, vector3d a)
{
	float v0 = a.fX;
	float v1 = a.fY;
	float v2 = a.fZ;
	float v3 = 1.0f;
	vector3d mulresult;

	mulresult.fX = mat[0] * v0 + mat[1] * v1 + mat[2] * v2 + mat[3] * v3;
	mulresult.fY = mat[4] * v0 + mat[5] * v1 + mat[6] * v2 + mat[7] * v3;
	mulresult.fZ = mat[8] * v0 + mat[9] * v1 + mat[10] * v2 + mat[11] * v3;
	return mulresult;
}

void Rotateslice::matrix_matrix_mult(double m1[3][3], double m2[3][3], double result[3][3])
{
	register int i, k;

	for (i = 0; i < 3; i++)
	{
		for (k = 0; k < 3; k++)
		{
			result[i][k] = m1[i][0] * m2[0][k] + m1[i][1] * m2[1][k] + m1[i][2] * m2[2][k];
		}
	}
}

//向量叉乘
vector3d Rotateslice::cross(vector3d a, vector3d b)
{
	vector3d provec;
	provec.fX = a.fY*b.fZ - a.fZ*b.fY;
	provec.fY = a.fZ*b.fX - a.fX*b.fZ;
	provec.fZ = a.fX*b.fY - a.fY*b.fX;
	return provec;
}

//计算顶点法向量
vector3d Rotateslice::calcNormal(vector3d &v0, vector3d &v1, vector3d &v2)
{
	vector3d edge0;
	edge0.fX = v1.fX - v0.fX;
	edge0.fY = v1.fY - v0.fY;
	edge0.fZ = v1.fZ - v0.fZ;
	vector3d edge1;
	edge1.fX = v2.fX - v0.fX;
	edge1.fY = v2.fY - v0.fY;
	edge1.fZ = v2.fZ - v0.fZ;

	// note - it's faster to perform normalization in vertex shader rather than here
	return cross(edge0, edge1);
}

float Rotateslice::CalcNValueofVec(vector3d v)
{
	return sqrt(v.fX*v.fX + v.fY*v.fY + v.fZ*v.fZ);
}

float Rotateslice::Dot3D(vector3d a, vector3d b)
{
	float dotresult = 0.0f;
	dotresult = a.fX*b.fX + a.fY*b.fY + a.fZ*b.fZ;
	return dotresult;
}

void Rotateslice::CalcMaxMinValueOfVolume(float *volume)
{
	int totalpts = m_Width*m_Height*m_Slicenum;
	float maxval = -4999.0f;
	float minval = 4999.0f;
	for (int i = 0; i < totalpts; i++)
	{
		if (volume[i] > maxval)
		{
			maxval = volume[i];
		}
		if (volume[i] < minval)
		{
			minval = volume[i];
		}
	}
	m_MaxValue = maxval;
	m_MinValue = minval;
}

Point3f Rotateslice::GetStdCorYZX(float a, float b, float c, float y, float z, Point3f ptOnplane)
{
	float x = -1.0f*((c*(z - ptOnplane.z) + b*(y - ptOnplane.y)) / a - ptOnplane.x);
	Point3f temp_pt;
	temp_pt.x = x;
	temp_pt.y = y;
	temp_pt.z = z;
	return temp_pt;
}
Point3f Rotateslice::GetStdCorXZY(float a, float b, float c, float x, float z, Point3f ptOnplane)
{
	float y = -1.0f*((c*(z - ptOnplane.z) + a*(x - ptOnplane.x)) / b - ptOnplane.y);
	Point3f temp_pt;
	temp_pt.x = (float)x;
	temp_pt.y = (float)y;
	temp_pt.z = (float)z;
	return temp_pt;
}
Point3f Rotateslice::GetStdCorXYZ(float a, float b, float c, float x, float y, Point3f ptOnplane)
{
	float z = -1.0f*((a*(x - ptOnplane.x) + b*(y - ptOnplane.y)) / c - ptOnplane.z);
	Point3f temp_pt;
	temp_pt.x = x;
	temp_pt.y = y;
	temp_pt.z = z;
	return temp_pt;
}

Point3f Rotateslice::GetWorldSensorPosition()
{
	return m_WorldSensorPt;
}

Point3f Rotateslice::GetWorldSensorUlPt()
{
	return m_WorldSensorFULPt;
}
Point3f Rotateslice::GetWorldSensorUrPt()
{
	return m_WorldSensorFURPt;
}
Point3f Rotateslice::GetWorldSensorDlPt()
{
	return m_WorldSensorFDLPt;
}
Point3f Rotateslice::GetWorldSensorDrPt()
{
	return m_WorldSensorFDRPt;
}

void Rotateslice::GetDrawingSensorPositionReal()
{
	Point3f ptop(m_Xmid, m_Ymid, m_Zmid, 0);
	float SensorOriPosition1 = 0.0f;
	float SensorOriPosition2 = 0.0f;
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
	if (1 == m_Interpflag)
	{
		SensorOriPosition1 = ptop.y + 25.0f;
		SensorOriPosition2 = ptop.z;

		SensorPosition1 = SensorOriPosition1 + 40.0f;
		SensorPosition2 = SensorOriPosition2;

		SensorFUL1 = SensorPosition1 - 35.0f;
		SensorFUL2 = SensorPosition2 - 40.0f;

		SensorFUR1 = SensorPosition1 + 35.0f;
		SensorFUR2 = SensorPosition2 - 40.0f;

		SensorFDL1 = SensorPosition1 - 35.0f;
		SensorFDL2 = SensorPosition2 + 40.0f;

		SensorFDR1 = SensorPosition1 + 35.0f;
		SensorFDR2 = SensorPosition2 + 40.0f;
		m_StdSensorPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorOriPosition1, SensorOriPosition2, ptop);
		m_WorldSensorFULPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFUL1, SensorFUL2, ptop);
		m_WorldSensorFURPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFUR1, SensorFUR2, ptop);
		m_WorldSensorFDLPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFDL1, SensorFDL2, ptop);
		m_WorldSensorFDRPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFDR1, SensorFDR2, ptop);
	}
	else if (2 == m_Interpflag)
	{
		SensorOriPosition1 = ptop.x;
		SensorOriPosition2 = ptop.z+ 25.0f;

		SensorPosition1 = SensorOriPosition1;
		SensorPosition2 = SensorOriPosition2 + 40.0f;

		SensorFUL1 = SensorPosition1 - 35.0f;
		SensorFUL2 = SensorPosition2 - 40.0f;

		SensorFUR1 = SensorPosition1 + 35.0f;
		SensorFUR2 = SensorPosition2 - 40.0f;

		SensorFDL1 = SensorPosition1 - 35.0f;
		SensorFDL2 = SensorPosition2 + 40.0f;

		SensorFDR1 = SensorPosition1 + 35.0f;
		SensorFDR2 = SensorPosition2 + 40.0f;
		m_StdSensorPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorOriPosition1, SensorOriPosition2, ptop);
		m_WorldSensorFULPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFUL1, SensorFUL2, ptop);
		m_WorldSensorFURPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFUR1, SensorFUR2, ptop);
		m_WorldSensorFDLPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFDL1, SensorFDL2, ptop);
		m_WorldSensorFDRPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFDR1, SensorFDR2, ptop);
	}
	else if (3 == m_Interpflag)
	{
		SensorOriPosition1 = ptop.x;
		SensorOriPosition2 = ptop.y + 25.0f;

		SensorPosition1 = SensorOriPosition1;
		SensorPosition2 = SensorOriPosition2 + 40.0f;

		SensorFUL1 = SensorPosition1 - 35.0f;
		SensorFUL2 = SensorPosition2 - 40.0f;

		SensorFUR1 = SensorPosition1 + 35.0f;
		SensorFUR2 = SensorPosition2 - 40.0f;

		SensorFDL1 = SensorPosition1 - 35.0f;
		SensorFDL2 = SensorPosition2 + 40.0f;

		SensorFDR1 = SensorPosition1 + 35.0f;
		SensorFDR2 = SensorPosition2 + 40.0f;
		m_StdSensorPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorOriPosition1, SensorOriPosition2, ptop);
		m_WorldSensorFULPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFUL1, SensorFUL2, ptop);
		m_WorldSensorFURPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFUR1, SensorFUR2, ptop);
		m_WorldSensorFDLPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFDL1, SensorFDL2, ptop);
		m_WorldSensorFDRPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFDR1, SensorFDR2, ptop);
	}
	m_WorldSensorPt.x = m_StdSensorPt.x;
	m_WorldSensorPt.y = m_StdSensorPt.y;
	m_WorldSensorPt.z = m_StdSensorPt.z;

	m_WorldSensorFULPt.x = m_WorldSensorFULPt.x;
	m_WorldSensorFULPt.y = m_WorldSensorFULPt.y;
	m_WorldSensorFULPt.z = m_WorldSensorFULPt.z;

	m_WorldSensorFURPt.x = m_WorldSensorFURPt.x;
	m_WorldSensorFURPt.y = m_WorldSensorFURPt.y;
	m_WorldSensorFURPt.z = m_WorldSensorFURPt.z;

	m_WorldSensorFDLPt.x = m_WorldSensorFDLPt.x;
	m_WorldSensorFDLPt.y = m_WorldSensorFDLPt.y;
	m_WorldSensorFDLPt.z = m_WorldSensorFDLPt.z;

	m_WorldSensorFDRPt.x = m_WorldSensorFDRPt.x;
	m_WorldSensorFDRPt.y = m_WorldSensorFDRPt.y;
	m_WorldSensorFDRPt.z = m_WorldSensorFDRPt.z;
}

void Rotateslice::GetDrawingSensorPositionDown()
{
	Point3f ptop(m_Xmid, m_Ymid, m_Zmid, 0);
	float SensorOriPosition1 = 0.0f;
	float SensorOriPosition2 = 0.0f;
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
	if (1 == m_Interpflag)
	{
		SensorOriPosition1 = GridSize[1];
		SensorOriPosition2 = GridSize[2] / 2.0f;

		SensorPosition1 = GridSize[1] + 40.0f;
		SensorPosition2 = GridSize[2] / 2.0f;

		SensorFUL1 = SensorPosition1 - 40.0f;
		SensorFUL2 = SensorPosition2 - 9.0f;

		SensorFUR1 = SensorPosition1 + 40.0f;
		SensorFUR2 = SensorPosition2 - 9.0f;

		SensorFDL1 = SensorPosition1 - 40.0f;
		SensorFDL2 = SensorPosition2 + 9.0f;

		SensorFDR1 = SensorPosition1 + 40.0f;
		SensorFDR2 = SensorPosition2 + 9.0f;
		m_StdSensorPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorOriPosition1*m_CtPixelSpacey, SensorOriPosition2*m_CtPixelSpacez, ptop);
		m_WorldSensorFULPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFUL1*m_CtPixelSpacey, SensorFUL2*m_CtPixelSpacez, ptop);
		m_WorldSensorFURPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFUR1*m_CtPixelSpacey, SensorFUR2*m_CtPixelSpacez, ptop);
		m_WorldSensorFDLPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFDL1*m_CtPixelSpacey, SensorFDL2*m_CtPixelSpacez, ptop);
		m_WorldSensorFDRPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFDR1*m_CtPixelSpacey, SensorFDR2*m_CtPixelSpacez, ptop);
	}
	else if (2 == m_Interpflag)
	{
		SensorOriPosition1 = GridSize[0] / 2.0f;
		SensorOriPosition2 = GridSize[2];

		SensorPosition1 = GridSize[0] / 2.0f;
		SensorPosition2 = GridSize[2] + 9.0f;

		SensorFUL1 = SensorPosition1 - 40.0f;
		SensorFUL2 = SensorPosition2 - 9.0f;

		SensorFUR1 = SensorPosition1 + 40.0f;
		SensorFUR2 = SensorPosition2 - 9.0f;

		SensorFDL1 = SensorPosition1 - 40.0f;
		SensorFDL2 = SensorPosition2 + 9.0f;

		SensorFDR1 = SensorPosition1 + 40.0f;
		SensorFDR2 = SensorPosition2 + 9.0f;
		m_StdSensorPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorOriPosition1*m_CtPixelSpacex, SensorOriPosition2*m_CtPixelSpacez, ptop);
		m_WorldSensorFULPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFUL1*m_CtPixelSpacex, SensorFUL2*m_CtPixelSpacez, ptop);
		m_WorldSensorFURPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFUR1*m_CtPixelSpacex, SensorFUR2*m_CtPixelSpacez, ptop);
		m_WorldSensorFDLPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFDL1*m_CtPixelSpacex, SensorFDL2*m_CtPixelSpacez, ptop);
		m_WorldSensorFDRPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFDR1*m_CtPixelSpacex, SensorFDR2*m_CtPixelSpacez, ptop);
	}
	else if (3 == m_Interpflag)
	{
		SensorOriPosition1 = GridSize[0] / 2.0f;
		SensorOriPosition2 = GridSize[1];

		SensorPosition1 = GridSize[0] / 2.0f;
		SensorPosition2 = GridSize[1] + 40.0f;

		SensorFUL1 = SensorPosition1 - 40.0f;
		SensorFUL2 = SensorPosition2 - 40.0f;

		SensorFUR1 = SensorPosition1 + 40.0f;
		SensorFUR2 = SensorPosition2 - 40.0f;

		SensorFDL1 = SensorPosition1 - 40.0f;
		SensorFDL2 = SensorPosition2 + 40.0f;

		SensorFDR1 = SensorPosition1 + 40.0f;
		SensorFDR2 = SensorPosition2 + 40.0f;
		m_StdSensorPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorOriPosition1*m_CtPixelSpacex, SensorOriPosition2*m_CtPixelSpacey, ptop);
		m_WorldSensorFULPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFUL1*m_CtPixelSpacex, SensorFUL2*m_CtPixelSpacey, ptop);
		m_WorldSensorFURPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFUR1*m_CtPixelSpacex, SensorFUR2*m_CtPixelSpacey, ptop);
		m_WorldSensorFDLPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFDL1*m_CtPixelSpacex, SensorFDL2*m_CtPixelSpacey, ptop);
		m_WorldSensorFDRPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFDR1*m_CtPixelSpacex, SensorFDR2*m_CtPixelSpacey, ptop);
	}
	m_WorldSensorPt.x = m_StdSensorPt.x;
	m_WorldSensorPt.y = m_StdSensorPt.y;
	m_WorldSensorPt.z = m_StdSensorPt.z;

	m_WorldSensorFULPt.x = m_WorldSensorFULPt.x;
	m_WorldSensorFULPt.y = m_WorldSensorFULPt.y;
	m_WorldSensorFULPt.z = m_WorldSensorFULPt.z;

	m_WorldSensorFURPt.x = m_WorldSensorFURPt.x;
	m_WorldSensorFURPt.y = m_WorldSensorFURPt.y;
	m_WorldSensorFURPt.z = m_WorldSensorFURPt.z;

	m_WorldSensorFDLPt.x = m_WorldSensorFDLPt.x;
	m_WorldSensorFDLPt.y = m_WorldSensorFDLPt.y;
	m_WorldSensorFDLPt.z = m_WorldSensorFDLPt.z;

	m_WorldSensorFDRPt.x = m_WorldSensorFDRPt.x;
	m_WorldSensorFDRPt.y = m_WorldSensorFDRPt.y;
	m_WorldSensorFDRPt.z = m_WorldSensorFDRPt.z;
}

void Rotateslice::GetDrawingSensorPositionUp()
{
	Point3f ptop(m_Xmid, m_Ymid, m_Zmid, 0);
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
	if (1 == m_Interpflag)
	{
		SensorPosition1 = -20;
		SensorPosition2 = GridSize[2] / 2.0f;

		SensorFUL1 = SensorPosition1 - 10.0f;
		SensorFUL2 = SensorPosition2 - 2.0f;

		SensorFUR1 = SensorPosition1 + 10.0f;
		SensorFUR2 = SensorPosition2 - 2.0f;

		SensorFDL1 = SensorPosition1 - 10.0f;
		SensorFDL2 = SensorPosition2 + 2.0f;

		SensorFDR1 = SensorPosition1 + 10.0f;
		SensorFDR2 = SensorPosition2 + 2.0f;
		m_StdSensorPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorPosition1*m_CtPixelSpacey, SensorPosition2*m_CtPixelSpacez, ptop);
		m_WorldSensorFULPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFUL1*m_CtPixelSpacey, SensorFUL2*m_CtPixelSpacez, ptop);
		m_WorldSensorFURPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFUR1*m_CtPixelSpacey, SensorFUR2*m_CtPixelSpacez, ptop);
		m_WorldSensorFDLPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFDL1*m_CtPixelSpacey, SensorFDL2*m_CtPixelSpacez, ptop);
		m_WorldSensorFDRPt = GetStdCorYZX(m_Pa, m_Pb, m_Pc, SensorFDR1*m_CtPixelSpacey, SensorFDR2*m_CtPixelSpacez, ptop);
	}
	else if (2 == m_Interpflag)
	{
		SensorPosition1 = GridSize[0] / 2.0f;
		SensorPosition2 = -2.0f;

		SensorFUL1 = SensorPosition1 - 10.0f;
		SensorFUL2 = SensorPosition2 - 2.0f;

		SensorFUR1 = SensorPosition1 + 10.0f;
		SensorFUR2 = SensorPosition2 - 2.0f;

		SensorFDL1 = SensorPosition1 - 10.0f;
		SensorFDL2 = SensorPosition2 + 2.0f;

		SensorFDR1 = SensorPosition1 + 10.0f;
		SensorFDR2 = SensorPosition2 + 2.0f;
		m_StdSensorPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorPosition1*m_CtPixelSpacex, SensorPosition2*m_CtPixelSpacez, ptop);
		m_WorldSensorFULPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFUL1*m_CtPixelSpacex, SensorFUL2*m_CtPixelSpacez, ptop);
		m_WorldSensorFURPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFUR1*m_CtPixelSpacex, SensorFUR2*m_CtPixelSpacez, ptop);
		m_WorldSensorFDLPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFDL1*m_CtPixelSpacex, SensorFDL2*m_CtPixelSpacez, ptop);
		m_WorldSensorFDRPt = GetStdCorXZY(m_Pa, m_Pb, m_Pc, SensorFDR1*m_CtPixelSpacex, SensorFDR2*m_CtPixelSpacez, ptop);
	}
	else if (3 == m_Interpflag)
	{
		SensorPosition1 = GridSize[0] / 2.0f;
		SensorPosition2 = 50.0f;

		SensorFUL1 = SensorPosition1 - 10.0f;
		SensorFUL2 = SensorPosition2 - 10.0f;

		SensorFUR1 = SensorPosition1 + 10.0f;
		SensorFUR2 = SensorPosition2 - 10.0f;

		SensorFDL1 = SensorPosition1 - 10.0f;
		SensorFDL2 = SensorPosition2 + 10.0f;

		SensorFDR1 = SensorPosition1 + 10.0f;
		SensorFDR2 = SensorPosition2 + 10.0f;
		m_StdSensorPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorPosition1*m_CtPixelSpacex, SensorPosition2*m_CtPixelSpacey, ptop);
		m_WorldSensorFULPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFUL1*m_CtPixelSpacex, SensorFUL2*m_CtPixelSpacey, ptop);
		m_WorldSensorFURPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFUR1*m_CtPixelSpacex, SensorFUR2*m_CtPixelSpacey, ptop);
		m_WorldSensorFDLPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFDL1*m_CtPixelSpacex, SensorFDL2*m_CtPixelSpacey, ptop);
		m_WorldSensorFDRPt = GetStdCorXYZ(m_Pa, m_Pb, m_Pc, SensorFDR1*m_CtPixelSpacex, SensorFDR2*m_CtPixelSpacey, ptop);
	}
	m_WorldSensorPt.x = m_StdSensorPt.x;
	m_WorldSensorPt.y = m_StdSensorPt.y;
	m_WorldSensorPt.z = m_StdSensorPt.z;

	m_WorldSensorFULPt.x = m_WorldSensorFULPt.x;
	m_WorldSensorFULPt.y = m_WorldSensorFULPt.y;
	m_WorldSensorFULPt.z = m_WorldSensorFULPt.z;

	m_WorldSensorFURPt.x = m_WorldSensorFURPt.x;
	m_WorldSensorFURPt.y = m_WorldSensorFURPt.y;
	m_WorldSensorFURPt.z = m_WorldSensorFURPt.z;

	m_WorldSensorFDLPt.x = m_WorldSensorFDLPt.x;
	m_WorldSensorFDLPt.y = m_WorldSensorFDLPt.y;
	m_WorldSensorFDLPt.z = m_WorldSensorFDLPt.z;

	m_WorldSensorFDRPt.x = m_WorldSensorFDRPt.x;
	m_WorldSensorFDRPt.y = m_WorldSensorFDRPt.y;
	m_WorldSensorFDRPt.z = m_WorldSensorFDRPt.z;
}

void Rotateslice::GetStdBoundCorOfSlice(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt, int flag)
{
	tlpt = m_StdTopleftPt;
	trpt = m_StdToprightPt;
	blpt = m_StdBottomleftPt;
	brpt = m_StdBottomrightPt;
}

//计算切片4个边界顶点在世界坐标系中的坐标
void Rotateslice::GetRealBoundCorOfSlice(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt, int flag)
{
	int lowz = 0;
	int upperz = 0;
	//topleft
	m_WorldTopleftPt.x = m_StdTopleftPt.x + m_CtUpperLeft_X;
	m_WorldTopleftPt.y = m_StdTopleftPt.y + m_CtUpperLeft_Y;
	m_WorldTopleftPt.z = m_CtUpperLeft_Z[0] + m_StdTopleftPt.z;
    
	//topright
	m_WorldToprightPt.x = m_StdToprightPt.x + m_CtUpperLeft_X;
	m_WorldToprightPt.y = m_StdToprightPt.y + m_CtUpperLeft_Y;
	m_WorldToprightPt.z = m_CtUpperLeft_Z[0] + m_StdToprightPt.z;

	//botleft
	m_WorldBottomleftPt.x = m_StdBottomleftPt.x + m_CtUpperLeft_X;
	m_WorldBottomleftPt.y = m_StdBottomleftPt.y + m_CtUpperLeft_Y;
	m_WorldBottomleftPt.z = m_CtUpperLeft_Z[0] + m_StdBottomleftPt.z;

	//botright
	m_WorldBottomrightPt.x = m_StdBottomrightPt.x + m_CtUpperLeft_X;
	m_WorldBottomrightPt.y = m_StdBottomrightPt.y + m_CtUpperLeft_Y;
	m_WorldBottomrightPt.z = m_CtUpperLeft_Z[0] + m_StdBottomrightPt.z;

	tlpt = m_WorldTopleftPt;
	trpt = m_WorldToprightPt;
	blpt = m_WorldBottomleftPt;
	brpt = m_WorldBottomrightPt;
}

//计算得到切片在空间中的4个边界上的顶点
void Rotateslice::CalcBoundcorOfSlice(float a, float b, float c, Point3f ptOnplane, int dstwidth, int dstheight, int flag)
{
	if (1 == flag)
	{
		float toplefty = 0;
		float topleftz = 0;
		m_StdTopleftPt = GetStdCorYZX(a, b, c, toplefty, topleftz, ptOnplane);
		float toprighty = dstwidth*Resizeratio[1];
		float toprightz = 0;
		m_StdToprightPt = GetStdCorYZX(a, b, c, toprighty, toprightz, ptOnplane);
		float bottomlefty = 0;
		float bottomleftz = dstheight*Resizeratio[2];
		m_StdBottomleftPt = GetStdCorYZX(a, b, c, bottomlefty, bottomleftz, ptOnplane);
		float bottomrighty = dstwidth*Resizeratio[1];
		float bottomrightz = dstheight*Resizeratio[2];
		m_StdBottomrightPt = GetStdCorYZX(a, b, c, bottomrighty, bottomrightz, ptOnplane);
	}
	else if (2 == flag)
	{
		float topleftx = 0;
		float topleftz = 0;
		m_StdTopleftPt = GetStdCorXZY(a, b, c, topleftx, topleftz, ptOnplane);
		float toprightx = dstwidth*Resizeratio[0];
		float toprightz = 0;
		m_StdToprightPt = GetStdCorXZY(a, b, c, toprightx, toprightz, ptOnplane);
		float bottomleftx = 0;
		float bottomleftz = dstheight*Resizeratio[2];
		m_StdBottomleftPt = GetStdCorXZY(a, b, c, bottomleftx, bottomleftz, ptOnplane);
		float bottomrightx = dstwidth*Resizeratio[0];
		float bottomrightz = dstheight*Resizeratio[2];
		m_StdBottomrightPt = GetStdCorXZY(a, b, c, bottomrightx, bottomrightz, ptOnplane);
	}
	else if (3 == flag)
	{
		float topleftx = 0;
		float toplefty = 0;
		m_StdTopleftPt = GetStdCorXYZ(a, b, c, topleftx, toplefty, ptOnplane);
		float toprightx = dstwidth*Resizeratio[0];
		float toprighty = 0;
		m_StdToprightPt = GetStdCorXYZ(a, b, c, toprightx, toprighty, ptOnplane);
		float bottomleftx = 0;
		float bottomlefty = dstheight*Resizeratio[1];
		m_StdBottomleftPt = GetStdCorXYZ(a, b, c, bottomleftx, bottomlefty, ptOnplane);
		float bottomrightx = dstwidth*Resizeratio[0];
		float bottomrighty = dstheight*Resizeratio[1];
		m_StdBottomrightPt = GetStdCorXYZ(a, b, c, bottomrightx, bottomrighty, ptOnplane);
	}
}

bool Rotateslice::SliceBorderConfine()
{
	bool returnflagdirection = true;
	bool returnflagLRdirection = true;
	bool returnflagout = true;
	bool returnflagin = true;
	//从下向上
	if (m_ProbeUpdirectionFlag == false)
	{
		//m_WorldSensorPt.y 
		if (m_WorldSensorPt.y < HEIGHT*m_CtPixelSpacey/2)
		{
			returnflagdirection = false;
		}
		else 
		{
			returnflagdirection = true;
		}
	}
	//从上向下
	if (m_ProbeUpdirectionFlag == true)
	{
		if (m_WorldSensorPt.y > HEIGHT*m_CtPixelSpacey / 2)
		{
			returnflagdirection = false;
		}
		else
		{
			returnflagdirection = true;
		}
	}

	//从左向右
	if (m_ProbeLRDirectionOutFlag == 0)
	{
		//m_WorldSensorPt.y 
		if (m_WorldSensorPt.x < 7.0f*WIDTH*m_CtPixelSpacex / 8)
		{
			returnflagLRdirection = false;
		}
		else
		{
			returnflagLRdirection = true;
		}
	}
	//从右向左
	else if (m_ProbeLRDirectionOutFlag == 2)
	{
		if (m_WorldSensorPt.x >= WIDTH*m_CtPixelSpacex / 8)
		{
			returnflagLRdirection = false;
		}
		else
		{
			returnflagLRdirection = true;
		}
	}
	else
	{
		returnflagLRdirection = true;
	}
	/*if ((m_WorldSensorPt.x < 0) || (m_WorldSensorPt.x > WIDTH*m_CtPixelSpacex) || (m_WorldSensorPt.y < 0) || (m_WorldSensorPt.y > HEIGHT*m_CtPixelSpacey) || (m_WorldSensorPt.z < 0) || (m_WorldSensorPt.z > m_Slicenum*m_CtPixelSpacez))
	{
		returnflagout = false;
	}*/

	if ((m_WorldSensorPt.x < m_BottomBoundRenderingX) || (m_WorldSensorPt.x > m_UpperBoundRenderingX) || (m_WorldSensorPt.y < m_BottomBoundRenderingY) || (m_WorldSensorPt.y > m_UpperBoundRenderingX) || (m_WorldSensorPt.z < m_BottomBoundRenderingZ) || (m_WorldSensorPt.z > m_UpperBoundRenderingX))
	{
		returnflagout = false;
	}
	int numslicept = m_DstWidth*m_DstHeight;

	float lastElement, lastScanElement;
#if 0
	float* h_slicezero = (float *)malloc(numslicept* sizeof(float));
	int err = clEnqueueReadBuffer(m_Queue, m_DSliceZero, CL_TRUE, 0, numslicept* sizeof(float), h_slicezero, 0, 0, 0);
	check_CL_error(err, "test copy to host failed");
	FILE *fp = NULL;
	fopen_s(&fp, "D:/Code_local/project_carbon/Configuration/slicezero.txt", "w+");
	for (int i = 0; i < numslicept / m_DstWidth; i++)
	{
		for (int j = 0; j < m_DstWidth; j++)
		{
			fprintf_s(fp, "%f ", h_slicezero[i*m_DstWidth + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	free(h_slicezero);
#endif
	ScanAPPLEProcess(m_DSliceZeroScan, m_DSliceZero, numslicept);

	clEnqueueReadBuffer(m_Queue, m_DSliceZero, CL_TRUE, (numslicept - 1) * sizeof(float), sizeof(float), &lastElement, 0, 0, 0);
	clEnqueueReadBuffer(m_Queue, m_DSliceZeroScan, CL_TRUE, (numslicept - 1) * sizeof(float), sizeof(float), &lastScanElement, 0, 0, 0);

	int activeVoxels = (uint)(lastElement + lastScanElement);
	float activeratio = (float)activeVoxels / numslicept;
	if (activeratio < 0.03)
	{
		returnflagin =  false;
	}
	else
	{
		returnflagin = true;
	}
	bool returnflag = (((returnflagdirection&&returnflagout) && returnflagLRdirection)&&returnflagin);
	return returnflag;
}

void Rotateslice::CalcInterpGridSize()
{
	float worldxsize = m_CtPixelSpacex*(GridSize[0] - 1);
	float worldysize = m_CtPixelSpacey*(GridSize[1] - 1);
	float worldzsize = m_CtPixelSpacez*(GridSize[2] - 1);
	Resizeratio[0] = m_CtPixelSpacex;
	Resizeratio[1] = m_CtPixelSpacey;
	Resizeratio[2] = m_CtPixelSpacez;
	Resizeratio[3] = 0;
}

void Rotateslice::GetSliceNormalized(float *dst)
{
	if ((m_DstWidth == 0) && (m_DstHeight == 0))
	{
		dst = NULL;
	}
	else
	{
		error = clEnqueueReadBuffer(m_Queue, m_DSliceImagefloat, CL_TRUE, 0, WIDTH*HEIGHT* sizeof(float), m_HSliceImagefloat, 0, 0, 0);
		check_CL_error(error, "sliceimage Copy from GPU failed!");
		memcpy(dst, m_HSliceImagefloat, WIDTH*HEIGHT* sizeof(float));
	}
}

void Rotateslice::GetSliceDataU8(UINT8 *dst)
{
	if ((m_DstWidth == 0) && (m_DstHeight == 0))
	{
		dst = NULL;
	}
	else
	{
		error = clEnqueueReadBuffer(m_Queue, m_DSliceImageU8, CL_TRUE, 0, m_DstWidth*m_DstHeight* sizeof(UINT8), m_HSliceImageU8, 0, 0, 0);
		check_CL_error(error, "sliceimage Copy from GPU failed!");
		memcpy(dst, m_HSliceImageU8, m_DstWidth*m_DstHeight* sizeof(UINT8));
	}
}

void Rotateslice::GetSliceDataU8GPU(cl_mem &sliceu8GPU)
{
	sliceu8GPU = m_DSliceImageU8;
}

void Rotateslice::ClearSlicePbo(cl_mem &slicepbo)
{
	int resizewidth = WIDTH;
	int resizeheight = HEIGHT;
	size_t local[2] = { BLOCKX, BLOCKY };
	int iWideDim = resizewidth / BLOCKX;
	int iHeightDim = resizeheight / BLOCKY;

	if (resizewidth % BLOCKX)
	{
		iWideDim = (iWideDim + 1) * BLOCKX;
	}
	else
	{
		iWideDim = iWideDim * BLOCKX;
	}

	if (resizeheight % BLOCKY)
	{
		iHeightDim = (iHeightDim + 1) * BLOCKY;
	}
	else
	{
		iHeightDim = iHeightDim * BLOCKY;
	}
	size_t global[2] = { iWideDim, iHeightDim };
	Launch_ClearSlicePBO(global, local, slicepbo, resizewidth, resizeheight);
}

void Rotateslice::GetSliceDataPBO(cl_mem &slicepbo, bool resizeflag)
{
	int dstwidth = GetDstWidth();
	int dstheight = GetDstHeight();
	int resizewidth = WIDTH;
	int resizeheight = HEIGHT;
	size_t local[2] = { BLOCKX, BLOCKY };
	int iWideDim = resizewidth / BLOCKX;
	int iHeightDim = resizeheight / BLOCKY;

	if (resizewidth % BLOCKX)
	{
		iWideDim = (iWideDim + 1) * BLOCKX;
	}
	else
	{
		iWideDim = iWideDim * BLOCKX;
	}

	if (resizeheight % BLOCKY)
	{
		iHeightDim = (iHeightDim + 1) * BLOCKY;
	}
	else
	{
		iHeightDim = iHeightDim * BLOCKY;
	}
	size_t global[2] = { iWideDim, iHeightDim };
	if (resizeflag == true)
	{
		Launch_GetResizeSlicePBO(global, local, m_DSliceImageU8, m_DSliceImagefloat, slicepbo, dstwidth, dstheight, resizewidth, resizeheight);
	}
	else
	{
		Launch_GetSlicePBO(global, local, m_DSliceImageU8, m_DSliceImagefloat, slicepbo, resizewidth, resizeheight);
	}
}

void Rotateslice::GetSlicePlane(vector3d normvec, Point3f ptop, int width, int height, int slicenum, cl_mem volume)
{
	m_Width = width;
	m_Height = height;
	m_Slicenum = slicenum;
	GridSize[0] = width;
	GridSize[1] = height;
	GridSize[2] = slicenum;
	GridSize[3] = 0;

	//定义最终切面的原始大小
	int dstwidth = 0;
	int dstheight = 0;
	int vheight = 0;
	int interpflag = 0;

	vector3d originz;
	vector3d originx;
	vector3d originy;
	CalcingClipPlaneCoefs(normvec.fX, normvec.fY, normvec.fZ, ptop);
	//x axis
	originx.fX = 1.0f;
	originx.fY = 0.0f;
	originx.fZ = 0.0f;
	//y axis
	originy.fX = 0.0f;
	originy.fY = 1.0f;
	originy.fZ = 0.0f;
	//z axis
	originz.fX = 0.0f;
	originz.fY = 0.0f;
	originz.fZ = 1.0f;
	float zangle = ClacAngleOfAxis(normvec, originz);
	float yangle = ClacAngleOfAxis(normvec, originy);
	float xangle = ClacAngleOfAxis(normvec, originx);

	//生成与法线垂直的切片平面
	//zangle = 0表示该切面平行于z轴
	//第一种情况，切面与z轴夹角较小，同时平行于y轴, 切面以yz平面为主，flag定义为1
	if (((zangle >= -5) && (zangle < 5)) && (yangle == 0))
	{
		dstwidth = height;
		dstheight = slicenum;
		vheight = width;
		interpflag = 1;
	}
	//第二种情况，切面与z轴夹角较小，同时平行于x轴, 切面以xz平面为主，flag定义为2
	else if (((zangle >= -5) && (zangle < 5)) && (xangle == 0))
	{
		dstwidth = width;
		dstheight = slicenum;
		vheight = height;
		interpflag = 2;
	}
	//第三种情况，切面与z轴夹角较小，x轴夹角绝对值在45度以内，取xz平面, 切面以xz平面为主，flag定义为2
	else if (((zangle >= -5) && (zangle < 5)) && ((xangle >= -45) && (xangle <= 45)))
	{
		dstwidth = width;
		dstheight = slicenum;
		vheight = height;
		interpflag = 2;
	}
	//第四种情况，切面与z轴夹角较小，x轴夹角绝对值大于45度，取yz平面， 切面以yz平面为主，flag定义为1
	else if (((zangle >= -5) && (zangle < 5) && (fabs(xangle) > 45)))
	{
		dstwidth = height;
		dstheight = slicenum;
		vheight = width;
		interpflag = 1;
	}
	//第五种情况，切面与z轴夹角绝对值大于10度，x轴夹角绝对值大于45度，取xy平面，flag定义为3， 后续还要考虑下x, y正负数的情况
	else
	{
		dstwidth = width;
		dstheight = height;
		vheight = slicenum;
		interpflag = 3;
	}
	m_DstWidth = dstwidth;
	m_DstHeight = dstheight;
	//计算切面数据
	GenerateSlicePts(normvec.fX, normvec.fY, normvec.fZ, width, height, slicenum, dstwidth, dstheight, vheight, interpflag, volume);
	//计算边界点信息(标准坐标系中)
	CalcBoundcorOfSlice(normvec.fX, normvec.fY, normvec.fZ, ptop, dstwidth, dstheight, interpflag);
}

void Rotateslice::GetSlicePlane(vector3d axisvec, float *RMat, Point3f ptop, int width, int height, int slicenum, cl_mem volume)
{
	vector3d normvec = matrix_mult(RMat, axisvec);
	m_Width = width;
	m_Height = height;
	m_Slicenum = slicenum;
	GridSize[0] = width;
	GridSize[1] = height;
	GridSize[2] = slicenum;
	GridSize[3] = 0;

	//定义最终切面的原始大小
	int dstwidth = 0;
	int dstheight = 0;
	int vheight = 0;
	int interpflag = 0;

	vector3d originz;
	vector3d originx;
	vector3d originy;
	CalcingClipPlaneCoefs(normvec.fX, normvec.fY, normvec.fZ, ptop);
	//x axis
	originx.fX = 1.0f;
	originx.fY = 0.0f;
	originx.fZ = 0.0f;
	//y axis
	originy.fX = 0.0f;
	originy.fY = 1.0f;
	originy.fZ = 0.0f;
	//z axis
	originz.fX = 0.0f;
	originz.fY = 0.0f;
	originz.fZ = 1.0f;
	float zangle = ClacAngleOfAxis(normvec, originz);
	float yangle = ClacAngleOfAxis(normvec, originy);
	float xangle = ClacAngleOfAxis(normvec, originx);

	//生成与法线垂直的切片平面
	//zangle = 0表示该切面平行于z轴
	//第一种情况，切面与z轴夹角较小，同时平行于y轴, 切面以yz平面为主，flag定义为1
	if (((zangle >= -5) && (zangle < 5)) && (yangle == 0))
	{
		dstwidth = height;
		dstheight = slicenum;
		vheight = width;
		interpflag = 1;
	}
	//第二种情况，切面与z轴夹角较小，同时平行于x轴, 切面以xz平面为主，flag定义为2
	else if (((zangle >= -5) && (zangle < 5)) && (xangle == 0))
	{
		dstwidth = width;
		dstheight = slicenum;
		vheight = height;
		interpflag = 2;
	}
	//第三种情况，切面与z轴夹角较小，x轴夹角绝对值在45度以内，取xz平面, 切面以xz平面为主，flag定义为2
	else if (((zangle >= -5) && (zangle < 5)) && ((xangle >= -45) && (xangle <= 45)))
	{
		dstwidth = width;
		dstheight = slicenum;
		vheight = height;
		interpflag = 2;
	}
	//第四种情况，切面与z轴夹角较小，x轴夹角绝对值大于45度，取yz平面， 切面以yz平面为主，flag定义为1
	else if (((zangle >= -5) && (zangle < 5) && (fabs(xangle) > 45)))
	{
		dstwidth = height;
		dstheight = slicenum;
		vheight = width;
		interpflag = 1;
	}
	//第五种情况，切面与z轴夹角绝对值大于10度，x轴夹角绝对值大于45度，取xy平面，flag定义为3， 后续还要考虑下x, y正负数的情况
	else
	{
		dstwidth = width;
		dstheight = height;
		vheight = slicenum;
		interpflag = 3;
	}
	m_DstWidth = dstwidth;
	m_DstHeight = dstheight;
	//计算切面数据
	GenerateSlicePts(normvec.fX, normvec.fY, normvec.fZ, width, height, slicenum, dstwidth, dstheight, vheight, interpflag, volume);
	//计算边界点信息(标准坐标系中)
	CalcBoundcorOfSlice(normvec.fX, normvec.fY, normvec.fZ, ptop, dstwidth, dstheight, interpflag);
}

void Rotateslice::GetSlicePlane(float startx, float starty, float startz, float endx, float endy, float endz, int width, int height, int slicenum, cl_mem volume)
{
	m_Width = width;
	m_Height = height;
	m_Slicenum = slicenum;
	GridSize[0] = width;
	GridSize[1] = height;
	GridSize[2] = slicenum;
	GridSize[3] = 0;
	m_Xmid = (startx + endx) / 2.0f;
	m_Ymid = (starty + endy) / 2.0f;
	m_Zmid = (startz + endz) / 2.0f;
	//定义最终切面的原始大小
	int dstwidth = 0;
	int dstheight = 0;
	int vheight = 0;
	int interpflag = 0;

	//定义平面法向量的三个分量
	float a = 0.0f;
	float b = 0.0f;
	float c = 0.0f;
	vector3d normvec;
	vector3d originz;
	vector3d originx;
	vector3d originy;

	ConstructSlicePlaneNormVec(startx, starty, startz, endx, endy, endz, a, b, c);
	//计算出平面上的一个点的坐标
	Point3f ptop;
	float xop = (startx + endx) / 2.0f;
	float yop = (starty + endy) / 2.0f;
	float zop = (startz + endz) / 2.0f;
	ptop.x = xop;
	ptop.y = yop;
	ptop.z = zop;
	CalcingClipPlaneCoefs(a, b, c, ptop);
	normvec.fX = a;
	normvec.fY = b;
	normvec.fZ = c;
	//x axis
	originx.fX = 1.0f;
	originx.fY = 0.0f;
	originx.fZ = 0.0f;
	//y axis
	originy.fX = 0.0f;
	originy.fY = 1.0f;
	originy.fZ = 0.0f;
	//z axis
	originz.fX = 0.0f;
	originz.fY = 0.0f;
	originz.fZ = 1.0f;
	float zangle = ClacAngleOfAxis(normvec, originz);
	float yangle = ClacAngleOfAxis(normvec, originy);
	float xangle = ClacAngleOfAxis(normvec, originx);

	//生成与法线垂直的切片平面
	//zangle = 0表示该切面平行于z轴
	//第一种情况，切面与z轴夹角较小，同时平行于y轴, 切面以yz平面为主，flag定义为1
	if (((zangle >= -5) && (zangle < 5)) && (yangle == 0))
	{
		dstwidth = height;
		dstheight = slicenum;
		vheight = width;
		interpflag = 1;
	}
	//第二种情况，切面与z轴夹角较小，同时平行于x轴, 切面以xz平面为主，flag定义为2
	else if (((zangle >= -5) && (zangle < 5)) && (xangle == 0))
	{
		dstwidth = width;
		dstheight = slicenum;
		vheight = height;
		interpflag = 2;
	}
	//第三种情况，切面与z轴夹角较小，x轴夹角绝对值在45度以内，取xz平面, 切面以xz平面为主，flag定义为2
	else if(((zangle >= -5) && (zangle < 5)) && ((xangle >= -45) && (xangle <= 45)))
	{
		dstwidth = width;
		dstheight = slicenum;
		vheight = height;
		interpflag = 2;
	}
	//第四种情况，切面与z轴夹角较小，x轴夹角绝对值大于45度，取yz平面， 切面以yz平面为主，flag定义为1
	else if (((zangle >= -5) && (zangle < 5) && (fabs(xangle) > 45)))
	{ 
		dstwidth = height;
		dstheight = slicenum;
		vheight = width;
		interpflag = 1;
	}
	//第五种情况，切面与z轴夹角绝对值大于10度，x轴夹角绝对值大于45度，取xy平面，flag定义为3， 后续还要考虑下x, y正负数的情况
	else
	{
		dstwidth = width;
		dstheight = height;
		vheight = slicenum;
		interpflag = 3;
	}
	m_DstWidth = dstwidth;
	m_DstHeight = dstheight;
	//计算切面数据
	GenerateSlicePts(normvec.fX, normvec.fY, normvec.fZ, width, height, slicenum, dstwidth, dstheight, vheight, interpflag, volume);
	//计算边界点信息(标准坐标系中)
	CalcBoundcorOfSlice(normvec.fX, normvec.fY, normvec.fZ, ptop, dstwidth, dstheight, interpflag);
}

//接口函数4,传入初始向量，三个坐标值，以及三个坐标轴的夹角，构建法向量
//azimuth为绕Z轴旋转的夹角
//elevation为绕x轴旋转的夹角
//roll为绕y轴旋转的夹角
//axisvec初始向量为Z轴向量（0, 0, 1）
void Rotateslice::GetSlicePlane(cl_mem volume)
{
	Point3f ptop(m_Xmid, m_Ymid, m_Zmid, 0);
	if (m_Directcopyflag == 1)
	{
		CopySliceDirectly(m_Zind, m_Width, m_Height, m_Slicenum, m_DstWidth, m_DstHeight, m_VHeight, volume);
	}
	else
	{
		//计算切面数据
		GenerateSlicePts(m_Pa, m_Pb, m_Pc, m_Width, m_Height, m_Slicenum, m_DstWidth, m_DstHeight, m_VHeight, m_Interpflag, volume);
	}
	//计算边界点信息(标准坐标系中)
	CalcBoundcorOfSlice(m_Pa, m_Pb, m_Pc, ptop, m_DstWidth, m_DstHeight, m_Interpflag);
}

void Rotateslice::InitSlicePlaneInfo(vector3d axisvec, float x, float y, float z, float azimuth, float roll, float elevation, float *basicRmat)
{
	if (fabs(roll) > PI / 2)
	{
		m_ProbeUpdirectionFlag = true;
	}
	else
	{
		m_ProbeUpdirectionFlag = false;
	}
	if (azimuth <= -5*PI/12)
	{
		m_ProbeLRDirectionOutFlag = 0;//向右
	}
	else if (azimuth >= 5*PI / 12)
	{
		m_ProbeLRDirectionOutFlag = 2;//向左
	}
	else
	{
		m_ProbeLRDirectionOutFlag = 1;//正常
	}
	m_Xmid = x;
	m_Ymid = y;
	m_Zmid = z;
	Point3f ptop(m_Xmid, m_Ymid, m_Zmid, 0);
	//GetRotationMat(azimuth, roll, elevation, m_TempRMat);
	//matrix_matrix_mult(basicRmat, m_TempRMat, m_RMat);
	/*printf("TempRMat\n");
	for (int i = 0; i < 9; i++)
	{
		printf("%f ", m_TempRMat[i]);
		if (i % 3 == 0)
		{
			printf("\n");
		}
	}
	float RDirectionMat[9] = { 0.0f };
	GetRotationMatDirectly(azimuth, roll, elevation, RDirectionMat);
	printf("\n");
	printf("\n");
	printf("RDirectionMat\n");
	for (int i = 0; i < 9; i++)
	{
		printf("%f ", RDirectionMat[i]);
		if (i % 3 == 0)
		{
			printf("\n");
		}
	}*/

	GetRotationMat(azimuth, roll, elevation, m_RMat);
	/*printf("TempRMat\n"););
	printf("basicRmat\n");
	for (int i = 0; i < 9; i++)
	{
		printf("%f ", basicRmat[i]);
		if (((i + 1) % 3 == 0) && (i>0))
		{
			printf("\n");
		}
	}
	printf("\n");
	printf("TempRMat\n");
	for (int i = 0; i < 9; i++)
	{
		printf("%f ", m_TempRMat[i]);
		if (((i + 1) % 3 == 0) && (i>0))
		{
			printf("\n");
		}
	}
	printf("\n");
	printf("m_RMat\n");
	for (int i = 0; i < 9; i++)
	{
		printf("%f ", m_RMat[i]);
		if (((i + 1) % 3 == 0) && (i>0))
		{
			printf("\n");
		}
	}
	printf("\n");*/

	//GetRotationMat(azimuth, elevation, roll, m_RMat);
	vector3d normvec = matrix_mult(m_RMat, axisvec);

	m_Zind = (int)std::round(z);

	vector3d originz;
	vector3d originx;
	vector3d originy;
	CalcingClipPlaneCoefs(normvec.fX, normvec.fY, normvec.fZ, ptop);
	//x axis
	originx.fX = 1.0f;
	originx.fY = 0.0f;
	originx.fZ = 0.0f;
	//y axis
	originy.fX = 0.0f;
	originy.fY = 1.0f;
	originy.fZ = 0.0f;
	//z axis
	originz.fX = 0.0f;
	originz.fY = 0.0f;
	originz.fZ = 1.0f;
	float zangle = ClacAngleOfAxis(normvec, originz);
	float yangle = ClacAngleOfAxis(normvec, originy);
	float xangle = ClacAngleOfAxis(normvec, originx);
	float xanglethr = 45.0f;
	float zanglethr = 30.0f;
	float yangletiny = 0.01f;
	float xangletiny = 0.01f;

	//生成与法线垂直的切片平面
	//zangle = 0表示该切面平行于z轴
	//第一种情况，切面与z轴夹角较小，同时平行于y轴, 切面以yz平面为主，flag定义为1
	if ((fabs(zangle) < zanglethr) && (fabs(yangle) < yangletiny))
	{
		m_DstWidth = GridSize[1];
		m_DstHeight = GridSize[2];
		m_VHeight = GridSize[0];

		if (xangle == 0.0f)
		{
			m_Interpflag = 0;
			m_Directcopyflag = 1;
		}
		else
		{
			m_Interpflag = 1;
			m_Directcopyflag = 0;
		}
	}
	//第二种情况，切面与z轴夹角较小，同时平行于x轴, 切面以xz平面为主，flag定义为2
	else if ((fabs(zangle) < zanglethr) && (fabs(xangle) < xangletiny))
	{
		m_DstWidth = GridSize[0];
		m_DstHeight = GridSize[2];
		m_VHeight = GridSize[1];

		m_Interpflag = 2;

		if (yangle == 0.0f)
		{
			m_Interpflag = 0;
			m_Directcopyflag = 1;
		}
		else
		{
			m_Interpflag = 2;
			m_Directcopyflag = 0;
		}
	}
	//第三种情况，切面与z轴夹角较小，x轴夹角绝对值在45度以内，取xz平面, 切面以xz平面为主，flag定义为2
	else if ((fabs(zangle) < zanglethr) && (fabs(xangle) <= xanglethr))
	{
		m_DstWidth = GridSize[0];
		m_DstHeight = GridSize[2];
		m_VHeight = GridSize[1];

		m_Interpflag = 2;
		m_Directcopyflag = 0;
	}
	//第四种情况，切面与z轴夹角较小，x轴夹角绝对值大于45度，取yz平面， 切面以yz平面为主，flag定义为1
	else if ((fabs(zangle) < zanglethr) && (fabs(xangle) > xanglethr))
	{
		m_DstWidth = GridSize[1];
		m_DstHeight = GridSize[2];
		m_VHeight = GridSize[0];

		m_Interpflag = 1;
		m_Directcopyflag = 0;
	}
	//第五种情况，切面与z轴夹角绝对值大于10度，x轴夹角绝对值大于45度，取xy平面，flag定义为3， 后续还要考虑下x, y正负数的情况
	else
	{
		m_DstWidth = GridSize[0];
		m_DstHeight = GridSize[1];
		m_VHeight = GridSize[2];

		m_Interpflag = 3;
		m_Directcopyflag = 0;
	}
}

void Rotateslice::ConstructSlicePlaneNormVec(float startx, float starty, float startz, float endx, float endy, float endz, float &a, float &b, float &c)
{
	a = (float)(endx - startx);
	b = (float)(endy - starty);
	c = (float)(endz - startz);
}

void Rotateslice::CalcingClipPlaneCoefs(float norma, float normb, float normc, Point3f ptOnplane)
{
	float tempa = norma;
	float tempb = normb;
	float tempc = normc;
	float tempd = -1.0f*(tempa*ptOnplane.x + tempb*ptOnplane.y + tempc*ptOnplane.z);
	m_Pa = tempa;
	m_Pb = tempb;
	m_Pc = tempc;
	m_Pd = tempd;
}

void Rotateslice::ClearPboBuffer()
{
	int resizewidth = WIDTH;
	int resizeheight = HEIGHT;
	size_t local[2] = { BLOCKX, BLOCKY };
	int iWideDim = resizewidth / BLOCKX;
	int iHeightDim = resizeheight / BLOCKY;

	if (resizewidth % BLOCKX)
	{
		iWideDim = (iWideDim + 1) * BLOCKX;
	}
	else
	{
		iWideDim = iWideDim * BLOCKX;
	}

	if (resizeheight % BLOCKY)
	{
		iHeightDim = (iHeightDim + 1) * BLOCKY;
	}
	else
	{
		iHeightDim = iHeightDim * BLOCKY;
	}
	size_t global[2] = { iWideDim, iHeightDim };
	Launch_ClearPBOBuffer(global, local, m_DSliceImageU8, m_DSliceImagefloat, resizewidth, resizeheight);
}

void Rotateslice::CopySliceDirectly(int sliceind, int width, int height, int slicenum, int dstwidth, int dstheight, int vheight, cl_mem volume)
{
	size_t local[2] = { BLOCKX, BLOCKY };
	int iWideDim = dstwidth / BLOCKX;
	int iHeightDim = dstheight / BLOCKY;
	if (dstwidth%BLOCKX)
	{
		iWideDim = (iWideDim + 1)*BLOCKX;
	}
	else
	{
		iWideDim = iWideDim*BLOCKX;
	}

	if (dstheight%BLOCKY)
	{
		iHeightDim = (iHeightDim + 1)*BLOCKX;
	}
	else
	{
		iHeightDim = (iHeightDim + 1)*BLOCKY;
	}
	size_t global[2] = { iWideDim, iHeightDim };

	unsigned int padding = BLOCKX*BLOCKY / NUM_BANKS;
	size_t shared = sizeof(float) * (BLOCKX*BLOCKY + padding);
	vmin = (short)((float)m_CtWinCenter - 0.5f*m_CtWinWidth);
	vmax = (float)((float)m_CtWinCenter + 0.5f*m_CtWinWidth);
	GetSliceDirectly(global, local, shared, volume, m_DSliceImageU8, width, height, slicenum, dstwidth, dstheight, vheight, sliceind, m_MaxValue, m_MinValue);
}

//根据法向量得到待求平面
void Rotateslice::GenerateSlicePts(float a, float b, float c, int width, int height, int slicenum, int dstwidth, int dstheight, int vheight, int flag, cl_mem volume)
{
	size_t local[2] = { BLOCKX, BLOCKY };
	int iWideDim = dstwidth / BLOCKX;
	int iHeightDim = dstheight / BLOCKY;

	if (dstwidth % BLOCKX)
	{
		iWideDim = (iWideDim + 1) * BLOCKX;
	}
	else
	{
		iWideDim = iWideDim * BLOCKX;
	}

	if (dstheight % BLOCKY)
	{
		iHeightDim = (iHeightDim + 1) * BLOCKY;
	}
	else
	{
		iHeightDim = iHeightDim * BLOCKY;
	}
	size_t global[2] = { iWideDim, iHeightDim };

	unsigned int padding = BLOCKX*BLOCKY / NUM_BANKS;
	size_t shared = sizeof(float) * (BLOCKX*BLOCKY + padding);
	vmin = (short)((float)m_CtWinCenter - 0.5f*m_CtWinWidth);
	vmax = (float)((float)m_CtWinCenter + 0.5f*m_CtWinWidth);
	if (1 == flag)
	{
		GetSliceYZX(global, local, shared, volume, m_DSliceImageU8, width, height, slicenum, dstwidth, dstheight, vheight, a, b, c, m_Xmid, m_Ymid, m_Zmid, m_MaxValue, m_MinValue);
	}
	else if (2 == flag)
	{
		GetSliceXZY(global, local, shared, volume, m_DSliceImageU8, width, height, slicenum, dstwidth, dstheight, vheight, a, b, c, m_Xmid, m_Ymid, m_Zmid, m_MaxValue, m_MinValue);
	}
	else if (3 == flag)
	{
		GetSliceXYZ(global, local, shared, volume, m_DSliceImageU8, width, height, slicenum, dstwidth, dstheight, vheight, a, b, c, m_Xmid, m_Ymid, m_Zmid, m_MaxValue, m_MinValue);
	}
}

float Rotateslice::ClacAngleOfAxis(vector3d normvec, vector3d axisvec)
{
	float dvalue = Dot3D(normvec, axisvec) / (CalcNValueofVec(normvec)*CalcNValueofVec(axisvec));
	float angle = acos(dvalue);
	if ((dvalue >= 0.0f) && (dvalue < 1.0f))
	{
		angle = (PI / 2 - angle)*180.0f/PI;
	}
	else
	{
		angle = -1.0f*(angle-PI/2) * 180.0f / PI;
	}
	return angle;
}

void Rotateslice::GetSliceYZX(size_t *global, size_t *local, size_t shared, cl_mem volume, cl_mem sliceu8, cl_uint width, cl_uint height, cl_uint slicenum,
	cl_uint dstwidth, cl_uint dstheight, cl_uint vheight, cl_float na, cl_float nb, cl_float nc, cl_float x_mid, cl_float y_mid, cl_float z_mid, cl_float maxval, cl_float minval)
{
	unsigned int k = SLICEYZX;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &volume);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &sliceu8);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &m_DSliceZero);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, shared, 0);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), GridSize);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_float), Resizeratio);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &na);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &nb);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &nc);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &x_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &y_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &z_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstwidth);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstheight);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &vheight);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_int), &minval);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_int), &maxval);

	check_CL_error(err, "Setting GetSliceYZX failed!");

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, global, local, 0, NULL, NULL);
	check_CL_error(err, "GetSliceYZX failed!");
}

void Rotateslice::GetSliceXZY(size_t *global, size_t *local, size_t shared, cl_mem volume, cl_mem sliceu8, cl_uint width, cl_uint height, cl_uint slicenum,
	cl_uint dstwidth, cl_uint dstheight, cl_uint vheight, cl_float na, cl_float nb, cl_float nc, cl_float x_mid, cl_float y_mid, cl_float z_mid, cl_float maxval, cl_float minval)
{
	unsigned int k = SLICEXZY;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &volume);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &sliceu8);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &m_DSliceZero);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, shared, 0);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), GridSize);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_float), Resizeratio);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &na);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &nb);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &nc);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &x_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &y_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &z_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstwidth);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstheight);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &vheight);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &minval);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &maxval);

	check_CL_error(err, "Setting GetSliceXZY failed!");

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, global, local, 0, NULL, NULL);
	check_CL_error(err, "GetSliceXZY failed!");
}

void Rotateslice::GetSliceXYZ(size_t *global, size_t *local, size_t shared, cl_mem volume, cl_mem sliceu8, cl_uint width, cl_uint height, cl_uint slicenum,
	cl_uint dstwidth, cl_uint dstheight, cl_uint vheight, cl_float na, cl_float nb, cl_float nc, cl_float x_mid, cl_float y_mid, cl_float z_mid, cl_float maxval, cl_float minval)
{
	unsigned int k = SLICEXYZ;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &volume);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &sliceu8);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &m_DSliceZero);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, shared, 0);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), GridSize);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_float), Resizeratio);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &na);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &nb);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &nc);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &x_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &y_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &z_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstwidth);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstheight);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &vheight);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &minval);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &maxval);

	check_CL_error(err, "Setting GetSliceXYZ failed!");

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, global, local, 0, NULL, NULL);
	check_CL_error(err, "GetSliceXYZ failed!");
}

void Rotateslice::GetSliceCorXYZ(size_t *global, size_t *local, size_t shared, cl_mem worldslicecor, cl_uint dstwidth, cl_uint dstheight, cl_uint dstvheight,
	cl_float na, cl_float nb, cl_float nc, cl_float x_mid, cl_float y_mid, cl_float z_mid)
{
	unsigned int k = GETWORLDSLICECORXYZ;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &worldslicecor);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, shared, 0);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), GridSize);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_float), Resizeratio);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &na);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &nb);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &nc);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &x_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &y_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &z_mid);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstwidth);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstheight);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstvheight);

	check_CL_error(err, "Setting GetSliceCorXYZ failed!");

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, global, local, 0, NULL, NULL);
	check_CL_error(err, "GetSliceCorXYZ failed!");
}

void Rotateslice::Launch_GetSlicePBO(size_t *global, size_t *local, cl_mem slicegpu, cl_mem slicenorm, cl_mem slicepbo, cl_uint width, cl_uint height)
{
	unsigned int k = GETSLICEPBO;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &slicegpu);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &slicenorm);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &slicepbo);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &width);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &height);

	check_CL_error(err, "Setting Launch_GetSlicePBO failed!");

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, global, local, 0, NULL, NULL);
	check_CL_error(err, "Launch_GetSlicePBO failed!");
#if 0
	unsigned int *sliceResult = (unsigned int *)malloc(width*height*sizeof(unsigned int));
	error = clEnqueueReadBuffer(m_SliceQueue, slicepbo, CL_TRUE, 0, width*height*sizeof(unsigned int), sliceResult, 0, 0, NULL);
	FILE *fp = NULL;
	fopen_s(&fp, "D:/GetBindata/GetBindata/slicepboReadBack.txt", "w+");
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			fprintf_s(fp, "%d ", sliceResult[i*width + j]);
		}
		fprintf_s(fp, "\n");
	}
	fclose(fp);
	free(sliceResult);
#endif
}

void Rotateslice::Launch_GetResizeSlicePBO(size_t *global, size_t *local, cl_mem slicegpu, cl_mem slicenorm, cl_mem slicepbo, cl_uint width, cl_uint height, cl_uint resizewidth, cl_uint resizeheight)
{
	unsigned int k = GETRESIZESLICEPBO;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &slicegpu);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &slicenorm);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &slicepbo);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &width);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &height);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &resizewidth);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &resizeheight);

	check_CL_error(err, "Setting Launch_GetResizeSlicePBO failed");

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, global, local, 0, NULL, NULL);
	check_CL_error(err, "Launch_GetResizeSlicePBO failed!");
}

void Rotateslice::Launch_ClearPBOBuffer(size_t *global, size_t *local, cl_mem slicegpu_u8, cl_mem slicenorm, cl_uint resizewidth, cl_uint resizeheight)
{
	unsigned int k = CLEARPBOBUF;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &slicegpu_u8);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &slicenorm);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &resizewidth);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &resizeheight);

	check_CL_error(err, "Setting Launch_ClearPBOBuffer failed!");

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, global, local, 0, NULL, NULL);
	check_CL_error(err, "Launch_ClearPBOBuffer failed!");
}

void Rotateslice::Launch_ClearSlicePBO(size_t *global, size_t *local, cl_mem slicepbo, cl_uint resizewidth, cl_uint resizeheight)
{
	unsigned int k = CLEARSLICEPBO;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &slicepbo);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &resizewidth);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &resizeheight);

	check_CL_error(err, "Setting Launch_ClearSlicePBO failed!");

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, global, local, 0, NULL, NULL);
	check_CL_error(err, "Launch_ClearSlicePBO failed!");
}

void Rotateslice::GetSliceDirectly(size_t *global, size_t *local, size_t shared, cl_mem volume, cl_mem sliceu8, cl_uint width, cl_uint height, cl_uint slicenum,
	cl_uint dstwidth, cl_uint dstheight, cl_uint vheight, cl_uint z_ind, cl_float maxval, cl_float minval)
{
	unsigned int k = CopySlice;
	unsigned int a = 0;

	int err = CL_SUCCESS;
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &volume);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_mem), &sliceu8);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, shared, 0);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, 4 * sizeof(cl_uint), GridSize);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstwidth);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &dstheight);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_uint), &vheight);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &minval);
	err |= clSetKernelArg(m_ComputeKernels[k], a++, sizeof(cl_float), &maxval);

	check_CL_error(err, "Setting GetSliceDirectly failed!");

	err = CL_SUCCESS;
	err |= clEnqueueNDRangeKernel(m_Queue, m_ComputeKernels[k], 2, NULL, global, local, 0, NULL, NULL);
	check_CL_error(err, "GetSliceDirectly failed!");
}