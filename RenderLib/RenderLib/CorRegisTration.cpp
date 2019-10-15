#include "CorRegisTration.h"

CorRegisTration::CorRegisTration()
{
	Point3f ptzero(0.0f, 0.0f, 0.0f, 0);
	m_RegPt1 = ptzero;
	m_RegPt2 = ptzero;
	m_RegPt3 = ptzero;
	m_RegPt4 = ptzero;
	m_RegMatchPt1 = ptzero;
	m_RegMatchPt2 = ptzero;
	m_RegMatchPt3 = ptzero;
	m_RegMatchPt4 = ptzero;
	m_SensorPt = ptzero;
	m_PuncStartPt = ptzero;
	m_PuncStartOriPt = ptzero;
	m_PuncEndPt = ptzero;
	m_RefDis = 0.0f;
	m_PuncVecx = 0.0f;
	m_PuncVecy = 0.0f;
	m_PuncVecz = 0.0f;
	m_Azimuth = 0.0f;
	m_Elevation = 0.0f;
	m_Roll = 0.0f;
	for (int i = 0; i < 9; i++)
	{
		m_SensorRotationMat[i] = 0.0f;
		m_RotationMat[i] = 0.0f;
		m_RefAxisRotationMat[i] = 0.0f;
	}
	m_SensorRotationMat[0] = 1.0f;
	m_SensorRotationMat[4] = 1.0f;
	m_SensorRotationMat[8] = 1.0f;
	m_RefAxisRotationMat[0] = 1.0f;
	m_RefAxisRotationMat[4] = 1.0f;
	m_RefAxisRotationMat[8] = 1.0f;
	for (int i = 0; i < 3; i++)
	{
		m_TransVec[i] = 0.0f;
	}
	m_RefAxis.fX = 0.0f;
	m_RefAxis.fY = 1.0f;
	m_RefAxis.fZ = 0.0f;

	m_Angle = 20.0f;
	m_PuncLength = 200.0f;
	m_PuncNeedleRadius = 1.0f;
	m_RegStep = 0;
	m_RegStart = false;
	m_RegFinish = false;
	m_CylinderBaseptFlag = false;
	//world cor information
	m_CtStart_X = 0.0f;
	m_CtStart_Y = 0.0f;
	m_CtStart_Z = 0.0f;
	m_XCenterOffset = 0.0f;
	m_YCenterOffset = 0.0f;
	m_ZCenterOffset = 0.0f;
	//init CylinderPt array
	memset(m_TopCylinderPtArray, 0, DEFAULTCYLINDERSLICE * 3 * sizeof(GLfloat));
	memset(m_BottomCylinderPtArray, 0, DEFAULTCYLINDERSLICE * 3 * sizeof(GLfloat));
	m_CylinderCenter[0] = 0.0f;
	m_CylinderCenter[1] = 0.0f;
	m_CylinderCenter[2] = 0.0f;
	m_ConeNeedleTopPt[0] = 0.0f;
	m_ConeNeedleTopPt[1] = 0.0f;
	m_ConeNeedleTopPt[2] = 0.0f;
}

CorRegisTration::~CorRegisTration()
{

}

void CorRegisTration::GetPuncAngle(float &angle)
{
	angle = m_Angle;
}

void CorRegisTration::GetRegistrationQCorInfo(Quaternion &rq, float *tvec)
{
	rq = m_RegQuObj;
	memcpy(tvec, m_TransVec, 3 * sizeof(float));
}

void CorRegisTration::SetRegistrationQCorInfo(Quaternion &rq, float *tvec)
{
	m_RegQuObj = rq;
	memcpy(m_TransVec, tvec, 3 * sizeof(float));
}

void CorRegisTration::GetRegistrationCorInfo(float *rmat, float *tvec)
{
	memcpy(rmat, m_RotationMat, 9 * sizeof(float));
	memcpy(tvec, m_TransVec, 3 * sizeof(float));
}

