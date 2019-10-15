#include "NDIDrivebay.h"

NDIDriveBay::NDIDriveBay()
{}

void NDIDriveBay::CInitNDIDriveBay(const char *err, int &errorCode)
{
	int				i;
	int				sensorID;
	short			id;
	// Must uncomment call to SetSystemParameter(MEASUREMENT_RATE, ...) in code below for 
	// rate to be applied.
	double			rate = 255.0f;

	errorCode = InitializeBIRDSystem();
	if (errorCode != BIRD_ERROR_SUCCESS)
	{
		err = errorHandler(errorCode);
		return;
	}

	errorCode = GetBIRDSystemConfiguration(&tracker.m_config);
	if (errorCode != BIRD_ERROR_SUCCESS)
	{
		err = errorHandler(errorCode);
		return;
	}

	pSensor = new CSensor[tracker.m_config.numberSensors];
	for (i = 0; i < tracker.m_config.numberSensors; i++)
	{
		errorCode = GetSensorConfiguration(i, &(pSensor + i)->m_config);
		if (errorCode != BIRD_ERROR_SUCCESS)
		{
			err = errorHandler(errorCode);
			return;
		}
		//设置基准位置
		//DOUBLE_ANGLES_RECORD anglesRecord = { 0.0, 0.0, 0.0 };
		DOUBLE_ANGLES_RECORD anglesRecord = { -90.0, 90.0, 0.0 };
		double pbuffer[3] = { 0.0 };
		pbuffer[0] = anglesRecord.a;
		pbuffer[1] = anglesRecord.e;
		pbuffer[2] = anglesRecord.r;
		errorCode = SetSensorParameter(i, ANGLE_ALIGN, pbuffer, sizeof(anglesRecord));
		if (errorCode != BIRD_ERROR_SUCCESS)
		{
			err = errorHandler(errorCode);
			return;
		}
	}

	pXmtr = new CXmtr[tracker.m_config.numberTransmitters];
	for (i = 0; i < tracker.m_config.numberTransmitters; i++)
	{
		errorCode = GetTransmitterConfiguration(i, &(pXmtr + i)->m_config);
		if (errorCode != BIRD_ERROR_SUCCESS)
		{
			err = errorHandler(errorCode);
			return;
		}
	}

	for (id = 0; id < tracker.m_config.numberTransmitters; id++)
	{
		if ((pXmtr + id)->m_config.attached)
		{
			// Transmitter selection is a system function.
			// Using the SELECT_TRANSMITTER parameter we send the id of the
			// transmitter that we want to run with the SetSystemParameter() call
			errorCode = SetSystemParameter(SELECT_TRANSMITTER, &id, sizeof(id));
			if (errorCode != BIRD_ERROR_SUCCESS) errorHandler(errorCode);
			break;
		}
	}

	//
	// METRIC
	//
	// NOTE: metric/inch reporting only affects the DOUBLE data formats
	// The imperial reporting is in inches and the metric reporting is
	// in millimeters
	//
	{
		BOOL buffer = true;			// set metric reporting = true
		BOOL *pBuffer = &buffer;
		//printf("METRIC: %d\n", buffer);
		errorCode = SetSystemParameter(METRIC, pBuffer, sizeof(buffer));
		if (errorCode != BIRD_ERROR_SUCCESS)
			errorHandler(errorCode);
	}

	for (i = 0; i < tracker.m_config.numberSensors; i++)
	{
		DATA_FORMAT_TYPE type = DOUBLE_POSITION_ANGLES_TIME_Q;
		errorCode = SetSensorParameter(i, DATA_FORMAT, &type, sizeof(type));
		if (errorCode != BIRD_ERROR_SUCCESS)
		{
			err = errorHandler(errorCode);
		}
	}
}

void NDIDriveBay::GetNDIDriveBaySensorRMatrix(int sensorid, double *RMatrix, int &errorCode, const char *err)
{
	DOUBLE_MATRIX_RECORD MatrixRecord[8 * 4], *pRecord = MatrixRecord;

	// Set the data format type for each attached sensor.
	for (int i = 0; i < tracker.m_config.numberSensors; i++)
	{
		DATA_FORMAT_TYPE type = DOUBLE_MATRIX;
		errorCode = SetSensorParameter(i, DATA_FORMAT, &type, sizeof(type));
		if (errorCode != BIRD_ERROR_SUCCESS)
		{
			err = errorHandler(errorCode);
		}
	}
	errorCode = GetSynchronousRecord(ALL_SENSORS, pRecord, sizeof(MatrixRecord[0]) * tracker.m_config.numberSensors);
	if (errorCode != BIRD_ERROR_SUCCESS)
	{
		err = errorHandler(errorCode);
	}
	unsigned int status = GetSensorStatus(sensorid);

	if (status == VALID_STATUS)
	{
		RMatrix[0] = pRecord[sensorid].s[0][0];
		RMatrix[1] = pRecord[sensorid].s[0][1];
		RMatrix[2] = pRecord[sensorid].s[0][2];
		RMatrix[3] = pRecord[sensorid].s[1][0];
		RMatrix[4] = pRecord[sensorid].s[1][1];
		RMatrix[5] = pRecord[sensorid].s[1][2];
		RMatrix[6] = pRecord[sensorid].s[2][0];
		RMatrix[7] = pRecord[sensorid].s[2][1];
		RMatrix[8] = pRecord[sensorid].s[2][2];
	}
	// reset Set the data format type as record for each attached sensor.
	for (int i = 0; i < tracker.m_config.numberSensors; i++)
	{
		DATA_FORMAT_TYPE type = DOUBLE_POSITION_ANGLES_TIME_Q;
		errorCode = SetSensorParameter(i, DATA_FORMAT, &type, sizeof(type));
		if (errorCode != BIRD_ERROR_SUCCESS)
		{
			err = errorHandler(errorCode);
		}
	}
}

