#include "PrintLog.h"

#include <Windows.h>

ofstream CPrintLog::SysLogFile;

char CPrintLog::logfilepath[64];
char CPrintLog::LogfileMessages[1024];

SYSTEMTIME curSysTime;

void CPrintLog::PrintLog(const char * sysmessage)
{
	GetLocalTime(&curSysTime);
	sprintf_s(LogfileMessages, "%4d-%02d-%02d  %02d:%02d:%02d  %s", curSysTime.wYear, curSysTime.wMonth, curSysTime.wDay, curSysTime.wHour, curSysTime.wMinute, curSysTime.wSecond, sysmessage);

	if(SysLogFile.is_open())
		SysLogFile<<LogfileMessages<<endl;
}

void CPrintLog::OpenLogFile()
{
	int ret = GenLogFile();
	if(ret >= 1)
		SysLogFile.open(logfilepath, ios::app);//,ios::trunc);
}

void CPrintLog::CloseLogFile()
{
	SysLogFile.close();
}

int CPrintLog::GenLogFile()
{
	const char *SysLogPath = CarbonmedLogPath;

	GetLocalTime(&curSysTime);
	sprintf_s(logfilepath, "%s%4d_%02d_%02d_%s",SysLogPath, curSysTime.wYear, curSysTime.wMonth, curSysTime.wDay, "LogFile.txt");

	if(SearchLogFile(logfilepath) == 1)
		return 2;

	ofstream createlog(logfilepath);

	if(createlog)
		return 1;
	else
		return 0;

	createlog.close();
}

int CPrintLog::SearchLogFile(const char* _LogFilePath)
{
	FILE* FHandle;
	fopen_s(&FHandle, _LogFilePath, "r");
	if(FHandle == NULL)
		return 0;
	else{
		fclose(FHandle);
		return 1;
	}
}