void CorRegisTration::SetRegistrationCorInfo(float *rmat, float *tvec)
{
	memcpy(m_RotationMat, rmat, 9 * sizeof(float));
	memcpy(m_TransVec, tvec, 3 * sizeof(float));
}

//Set Sensor pt and angle
void CorRegisTration::SetSensorPoint(Point3f pt)
{
	m_SensorPt = pt;
}

void CorRegisTration::SetSensorAngle(float azimuth, float elevation, float roll)
{
	m_Azimuth = azimuth;
	m_Elevation = elevation;
	m_Roll = roll;
}

void CorRegisTration::SetSensorRotationMat(float *rmat)
{
	memcpy(m_SensorRotationMat, rmat, 9 * sizeof(float));
}

void CorRegisTration::SetRefAxisRotationMat(float *rmat)
{
	memcpy(m_RefAxisRotationMat, rmat, 9 * sizeof(float));
}

//Quaternion
void CorRegisTration::GetRotationQuaternion(Quaternion &Qrotation)
{
	Qrotation = m_RotationQuObj;
}

void CorRegisTration::SetRotationQuaternion(Quaternion &rq)
{
	m_RotationQuObj = rq;
}

void CorRegisTration::GetRefRotationQuaternion(Quaternion &Qrotation)
{
	Qrotation = m_RefAxisQuObj;
}

void CorRegisTration::SetRefAxisRotationQuaternion(Quaternion &rq)
{
	m_RefAxisQuObj = rq;
}

void CorRegisTration::SetBasePuncvec(vector3d basepuncvec, bool cylinderflag)
{
	m_BasePuncvec = basepuncvec;
	m_CylinderBaseptFlag = cylinderflag;
}

void CorRegisTration::SetAxis(float axis[3])
{
	m_RefAxis.fX = axis[0];
	m_RefAxis.fY = axis[1];
	m_RefAxis.fZ = axis[2];
}

void CorRegisTration::SetRefDistance(float dis)
{
	m_RefDis = dis;
}

void CorRegisTration::SetPuncLineAngle(float ang)
{
	m_Angle = ang;
}

void CorRegisTration::SetPuncLineLength(float plength)
{
	m_PuncLength = plength;
}

void CorRegisTration::SetPuncNeedleRadius(float radius)
{
	m_PuncNeedleRadius = radius;
}

void CorRegisTration::SetRegistrationStep(int step)
{
	m_RegStep = step;
	if (m_RegStep == 1)
	{
		m_RegStart = true;
		m_RegFinish = false;
	}
	if (m_RegStep == 4)
	{
		m_RegStart = false;
		m_RegFinish = true;
	}
}

void CorRegisTration::SetRegStartFlag(bool flag)
{
	m_RegStart = flag;
}

void CorRegisTration::SetRegFinshedFlag(bool flag)
{
	m_RegFinish = flag;
}

bool CorRegisTration::GetRegStartFlag()
{
	return m_RegStart;
}
bool CorRegisTration::GetRegFinshedFlag()
{
	return m_RegFinish;
}

void CorRegisTration::InitRegistrationStartAngle(float roll, float elevation, float azimuth)
{
	if (m_RegStart)
	{
		m_Rollstart = roll;
		m_Elevationstart = elevation;
		m_Azimuthstart = azimuth;
	}
}

void CorRegisTration::GetRegistrationStartAngle(float &rollstart, float &elevationstart, float &azimuthstart)
{
	rollstart = m_Rollstart;
	elevationstart = m_Elevationstart;
	azimuthstart = m_Azimuthstart;
}

