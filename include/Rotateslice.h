// Rotateslice:  
// Get the slice image of the volumedata from any angle  
//    
//  Created by Chriswoo on 2017-08-02. 
//  Contact: wsscx01@aliyun.com
// 
#ifndef _ROTATESLICE_H_
#define _ROTATESLICE_H_

#include "DataTypes.h"
#include "OpenCLBase.h"
#include <stdlib.h>
#include <sys/types.h> 
#include "QuaternionProcess.h"

#define GROUP_SIZE 256
#define MAXSLICENUM 512
using namespace QuaternionProcess;

typedef struct PositionAngleUnit
{
	double	x;
	double	y;
	double	z;
	double	a;
	double	e;
	double	r;
	double	time;
	USHORT	quality;
} PositionAngleUnit;

const char SliceKernelFile[256] = "D:/CarbonMed/Config/Local/CL/RotateSlice.cl";
class Rotateslice :public OpenCLBase
{
public:
	Rotateslice();
	~Rotateslice();
    
	//virtual func
	void KernelInit();
	void RotateSliceOpenCLRelease();
	//外部接口函数,获取切片数据，完成从GPU到CPU的拷贝
	void GetSliceDataU8(UINT8 *dst);
	void GetSliceNormalized(float *dst);

	//获取显存中的切片数据
	void GetSliceDataU8GPU(cl_mem &sliceu8GPU);
	void GetSliceDataPBO(cl_mem &slicepbo, bool resizeflag);
	void ClearSlicePbo(cl_mem &slicepbo);

	//接口函数1, 直接传入法向量
	void GetSlicePlane(vector3d normvec, Point3f ptop, int width, int height, int slicenum, cl_mem volume);
	//接口函数2，传入一个向量与一个旋转矩阵，构建法向量
	void GetSlicePlane(vector3d axisvec, float *RMat, Point3f ptop, int width, int height, int slicenum, cl_mem volume);
	//接口函数3,构建法向量
	void GetSlicePlane(float startx, float starty, float startz, float endx, float endy, float endz, int width, int height, int slicenum, cl_mem volume);
	//接口函数4,传入初始向量，三个坐标值，以及三个坐标轴的夹角，构建法向量
	//azimuth为绕Z轴旋转的夹角
	//elevation为绕x轴旋转的夹角
	//roll为绕y轴旋转的夹角
	//axisvec初始向量为Z轴向量（0, 0, 1）
	void GetSlicePlane(cl_mem volume);
    //初始化切片计算信息
	void InitSlicePlaneInfo(vector3d axisvec, float x, float y, float z, float azimuth, float elevation, float roll, float *basicRmat);
	//根据已有信息构建切割平面,得到平面的法向量,同时确定最后的切面的长和宽
	void ConstructSlicePlaneNormVec(float startx, float starty, float startz, float endx, float endy, float endz, float &a, float &b, float &c);

	//如果直接拷贝CT序列中的切片
	void CopySliceDirectly(int sliceind, int width, int height, int slicenum, int dstwidth, int dstheight, int vheight, cl_mem volume);
	//根据法向量得到待求平面
	void GenerateSlicePts(float a, float b, float c, int width, int height, int slicenum, int dstwidth, int dstheight, int vheight, int flag, cl_mem volume);
	//清理PBO内存空间
	void ClearPboBuffer();

	//根据旋转角度获取旋转矩阵
	void GetRotationMat(float azimuth, float elevation, float roll, float *RMat);

	//根据四元数获取相应的旋转结果
	void GetQuaternionRotation(float azimuth, float roll, float elevation, int radian_flag, vector3d &inputvec, vector3d &resultvec);

	//根据旋转角度和公式直接获取旋转矩阵
	void GetRotationMatDirectly(float azimuth, float elevation, float roll, float *RMat);

	//向量叉乘
	vector3d cross(vector3d a, vector3d b);

	//计算顶点法向量
	vector3d calcNormal(vector3d &v0, vector3d &v1, vector3d &v2);

	//计算向量的模
	float CalcNValueofVec(vector3d v);

	//向量点乘
	float Dot3D(vector3d a, vector3d b);