void NDIDriveBay::GetNDIDriveBaySensorQuaterNion(int sensorid, double *Quaternion, int &errorCode, const char *err)
{
	DOUBLE_QUATERNIONS_RECORD QuaternionRecord[4], *pRecord = QuaternionRecord;

	// Set the data format type for each attached sensor.
	for (int i = 0; i < tracker.m_config.numberSensors; i++)
	{
		DATA_FORMAT_TYPE type = DOUBLE_QUATERNIONS;
		errorCode = SetSensorParameter(i, DATA_FORMAT, &type, sizeof(type));
		if (errorCode != BIRD_ERROR_SUCCESS)
		{
			err = errorHandler(errorCode);
		}
	}
	errorCode = GetSynchronousRecord(ALL_SENSORS, pRecord, sizeof(QuaternionRecord[0]) * tracker.m_config.numberSensors);
	if (errorCode != BIRD_ERROR_SUCCESS)
	{
		err = errorHandler(errorCode);
	}
	unsigned int status = GetSensorStatus(sensorid);

	if (status == VALID_STATUS)
	{
		Quaternion[0] = pRecord[sensorid].q[0];
		Quaternion[1] = pRecord[sensorid].q[1];
		Quaternion[2] = pRecord[sensorid].q[2];
		Quaternion[3] = pRecord[sensorid].q[3];
	}
	// reset Set the data format type as record for each attached sensor.
	for (int i = 0; i < tracker.m_config.numberSensors; i++)
	{
		DATA_FORMAT_TYPE type = DOUBLE_POSITION_ANGLES_TIME_Q;
		errorCode = SetSensorParameter(i, DATA_FORMAT, &type, sizeof(type));
		if (errorCode != BIRD_ERROR_SUCCESS)
		{
			err = errorHandler(errorCode);
		}
	}
}

void NDIDriveBay::CGetNDIDriveBaySynRecord(void *pRecord, int elementsize, int recordSize, int &validsensornum, int *sensorindex, int &errorCode, const char *err)
{
	//DOUBLE_POSITION_ANGLES_TIME_Q_RECORD record[8 * 4], *pRecord1 = record;
	errorCode = GetSynchronousRecord(ALL_SENSORS, pRecord, elementsize * tracker.m_config.numberSensors);
	//errorCode = GetSynchronousRecord(ALL_SENSORS, pRecord1, sizeof(record[0])* tracker.m_config.numberSensors);
	if (errorCode != BIRD_ERROR_SUCCESS)
	{
		err = errorHandler(errorCode);
		return;
	}
	validsensornum = 0;
	for (int sensorID = 0; sensorID < tracker.m_config.numberSensors; sensorID++)
	{
		// get the status of the last data record
		// only report the data if everything is okay
		unsigned int status = GetSensorStatus(sensorID);

		if (status == VALID_STATUS)
		{
			validsensornum++;
			sensorindex[validsensornum] = sensorID;
		}
		else
		{
			sensorindex[sensorID] = -1;
		}
	}
}

void NDIDriveBay::GetNDIDriveBayRecordOffset(void *pRecord, int elementsize, int recordsize, int sensorid, double offsetx, double offsety, double offsetz, int &errorCode, const char *err)
{
	if (sensorid >= tracker.m_config.numberSensors)
	{
		//sprintf(err, "sensor id over range!");
		return;
	}
	double pbuffer[3] = { 0.0 };
	pbuffer[0] = offsetx;
	pbuffer[1] = offsety;
	pbuffer[2] = offsetz;
	double pbuffertest[3] = { 0.0 };
	errorCode = SetSensorParameter(sensorid, SENSOR_OFFSET, pbuffer, sizeof(DOUBLE_POSITION_RECORD));
	if (errorCode != BIRD_ERROR_SUCCESS)
	{
		err = errorHandler(errorCode);
		return;
	}
	errorCode = GetSensorParameter(sensorid, SENSOR_OFFSET, pbuffertest, sizeof(DOUBLE_POSITION_RECORD));
	if (errorCode != BIRD_ERROR_SUCCESS)
	{
		err = errorHandler(errorCode);
		return;
	}
	errorCode = GetSynchronousRecord(ALL_SENSORS, pRecord, elementsize * tracker.m_config.numberSensors);
	if (errorCode != BIRD_ERROR_SUCCESS)
	{
		err = errorHandler(errorCode);
		return;
	}
}

void NDIDriveBay::CReleaseNDIDriveBayResource()
{
	if (pSensor)
		delete[]pSensor;
	if (pXmtr)
		delete[]pXmtr;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//	ERROR HANDLER
//	=============
//
// This is a simplified error handler.
// This error handler takes the error code and passes it to the GetErrorText()
// procedure along with a buffer to place an error message string.
// This error message string can then be output to a user display device
// like the console
// Specific error codes should be parsed depending on the application.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
const char* NDIDriveBay::errorHandler(int error)
{
	char			buffer[1024];
	char			successbuffer[1024] = "ok!";
	char			*pBuffer = &buffer[0];
	char			*successsBuffer = &successbuffer[0];
	int				numberBytes;

	while (error != BIRD_ERROR_SUCCESS)
	{
		error = GetErrorText(error, pBuffer, sizeof(buffer), SIMPLE_MESSAGE);
		numberBytes = strlen(buffer);
		buffer[numberBytes] = '\n';		// append a newline to buffer
		return buffer;
	}
	return successsBuffer;
}