//Set Registration pt
void CorRegisTration::SetRegistrationPt1(Point3f pt, Point3f pt_match)
{
	if (m_RegStart)
	{
		m_RegPt1 = pt;
		m_RegMatchPt1 = pt_match;
	}
}
void CorRegisTration::SetRegistrationPt2(Point3f pt, Point3f pt_match)
{
	if (m_RegStart)
	{
		m_RegPt2 = pt;
		m_RegMatchPt2 = pt_match;
	}
}
void CorRegisTration::SetRegistrationPt3(Point3f pt, Point3f pt_match)
{
	if (m_RegStart)
	{
		m_RegPt3 = pt;
		m_RegMatchPt3 = pt_match;
	}
}
void CorRegisTration::SetRegistrationPt4(Point3f pt, Point3f pt_match)
{
	if (m_RegStart)
	{
		m_RegPt4 = pt;
		m_RegMatchPt4 = pt_match;
	}
}

Point3f CorRegisTration::GetRegistrationPt1()
{
	return m_RegMatchPt1;
}
Point3f CorRegisTration::GetRegistrationPt2()
{
	return m_RegMatchPt2;
}
Point3f CorRegisTration::GetRegistrationPt3()
{
	return m_RegMatchPt3;
}
Point3f CorRegisTration::GetRegistrationPt4()
{
	return m_RegMatchPt4;
}

void CorRegisTration::GetPuncNeedlePtArray(GLfloat *topcptarray, GLfloat *botcptarray, GLfloat *cylindercenter, GLfloat *ConeCircleArray, GLfloat *NeedleTop)
{
	memcpy(topcptarray, m_TopCylinderPtArray, DEFAULTCYLINDERSLICE*3*sizeof(GLfloat));
	memcpy(botcptarray, m_BottomCylinderPtArray, DEFAULTCYLINDERSLICE*3*sizeof(GLfloat));
	memcpy(cylindercenter, m_CylinderCenter, 3 * sizeof(GLfloat));
	memcpy(ConeCircleArray, m_ConeCirclePtArray, 3 * DEFAULTCYLINDERSLICE*sizeof(GLfloat));
	memcpy(NeedleTop, m_ConeNeedleTopPt, 3 * sizeof(GLfloat));
}

void CorRegisTration::GetDrawingSensorPositionDown(Point3f sensor_pt, int interpflag)
{
}