	//矩阵矩阵乘法
	void matrix_matrix_mult(float *m1, float *m2, float *result);

	//矩阵向量乘法
	vector3d matrix_mult(float *mat, vector3d a);

	//摄像机变换矩阵向量乘法
	vector3d Camtransmatrix_mult(float *mat, vector3d a);

	//矩阵矩阵乘法
	void matrix_matrix_mult(double m1[3][3], double m2[3][3], double result[3][3]);

	//计算切面与坐标轴之间的夹角
	float ClacAngleOfAxis(vector3d normvec, vector3d axisvec);

	//获取体数据中的最大和最小值
	void CalcMaxMinValueOfVolume(float *volume);

	//计算得到切片在空间中的4个边界上的顶点(标准)
	void CalcBoundcorOfSlice(float a, float b, float c, Point3f ptOnplane, int dstwidth, int dstheight, int flag);

	//获取切片4个边界上的顶点在CT数据中的真实坐标(外部调用接口)
	void GetRealBoundCorOfSlice(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt, int flag);

	//获取切片4个边界上的顶点在CT数据中的标准坐标(外部调用接口)
	void GetStdBoundCorOfSlice(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt, int flag);

	bool SliceBorderConfine();

	//用真实的物理尺寸初始化InterpGridSize，这样才能真实的计算偏转角度
	void CalcInterpGridSize();

	//opencl相关的初始化
	void SliceOpenCLini(cl_context cxGPUContext, cl_command_queue cqCommandQueue, cl_device_id device);

	//初始化切片数据
	void InitSlice(int width, int height, int slicenum);

	//设置CT空间信息, 同时计算Z方向上的密度
	void SetCTCorInfo(float pixelx, float pixely, float upperlx, float upperly, float *upperlz, float wincenter, float winwidth, int scnum)
	{
		m_CtPixelSpacex = pixelx;
		m_CtPixelSpacey = pixely;
		m_CtUpperLeft_X = upperlx;
		m_CtUpperLeft_Y = upperly;

		float tempsum = 0.0f;
		for (int i = 0; i < scnum; i++)
		{
			m_CtUpperLeft_Z[i] = upperlz[i];
		}
		m_CtPixelSpacez = (m_CtUpperLeft_Z[scnum - 1] - m_CtUpperLeft_Z[0]) / (scnum-1);
		m_CtWinCenter = wincenter;
		m_CtWinWidth = winwidth;
	}

	//set the bounds of the rendering
	void SetSliceBounds(float upperx, float uppery, float upperz, float botx, float boty, float botz)
	{
		m_UpperBoundRenderingX = upperx;
		m_UpperBoundRenderingY = uppery;
		m_UpperBoundRenderingZ = upperz;
		m_BottomBoundRenderingX = botx;
		m_BottomBoundRenderingY = boty;
		m_BottomBoundRenderingZ = botz;
	}

	//设置维度相关信息函数
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

	void GetCtPixelSpacez(float &spacez)
	{
		spacez = m_CtPixelSpacez;
	}

	void GetWorldCorOfSliceBound(Point3f &wtopleft, Point3f &wtopright, Point3f &wbottomleft, Point3f &wbottomright)
	{
		wtopleft = m_WorldTopleftPt;
		wtopright = m_WorldToprightPt;
		wbottomleft = m_WorldBottomleftPt;
		wbottomright = m_WorldBottomrightPt;
	}

	void GetStdCorOfSliceBound(Point3f &stdtopleft, Point3f &stdtopright, Point3f &stdbottomleft, Point3f &stdbottomright)
	{
		stdtopleft = m_StdTopleftPt;
		stdtopright = m_StdToprightPt;
		stdbottomleft = m_StdBottomleftPt;
		stdbottomright = m_StdBottomrightPt;
	}

	int GetInterpFlag()
	{
		return m_Interpflag;
	}

	int GetDstWidth()
	{
		return m_DstWidth;
	}

	int GetDstHeight()
	{
		return m_DstHeight;
	}

	void SetVolumeMaxValue(float val)
	{
		m_MaxValue = val;
	}

