#include "PrintLog.h"
#include "ProcessMutex.h"
#include "PrintLog_DLL.h"

CProcessMutex * _CProcessMutex;

PRINTLOG_API void OpenLog();
PRINTLOG_API void CloseLog();
PRINTLOG_API void PrintLog(const char * sysmessage);

// generate mutex and system log file
void OpenLog()
{
	_CProcessMutex = new CProcessMutex("CarbonMedSysLogMutex");
	CPrintLog::OpenLogFile();
}
// close mutex and system log file
void CloseLog()
{
	_CProcessMutex->~CProcessMutex();
	CPrintLog::CloseLogFile();
}
// print log info in file
void PrintLog(const char * sysmessage)
{
	_CProcessMutex->Lock();
	CPrintLog::PrintLog(sysmessage);
	_CProcessMutex->UnLock();
}