void CorRegisTration::CalcingCylinderPtArraySpace()
{
	m_PuncConLength = m_PuncLength / 4.0;
	float radius = m_PuncNeedleRadius;
	//half length of the needle without cone
	float halfLength = (m_PuncLength - m_PuncConLength) / 2;
	float fstep = 2.0f*PI / DEFAULTCYLINDERSLICE;
	vector3d topcylindervec;
	vector3d bottomcylindervec;
	vector3d cylindercentervec;
	if (m_CylinderBaseptFlag)
	{
		for (int i = 0; i < DEFAULTCYLINDERSLICE; i++)
		{
			if ((m_RefAxis.fZ == 1.0f) && (m_RefAxis.fX == 0.0f) && (m_RefAxis.fY == 0.0f))
			{
				float theta = ((float)i)*fstep;
				topcylindervec.fX = radius*cos(theta);
				topcylindervec.fY = radius*sin(theta);
				topcylindervec.fZ = 0.0f;
				bottomcylindervec.fX = radius*cos(theta);
				bottomcylindervec.fY = radius*sin(theta);
				bottomcylindervec.fZ = 2 * halfLength;
			}
			else if ((m_RefAxis.fY == 1.0f) && (m_RefAxis.fX == 0.0f) && (m_RefAxis.fZ == 0.0f))
			{
				float theta = ((float)i)*fstep;
				topcylindervec.fX = radius*cos(theta);
				topcylindervec.fY = 0.0f;
				topcylindervec.fZ = radius*sin(theta);
				bottomcylindervec.fX = radius*cos(theta);
				bottomcylindervec.fY = 2 * halfLength;
				bottomcylindervec.fZ = radius*sin(theta);
			}
			else if ((m_RefAxis.fX == 1.0f) && (m_RefAxis.fY == 0.0f) && (m_RefAxis.fZ == 0.0f))
			{
				float theta = ((float)i)*fstep;
				topcylindervec.fX = 0.0f;
				topcylindervec.fY = radius*cos(theta);
				topcylindervec.fZ = radius*sin(theta);
				bottomcylindervec.fX = 2 * halfLength;
				bottomcylindervec.fY = radius*cos(theta);
				bottomcylindervec.fZ = radius*sin(theta);
			}
			//define quaternion of start pt
			Quaternion q_topcylindervec(0.0f, topcylindervec.fX, topcylindervec.fY, topcylindervec.fZ);
			//result of start pt after rotation of quaternion
			Quaternion q_rtopvec = m_RotationQuObj.GetQRotationResult(q_topcylindervec);
			//define quaternion of end pt
			Quaternion q_bottomcylindervec(0.0f, bottomcylindervec.fX, bottomcylindervec.fY, bottomcylindervec.fZ);
			//result of end pt after rotation of quaternion
			Quaternion q_rbottomvec = m_RotationQuObj.GetQRotationResult(q_bottomcylindervec);
			Point3f toppt;
			Point3f bottompt;
			toppt.x = -q_rtopvec.GetQuaternionZ();
			toppt.y = q_rtopvec.GetQuaternionX();
			toppt.z = q_rtopvec.GetQuaternionY();
			bottompt.x = -q_rbottomvec.GetQuaternionZ();
			bottompt.y = q_rbottomvec.GetQuaternionX();
			bottompt.z = q_rbottomvec.GetQuaternionY();
			toppt = toppt + m_PuncStartPt;
			bottompt = bottompt + m_PuncStartPt;
			m_TopCylinderPtArray[i * 3 + 0] = toppt.x;
			m_TopCylinderPtArray[i * 3 + 1] = toppt.y;
			m_TopCylinderPtArray[i * 3 + 2] = toppt.z;
			m_BottomCylinderPtArray[i * 3 + 0] = bottompt.x;
			m_BottomCylinderPtArray[i * 3 + 1] = bottompt.y;
			m_BottomCylinderPtArray[i * 3 + 2] = bottompt.z;
		}
	}
	cylindercentervec.fX = 0.0f;
	cylindercentervec.fY = 0.0f;
	cylindercentervec.fZ = 0.0f;
	if ((m_RefAxis.fZ == 1.0f) && (m_RefAxis.fX == 0.0f) && (m_RefAxis.fY == 0.0f))
	{
		cylindercentervec.fZ = halfLength;
	}
	else if ((m_RefAxis.fY == 1.0f) && (m_RefAxis.fY == 0.0f) && (m_RefAxis.fZ == 0.0f))
	{
		cylindercentervec.fY = halfLength;
	}
	else if ((m_RefAxis.fX == 1.0f) && (m_RefAxis.fY == 0.0f) && (m_RefAxis.fZ == 0.0f))
	{
		cylindercentervec.fX = halfLength;
	}
	//vector3d centervec = matrix_mult(m_RefAxisRotationMat, cylindercentervec);
	//vector3d rcentervec = matrix_mult(m_SensorRotationMat, centervec);
	Quaternion q_cylindercentervec(0.0f, cylindercentervec.fX, cylindercentervec.fY, cylindercentervec.fZ);
	Quaternion q_rcentervec = m_RotationQuObj.GetQRotationResult(q_cylindercentervec);

	Point3f cpt;
	cpt.x = -q_rcentervec.GetQuaternionZ();
	cpt.y = q_rcentervec.GetQuaternionX();
	cpt.z = q_rcentervec.GetQuaternionY();
	cpt = cpt + m_PuncStartPt;
	m_CylinderCenter[0] = cpt.x;
	m_CylinderCenter[1] = cpt.y;
	m_CylinderCenter[2] = cpt.z;

	//cone circle and needletop
	memcpy(m_ConeCirclePtArray, m_BottomCylinderPtArray, 3 * DEFAULTCYLINDERSLICE * sizeof(GLfloat));
	m_ConeNeedleTopPt[0] = m_PuncEndPt.x;
	m_ConeNeedleTopPt[1] = m_PuncEndPt.y;
	m_ConeNeedleTopPt[2] = m_PuncEndPt.z;
}

