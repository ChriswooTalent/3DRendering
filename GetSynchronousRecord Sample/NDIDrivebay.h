#ifndef _NDIDRIVEBAY_H_
#define _NDIDRIVEBAY_H_

#include "StdAfx.h"

#ifdef NDILIB_EXPORTS  
#define NDILIB_API __declspec(dllexport)  
#else  
#define NDILIB_API __declspec(dllimport)  
#endif

#define SET_SYSTEM_PARAMETER(type, value)										\
	{																			\
	   type##_TYPE buffer = value;												\
	   type##_TYPE *pBuffer = &buffer;											\
	   errorCode = SetSystemParameter(type, pBuffer, sizeof(buffer));			\
       if (errorCode != BIRD_ERROR_SUCCESS) errorHandler(errorCode);		    \
	}

#define	SET_SENSOR_PARAMETER(sensor, type, value)								\
	{																			\
	   type##_TYPE buffer = value;												\
	   type##_TYPE *pBuffer = &buffer;											\
	   errorCode = SetSensorParameter(sensor, type, pBuffer, sizeof(buffer));	\
       if (errorCode != BIRD_ERROR_SUCCESS) errorHandler(errorCode);	        \
	}

class CSystem
{
public:
	SYSTEM_CONFIGURATION	m_config;
};

class CSensor
{
public:
	SENSOR_CONFIGURATION	m_config;
};

class CXmtr
{
public:
	TRANSMITTER_CONFIGURATION	m_config;
};

class NDIDriveBay
{
public:
	NDIDriveBay();
	~NDIDriveBay(){};

	void CInitNDIDriveBay(const char *err, int &errorCode);
	void CGetNDIDriveBaySynRecord(void *pRecord, int elementsize, int recordSize, int &validsensornum, int *sensorindex, int &errorCode, const char *err);
	void GetNDIDriveBayRecordOffset(void *pRecord, int elementsize, int recordsize, int sensorid, double offsetx, double offsety, double offsetz, int &errorcode, const char *err);
	void GetNDIDriveBaySensorRMatrix(int sensorid, double *RMatrix, int &errorCode, const char *err);
	void CReleaseNDIDriveBayResource();
protected:
	const char* errorHandler(int error);
private:
	CSystem			tracker;
	CSensor			*pSensor;
	CXmtr			*pXmtr;
};
#endif