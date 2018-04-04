#include "StdAfx.h"
#include "NDI_DLL.h"
#include "NDIDrivebay.h"

NDIDriveBay  _NDIObj;

NDIDRIVEVAY_API void InitNDIDriveBay(const char *err, int &errorCode);
NDIDRIVEVAY_API void GetNDIDriveBaySynRecord(void *pRecord, int elementsize, int recordSize, int &validsensornum, int *sensorindex, int &errorCode, const char *err);
NDIDRIVEVAY_API void GetNDIDriveBayRecordOffset(void *pRecord, int elementsize, int recordsize, int sensorid, double offsetx, double offsety, double offsetz, int &errorcode, const char *err);
NDIDRIVEVAY_API void GetNDIDriveBayRotationMatrix(int sensorid, double *RMatrix, int &errorCode, const char *err);
NDIDRIVEVAY_API void ReleaseNDIDriveBayResource();

void InitNDIDriveBay(const char *err, int &errorCode)
{
	//_NDIObj = new NDIDriveBay();
	_NDIObj.CInitNDIDriveBay(err, errorCode);
}

void GetNDIDriveBaySynRecord(void *pRecord, int elementsize, int recordSize, int &validsensornum, int *sensorindex, int &errorCode, const char *err)
{
	_NDIObj.CGetNDIDriveBaySynRecord(pRecord, elementsize, recordSize, validsensornum, sensorindex, errorCode, err);
}

void GetNDIDriveBayRecordOffset(void *pRecord, int elementsize, int recordsize, int sensorid, double offsetx, double offsety, double offsetz, int &errorcode, const char *err)
{
	_NDIObj.GetNDIDriveBayRecordOffset(pRecord, elementsize, recordsize, sensorid, offsetx, offsety, offsetz, errorcode, err);
}

void GetNDIDriveBayRotationMatrix(int sensorid, double *RMatrix, int &errorCode, const char *err)
{
	_NDIObj.GetNDIDriveBaySensorRMatrix(sensorid, RMatrix, errorCode, err);
}

void ReleaseNDIDriveBayResource()
{
	_NDIObj.CReleaseNDIDriveBayResource();
}
