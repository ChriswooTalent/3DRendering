#ifndef _CORREGISTRATION_H_
#define _CORREGISTRATION_H_

#include "stdafx.h"
#include "DataTypes.h"
#include "InternalThread.h"
#include "QuaternionProcess.h"
using namespace std;
#define DEFAULTCYLINDERSLICE 1080
using namespace QuaternionProcess;

enum RegistrationStep
{
	REGStep1 = 0,
	REGStep2 = 1,
	REGStep3 = 2,
	REGStep4 = 3
};

class CorRegisTration : public InternalThread
{
public:
	CorRegisTration();
	~CorRegisTration();

	//Get Registrationinfo
	void GetPunctureLineStart(Point3f &pt_start);
	void GetPunctureLineEnd(Point3f &pt_end);

	//Set Sensor pt and angle
	void SetSensorPoint(Point3f pt);
	void SetRefDistance(float dis);
	void SetPuncLineAngle(float ang);
	void SetPuncLineLength(float length);
	void SetPuncNeedleRadius(float radius);
	void SetSensorAngle(float azimuth, float elevation, float roll);
	void SetSensorRotationMat(float *rmat);
	void SetRefAxisRotationMat(float *rmat);
	void SetAxis(float axis[3]);
	void SetBasePuncvec(vector3d basepuncvec, bool cylinderflag);
	void GetRegistrationCorInfo(float *rmat, float *tvec);
	void SetRegistrationCorInfo(float *rmat, float *tvec);
	void GetRegistrationQCorInfo(Quaternion &rq, float *tvec);
	void SetRegistrationQCorInfo(Quaternion &rq, float *tvec);
	void GetPuncAngle(float &angle);

	//Registration step
	void SetRegistrationStep(int step);
	void SetRegStartFlag(bool flag);
	void SetRegFinshedFlag(bool flag);
	bool GetRegStartFlag();
	bool GetRegFinshedFlag();

	//Set Registration start angle
	void InitRegistrationStartAngle(float roll, float elevation, float azimuth);
	//Get Registration start angle
	void GetRegistrationStartAngle(float &rollstart, float &elevationstart, float &azimuthstart);
	//Set Registration pt
	void SetRegistrationPt1(Point3f pt, Point3f pt_match);
	void SetRegistrationPt2(Point3f pt, Point3f pt_match);
	void SetRegistrationPt3(Point3f pt, Point3f pt_match);
	void SetRegistrationPt4(Point3f pt, Point3f pt_match);

	Point3f GetRegistrationPt1();
	Point3f GetRegistrationPt2();
	Point3f GetRegistrationPt3();
	Point3f GetRegistrationPt4();

	//Quaternion Process
	// 获取旋转四元数
	void GetRotationQuaternion(Quaternion &Qrotation);
	// 设置旋转四元数
	void SetRotationQuaternion(Quaternion &rq);
	// 获取坐标轴旋转四元数
	void GetRefRotationQuaternion(Quaternion &Qrotation);
	// 设置坐标轴旋转四元数
	void SetRefAxisRotationQuaternion(Quaternion &rq);

	//PunctureLine
	void GetPuncNeedlePtArray(GLfloat *topcptarray, GLfloat *botcptarray, GLfloat *cylindercenter, GLfloat *ConeCircleArray, GLfloat *NeedleTop);
	void GetDrawingSensorPositionDown(Point3f sensor_pt, int interpflag);
	void CalcingCylinderPtArray();
	void CalcingPunctureLineStart(Point3f &pt_start);
	void CalcingPunctureLineEnd(Point3f &pt_end);
	void CalcingPunctureLineVector(float &x, float &y, float &z);

	//Registration Process
	//Registration with Quaternion
	void RegistrationQuaternion();
	//Registration with RotationMat/calcing RotationMat and Translation vector
	void Registration();
	//Registration simplely with Quaternion
	void RegistrationQuaternionSimple();
	//Registration simplely with RotationMat
	void RegistrationSimple();
	void CalcingRotationMatAndTVec(Point3f pt1, Point3f pt1_match, Point3f pt2, Point3f pt2_match, 
		                           Point3f pt3, Point3f pt3_match, Point3f pt4, Point3f pt4_match, 
								   float *Rmat, float *Tvec);
	void CalcingTranslateVecSimple(Point3f pt, Point3f pt_match, float *Tvec);

	void GetRotationMat(float azimuth, float elevation, float roll, float *RMat);

	void GetQuaternionRotation(float azimuth, float roll, float elevation, int radian_flag, vector3d &inputvec, vector3d &resultvec);

	void GetRegistrationQuaternion(Quaternion &rq);

	void matrix_matrix_mult(float *m1, float *m2, float *result);

	//Init World Coordinate information
	void InitWorlCorInformation(float pixelspacex, float pixelspacey, float pixelspacez, float startx, float starty, float startz, float xcenteroffset, float ycenteroffset, float zcenteroffset);

	//Waiting for the registration flag
	void WaitingForRegFlag();
protected:
	virtual DWORD InternalThreadEntry();

protected:
	//矩阵向量乘法
	inline vector3d matrix_mult(float *mat, vector3d a);
private:
	Point3f m_RegPt1;
	Point3f m_RegPt2;
	Point3f m_RegPt3;
	Point3f m_RegPt4;
	Point3f m_RegMatchPt1;
	Point3f m_RegMatchPt2;
	Point3f m_RegMatchPt3;
	Point3f m_RegMatchPt4;
	Point3f m_SensorPt;
	Point3f m_PuncStartPt;
	Point3f m_PuncStartOriPt;
	Point3f m_PuncEndPt;
	vector3d m_BasePuncvec;
	vector3d m_BaseHPuncvec;
	vector3d m_RefAxis;
	bool m_CylinderBaseptFlag;
	float m_RefDis;
	float m_PuncLength;
	float m_PuncNeedleRadius;
	float m_PuncConLength;
	float m_PuncVecx;
	float m_PuncVecy;
	float m_PuncVecz;
	float m_Azimuth;
	float m_Elevation;
	float m_Roll;
	float m_SensorRotationMat[9];
	float m_RefAxisRotationMat[9];
	float m_RotationMat[9];
	float m_TransVec[3];
	//Quaternion
	Quaternion m_RotationQuObj;
	Quaternion m_RefAxisQuObj;
	Quaternion m_RegQuObj;

	//Registration Angle Info
	float m_Rollstart;
	float m_Elevationstart;
	float m_Azimuthstart;

	//world cor information
	float m_CtPixelSpacex;
	float m_CtPixelSpacey;
	float m_CtPixelSpacez;
	float m_CtStart_X;
	float m_CtStart_Y;
	float m_CtStart_Z;
	float m_XCenterOffset;
	float m_YCenterOffset;
	float m_ZCenterOffset;

	//punc angle
	float m_Angle;

	//registration step
	int m_RegStep;
	bool m_RegStart;
	bool m_RegFinish;

	//Cylinder pt array
	GLfloat m_TopCylinderPtArray[3 * DEFAULTCYLINDERSLICE];
	GLfloat m_BottomCylinderPtArray[3 * DEFAULTCYLINDERSLICE];
	GLfloat m_ConeCirclePtArray[3 * DEFAULTCYLINDERSLICE];
	GLfloat m_CylinderCenter[3];
	GLfloat m_ConeNeedleTopPt[3];
};
#endif // !_CORREGISTRATION_H_