	void SetVolumeMinValue(float val)
	{
		m_MinValue = val;
	}

	float GetVolumeMaxValue()
	{
		return m_MaxValue;
	}

	float GetVolumeMinValue()
	{
		return m_MinValue;
	}

	//定义获取切面系数的相关函数
	void GetPlaneCoef(float &pa, float &pb, float &pc, float &pd)
	{
		pa = m_Pa;
		pb = m_Pb;
		pc = m_Pc;
		pd = m_Pd;
	}

	//获取旋转矩阵
	void GetRotationMat(float *rmat);

	//获取旋转四元数
	void GetRotationQuaternion(Quaternion &Qrotation);
	//设置旋转四元数
	void SetRotationQuaternion(Quaternion &rq);

	//定义一个函数计算平面系数
	void CalcingClipPlaneCoefs(float norma, float normb, float normc, Point3f ptOnplane);

	//定义在不同情况下获取边界顶点坐标的函数
	Point3f GetStdCorYZX(float a, float b, float c, float y, float z, Point3f ptOnplane);
	Point3f GetStdCorXZY(float a, float b, float c, float x, float z, Point3f ptOnplane);
	Point3f GetStdCorXYZ(float a, float b, float c, float x, float y, Point3f ptOnplane);

	//获取绘制探头传感器的位置点
	void GetDrawingSensorPositionUp();
	void GetDrawingSensorPositionDown();

	void GetDrawingSensorPositionReal();
	Point3f GetWorldSensorPosition();
	Point3f GetWorldSensorUlPt();
	Point3f GetWorldSensorUrPt();
	Point3f GetWorldSensorDlPt();
	Point3f GetWorldSensorDrPt();
protected:
	//opencl processing
	//the volume data has been normalized, don't use the params of "window_width" and "window_center"
	void GetSliceDirectly(size_t *global, size_t *local, size_t shared, cl_mem volume, cl_mem sliceu8, cl_uint width, cl_uint height, cl_uint slicenum,
		cl_uint dstwidth, cl_uint dstheight, cl_uint vheight,
		cl_uint z_ind, cl_float maxval, cl_float minval);
	void GetSliceYZX(size_t *global, size_t *local, size_t shared, cl_mem volume, cl_mem sliceu8, cl_uint width, cl_uint height, cl_uint slicenum,
		cl_uint dstwidth, cl_uint dstheight, cl_uint vheight,
		cl_float a, cl_float b, cl_float c, cl_float x_mid, cl_float y_mid, cl_float z_mid, cl_float maxval, cl_float minval);
	void GetSliceXZY(size_t *global, size_t *local, size_t shared, cl_mem volume, cl_mem sliceu8, cl_uint width, cl_uint height, cl_uint slicenum,
		cl_uint dstwidth, cl_uint dstheight, cl_uint vheight,
		cl_float a, cl_float b, cl_float c, cl_float x_mid, cl_float y_mid, cl_float z_mid, cl_float maxval, cl_float minval);
	void GetSliceXYZ(size_t *global, size_t *local, size_t shared, cl_mem volume, cl_mem sliceu8, cl_uint width, cl_uint height, cl_uint slicenum,
		cl_uint dstwidth, cl_uint dstheight, cl_uint vheight,
		cl_float a, cl_float b, cl_float c, cl_float x_mid, cl_float y_mid, cl_float z_mid, cl_float maxval, cl_float minval);
	void GetSliceCorXYZ(size_t *global, size_t *local, size_t shared, cl_mem worldslicecor, cl_uint dstwidth, cl_uint dstheight, cl_uint dstvheight,
		cl_float a, cl_float b, cl_float c, cl_float x_mid, cl_float y_mid, cl_float z_mid);
	void Launch_GetSlicePBO(size_t *global, size_t *local, cl_mem slicegpu, cl_mem slicenorm, cl_mem slicepbo, cl_uint width, cl_uint height);
	void Launch_GetResizeSlicePBO(size_t *global, size_t *local, cl_mem slicegpu, cl_mem slicenorm, cl_mem slicepbo, cl_uint width, cl_uint height, cl_uint resizewidth, cl_uint resizeheight);
	void Launch_ClearPBOBuffer(size_t *global, size_t *local, cl_mem slicegpu_u8, cl_mem slicenorm, cl_uint resizewidth, cl_uint resizeheight);
	void Launch_ClearSlicePBO(size_t *global, size_t *local, cl_mem slicepbo, cl_uint resizewidth, cl_uint resizeheight);

private:
	short dstplane[WIDTH*HEIGHT];
	//Interp and copy flag
	int m_Interpflag;
	int m_Directcopyflag;