//PunctureLine
void CorRegisTration::CalcingPunctureLineStartSpace(Point3f &pt_start)
{
	Quaternion q_RefAxis(0.0f, m_RefAxis.fX, m_RefAxis.fY, m_RefAxis.fZ);
	Quaternion q_tempvec = m_RotationQuObj.GetQRotationResult(q_RefAxis);
	vector3d tempvec(-q_tempvec.GetQuaternionZ(), q_tempvec.GetQuaternionX(), q_tempvec.GetQuaternionY());
	vector3d tempvar;
	tempvar.fX = tempvec.fX*m_RefDis;
	tempvar.fY = tempvec.fY*m_RefDis;
	tempvar.fZ = tempvec.fZ*m_RefDis;
	Point3f temppt;
	temppt.x = tempvar.fX;
	temppt.y = tempvar.fY;
	temppt.z = tempvar.fZ;
	temppt.indsort = 0;
	m_PuncStartPt = m_SensorPt - temppt;
	m_PuncStartOriPt = m_PuncStartPt;
	m_PuncStartPt.x = m_PuncStartPt.x + m_CtStart_X + m_XCenterOffset;
	m_PuncStartPt.y = m_PuncStartPt.y + m_CtStart_Y + m_YCenterOffset;
	m_PuncStartPt.z = m_PuncStartPt.z + m_CtStart_Z + m_ZCenterOffset;
	pt_start = m_PuncStartPt;
}

void CorRegisTration::CalcingPunctureLineEndSpace(Point3f &pt_end)
{
	//vector3d tempvec = matrix_mult(m_SensorRotationMat, m_BasePuncvec);
	Quaternion q_BasePuncvec(0.0f, m_BasePuncvec.fX, m_BasePuncvec.fY, m_BasePuncvec.fZ);
	Quaternion q_tempvec = m_RotationQuObj.GetQRotationResult(q_BasePuncvec);
	vector3d tempvec(-q_tempvec.GetQuaternionZ(), q_tempvec.GetQuaternionX(), q_tempvec.GetQuaternionY());
	vector3d tempvar;
	tempvar.fX = tempvec.fX*m_PuncLength;
	tempvar.fY = tempvec.fY*m_PuncLength;
	tempvar.fZ = tempvec.fZ*m_PuncLength;
	Point3f temppt;
	temppt.x = tempvar.fX;
	temppt.y = tempvar.fY;
	temppt.z = tempvar.fZ;
	temppt.indsort = 0;
	m_PuncEndPt = m_PuncStartPt + temppt;
	pt_end = m_PuncEndPt;
}

void CorRegisTration::GetPunctureLineStart(Point3f &pt_start)
{
	pt_start = m_PuncStartPt;
}

void CorRegisTration::GetPunctureLineEnd(Point3f &pt_end)
{
	pt_end = m_PuncEndPt;
}

void CorRegisTration::CalcingPunctureLineVector(float &x, float &y, float &z)
{
	x = m_BasePuncvec.fX;
	y = m_BasePuncvec.fY;
	z = m_BasePuncvec.fZ;
}

//Registration Process
//calcing RotationMat and Translation vector
void CorRegisTration::Registration()
{
	CalcingRotationMatAndTVec(m_RegPt1, m_RegMatchPt1, m_RegPt2, m_RegMatchPt2, m_RegPt3, m_RegMatchPt3, m_RegPt4, m_RegMatchPt4, m_RotationMat, m_TransVec);
}

void CorRegisTration::RegistrationQuaternion()
{}

void CorRegisTration::RegistrationSimple()
{
	GetRotationMat(0, PI / 2, PI, m_RotationMat);
	CalcingTranslateVecSimple(m_RegPt1, m_RegMatchPt1, m_TransVec);
}

void CorRegisTration::RegistrationQuaternionSimple()
{
	m_RegQuObj.FromEuler(0.0f, PI / 2, PI, 1);
	CalcingTranslateVecSimple(m_RegPt1, m_RegMatchPt1, m_TransVec);
}

