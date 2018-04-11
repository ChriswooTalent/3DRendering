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
	//�ⲿ�ӿں���,��ȡ��Ƭ���ݣ���ɴ�GPU��CPU�Ŀ���
	void GetSliceDataU8(UINT8 *dst);
	void GetSliceNormalized(float *dst);

	//��ȡ�Դ��е���Ƭ����
	void GetSliceDataU8GPU(cl_mem &sliceu8GPU);
	void GetSliceDataPBO(cl_mem &slicepbo, bool resizeflag);
	void ClearSlicePbo(cl_mem &slicepbo);

	//�ӿں���1, ֱ�Ӵ��뷨����
	void GetSlicePlane(vector3d normvec, Point3f ptop, int width, int height, int slicenum, cl_mem volume);
	//�ӿں���2������һ��������һ����ת���󣬹���������
	void GetSlicePlane(vector3d axisvec, float *RMat, Point3f ptop, int width, int height, int slicenum, cl_mem volume);
	//�ӿں���3,����������
	void GetSlicePlane(float startx, float starty, float startz, float endx, float endy, float endz, int width, int height, int slicenum, cl_mem volume);
	//�ӿں���4,�����ʼ��������������ֵ���Լ�����������ļнǣ�����������
	//azimuthΪ��Z����ת�ļн�
	//elevationΪ��x����ת�ļн�
	//rollΪ��y����ת�ļн�
	//axisvec��ʼ����ΪZ��������0, 0, 1��
	void GetSlicePlane(cl_mem volume);
    //��ʼ����Ƭ������Ϣ
	void InitSlicePlaneInfo(vector3d axisvec, float x, float y, float z, float azimuth, float elevation, float roll, float *basicRmat);
	//����������Ϣ�����и�ƽ��,�õ�ƽ��ķ�����,ͬʱȷ����������ĳ��Ϳ�
	void ConstructSlicePlaneNormVec(float startx, float starty, float startz, float endx, float endy, float endz, float &a, float &b, float &c);

	//���ֱ�ӿ���CT�����е���Ƭ
	void CopySliceDirectly(int sliceind, int width, int height, int slicenum, int dstwidth, int dstheight, int vheight, cl_mem volume);
	//���ݷ������õ�����ƽ��
	void GenerateSlicePts(float a, float b, float c, int width, int height, int slicenum, int dstwidth, int dstheight, int vheight, int flag, cl_mem volume);
	//����PBO�ڴ�ռ�
	void ClearPboBuffer();

	//������ת�ǶȻ�ȡ��ת����
	void GetRotationMat(float azimuth, float elevation, float roll, float *RMat);

	//������Ԫ����ȡ��Ӧ����ת���
	void GetQuaternionRotation(float azimuth, float roll, float elevation, int radian_flag, vector3d &inputvec, vector3d &resultvec);

	//������ת�ǶȺ͹�ʽֱ�ӻ�ȡ��ת����
	void GetRotationMatDirectly(float azimuth, float elevation, float roll, float *RMat);

	//�������
	vector3d cross(vector3d a, vector3d b);

	//���㶥�㷨����
	vector3d calcNormal(vector3d &v0, vector3d &v1, vector3d &v2);

	//����������ģ
	float CalcNValueofVec(vector3d v);

	//�������
	float Dot3D(vector3d a, vector3d b);

	//�������˷�
	void matrix_matrix_mult(float *m1, float *m2, float *result);

	//���������˷�
	vector3d matrix_mult(float *mat, vector3d a);

	//������任���������˷�
	vector3d Camtransmatrix_mult(float *mat, vector3d a);

	//�������˷�
	void matrix_matrix_mult(double m1[3][3], double m2[3][3], double result[3][3]);

	//����������������֮��ļн�
	float ClacAngleOfAxis(vector3d normvec, vector3d axisvec);

	//��ȡ�������е�������Сֵ
	void CalcMaxMinValueOfVolume(float *volume);

	//����õ���Ƭ�ڿռ��е�4���߽��ϵĶ���(��׼)
	void CalcBoundcorOfSlice(float a, float b, float c, Point3f ptOnplane, int dstwidth, int dstheight, int flag);

	//��ȡ��Ƭ4���߽��ϵĶ�����CT�����е���ʵ����(�ⲿ���ýӿ�)
	void GetRealBoundCorOfSlice(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt, int flag);

	//��ȡ��Ƭ4���߽��ϵĶ�����CT�����еı�׼����(�ⲿ���ýӿ�)
	void GetStdBoundCorOfSlice(Point3f &tlpt, Point3f &trpt, Point3f &blpt, Point3f &brpt, int flag);

	bool SliceBorderConfine();

	//����ʵ������ߴ��ʼ��InterpGridSize������������ʵ�ļ���ƫת�Ƕ�
	void CalcInterpGridSize();

	//opencl��صĳ�ʼ��
	void SliceOpenCLini(cl_context cxGPUContext, cl_command_queue cqCommandQueue, cl_device_id device);

	//��ʼ����Ƭ����
	void InitSlice(int width, int height, int slicenum);

	//����CT�ռ���Ϣ, ͬʱ����Z�����ϵ��ܶ�
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

	//����ά�������Ϣ����
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

	//�����ȡ����ϵ������غ���
	void GetPlaneCoef(float &pa, float &pb, float &pc, float &pd)
	{
		pa = m_Pa;
		pb = m_Pb;
		pc = m_Pc;
		pd = m_Pd;
	}

	//��ȡ��ת����
	void GetRotationMat(float *rmat);

	//��ȡ��ת��Ԫ��
	void GetRotationQuaternion(Quaternion &Qrotation);
	//������ת��Ԫ��
	void SetRotationQuaternion(Quaternion &rq);

	//����һ����������ƽ��ϵ��
	void CalcingClipPlaneCoefs(float norma, float normb, float normc, Point3f ptOnplane);

	//�����ڲ�ͬ����»�ȡ�߽綥������ĺ���
	Point3f GetStdCorYZX(float a, float b, float c, float y, float z, Point3f ptOnplane);
	Point3f GetStdCorXZY(float a, float b, float c, float x, float z, Point3f ptOnplane);
	Point3f GetStdCorXYZ(float a, float b, float c, float x, float y, Point3f ptOnplane);

	//��ȡ����̽ͷ��������λ�õ�
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

	//��������Ϣ
	int m_Width;
	int m_Height;
	int m_Slicenum;
	float m_MaxValue;
	float m_MinValue;

	//CTͼ��任��Ϣ
	short vmin;
	short vmax;
	//����������һ��pt(m_Xmid, m_Ymid, m_Zmid)
	float m_Xmid;
	float m_Ymid;
	float m_Zmid;
	//������ÿ��������֮���
	float m_XaxisAngle;
	float m_YaxisAngle;
	float m_ZaxisAngle;
	//���淨������ʾ
	float m_Normvecx;
	float m_Normvecy;
	float m_Normvecz;
	//�����ԭʼ��С
	int m_DstWidth;
	int m_DstHeight;
	int m_VHeight;
	int m_Zind;

	//����ƽ��ϵ��
	float m_Pa;
	float m_Pb;
	float m_Pc;
	float m_Pd;

	//��Ԫ����ת
	Quaternion m_QuObj;

	//���嵱ǰ��ת������м��������ת����
	float m_TempRMat[9];
	float m_RMat[9];
	//���嵱ǰ̽ͷ�Ƿ��Ϸ��ı�־
	bool m_ProbeUpdirectionFlag;
	int m_ProbeLRDirectionOutFlag;

	//����CT���ݵĿռ�λ����Ϣ
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

	//̽ͷ���������Ƶ����ģ���׼���������꣩
	Point3f m_StdSensorPt;
	Point3f m_WorldSensorPt;

	//̽ͷ������������ʾ��ͼ���㼯��
	Point3f m_WorldSensorFULPt;
	Point3f m_WorldSensorFURPt;
	Point3f m_WorldSensorFDLPt;
	Point3f m_WorldSensorFDRPt;
	Point3f m_WorldSensorBULPt;
	Point3f m_WorldSensorBURPt;
	Point3f m_WorldSensorBDLPt;
	Point3f m_WorldSensorBDRPt;

	//������Ƭ���ݵı߽綥������(��׼)
	Point3f m_StdTopleftPt;
	Point3f m_StdToprightPt;
	Point3f m_StdBottomleftPt;
	Point3f m_StdBottomrightPt;

	//������Ƭ���ݵı߽綥������(��������)
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