	//体数据信息
	int m_Width;
	int m_Height;
	int m_Slicenum;
	float m_MaxValue;
	float m_MinValue;

	//CT图像变换信息
	short vmin;
	short vmax;
	//定义切面上一点pt(m_Xmid, m_Ymid, m_Zmid)
	float m_Xmid;
	float m_Ymid;
	float m_Zmid;
	//切面与每个坐标轴之间的
	float m_XaxisAngle;
	float m_YaxisAngle;
	float m_ZaxisAngle;
	//切面法向量表示
	float m_Normvecx;
	float m_Normvecy;
	float m_Normvecz;
	//切面的原始大小
	int m_DstWidth;
	int m_DstHeight;
	int m_VHeight;
	int m_Zind;

	//定义平面系数
	float m_Pa;
	float m_Pb;
	float m_Pc;
	float m_Pd;

	//四元数旋转
	Quaternion m_QuObj;

	//定义当前旋转矩阵和中间变量的旋转矩阵
	float m_TempRMat[9];
	float m_RMat[9];
	//定义当前探头是否朝上方的标志
	bool m_ProbeUpdirectionFlag;
	int m_ProbeLRDirectionOutFlag;

	//定义CT数据的空间位置信息
	float m_CtPixelSpacex;
	float m_CtPixelSpacey;
	float m_CtPixelSpacez;
	float m_CtUpperLeft_X;
	float m_CtUpperLeft_Y;
	float m_CtWinCenter;
	float m_CtWinWidth;
	float m_CtUpperLeft_Z[MAXSLICENUM];

	//the upper bound and bottom bound of the rendering range
	float m_UpperBoundRenderingX;
	float m_UpperBoundRenderingY;
	float m_UpperBoundRenderingZ;
	float m_BottomBoundRenderingX;
	float m_BottomBoundRenderingY;
	float m_BottomBoundRenderingZ;

	//探头传感器绘制点中心（标准和世界坐标）
	Point3f m_StdSensorPt;
	Point3f m_WorldSensorPt;

	//探头传感器立方体示意图顶点集合
	Point3f m_WorldSensorFULPt;
	Point3f m_WorldSensorFURPt;
	Point3f m_WorldSensorFDLPt;
	Point3f m_WorldSensorFDRPt;
	Point3f m_WorldSensorBULPt;
	Point3f m_WorldSensorBURPt;
	Point3f m_WorldSensorBDLPt;
	Point3f m_WorldSensorBDRPt;

	//定义切片数据的边界顶点坐标(标准)
	Point3f m_StdTopleftPt;
	Point3f m_StdToprightPt;
	Point3f m_StdBottomleftPt;
	Point3f m_StdBottomrightPt;

	//定义切片数据的边界顶点坐标(世界坐标)
	Point3f m_WorldTopleftPt;
	Point3f m_WorldToprightPt;
	Point3f m_WorldBottomleftPt;
	Point3f m_WorldBottomrightPt;

	//slice mem host(cpu)
	UINT8 *m_HSliceImageU8;
	float *m_HSliceImagefloat;

	//gridsize
	cl_uint GridSize[4];
	cl_float Resizeratio[4];
	cl_float VoxelSize[4];

	// Initial OpenCL component
	int                     m_GroupSize;

	// opencl mem
	cl_mem                  m_DSliceImageU8;
	cl_mem                  m_DSliceImagefloat;
	cl_mem                  m_DSliceZero;
	cl_mem                  m_DSliceZeroScan;
};
#endif // !_ROTATESLICE_H_