void CorRegisTration::GetQuaternionRotation(float azimuth, float roll, float elevation, int radian_flag, vector3d &inputvec, vector3d &resultvec)
{
	Quaternion q_obj_src(0.0f, inputvec.fX, inputvec.fY, inputvec.fZ);
	Quaternion q_obj_result;
	m_RotationQuObj.FromEuler(azimuth, roll, elevation, radian_flag);
	q_obj_result = m_RotationQuObj.GetQRotationResult(q_obj_src);
	resultvec.fX = q_obj_result.GetQuaternionX();
	resultvec.fY = q_obj_result.GetQuaternionY();
	resultvec.fZ = q_obj_result.GetQuaternionZ();
}

void CorRegisTration::GetRegistrationQuaternion(Quaternion &rq)
{
	rq = m_RegQuObj;
}

void CorRegisTration::GetRotationMat(float azimuth, float roll, float elevation, float *RMat)
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
}

void CorRegisTration::matrix_matrix_mult(float *m1, float *m2, float *result)
{
	register int i, k;

	for (i = 0; i < 3; i++)
	{
		for (k = 0; k < 3; k++)
		{
			result[i * 3 + k] = m1[i * 3] * m2[k] + m1[i * 3 + 1] * m2[3 + k] + m1[i * 3 + 2] * m2[6 + k];
		}
	}
}

void CorRegisTration::CalcingRotationMatAndTVec(Point3f pt1, Point3f pt1_match, Point3f pt2, Point3f pt2_match,
	Point3f pt3, Point3f pt3_match, Point3f pt4, Point3f pt4_match,
	float *Rmat, float *Tvec)
{

}

void CorRegisTration::CalcingTranslateVecSimple(Point3f pt, Point3f pt_match, float *Tvec)
{
	vector3d originpt;
	vector3d transformpt;
	originpt.fX = pt.x;
	originpt.fY = pt.y;
	originpt.fZ = pt.z;
	transformpt = matrix_mult(m_RotationMat, originpt);
	Tvec[0] = pt_match.x - transformpt.fX;
	Tvec[1] = pt_match.y - transformpt.fY;
	Tvec[2] = pt_match.z - transformpt.fZ;
}

void CorRegisTration::InitWorlCorInformation(float pixelspacex, float pixelspacey, float pixelspacez, float startx, float starty, float startz, float xcenteroffset, float ycenteroffset, float zcenteroffset)
{
	m_CtPixelSpacex = pixelspacex;
	m_CtPixelSpacey = pixelspacey;
	m_CtPixelSpacez = pixelspacez;
	m_CtStart_X = startx;
	m_CtStart_Y = starty;
	m_CtStart_Z = startz;
	m_XCenterOffset = xcenteroffset;
	m_YCenterOffset = ycenteroffset;
	m_ZCenterOffset = zcenteroffset;
}

inline vector3d CorRegisTration::matrix_mult(float *mat, vector3d a)
{
	int i;
	float v0 = a.fX;
	float v1 = a.fY;
	float v2 = a.fZ;
	vector3d mulresult;

	mulresult.fX = mat[0] * v0 + mat[1] * v1 + mat[2] * v2;
	mulresult.fY = mat[3] * v0 + mat[4] * v1 + mat[5] * v2;
	mulresult.fZ = mat[6] * v0 + mat[7] * v1 + mat[8] * v2;
	return mulresult;
}

void CorRegisTration::WaitingForRegFlag()
{
	char *errmsg = NULL;
	StartInternalThread(errmsg);
}

DWORD CorRegisTration::InternalThreadEntry()
{
	SetEvent(m_NotifyEvent);
	WaitForSingleObject(m_NotifyEvent, INFINITE);
	while (m_RegStart&&m_RegFinish)
	{
		break;
	}
	SetEvent(m_NotifyEvent);
	return 1;
}