#include "InternalThread.h"

// PrintLog
extern "C" __declspec(dllimport) void OpenLog();
extern "C" __declspec(dllimport) void CloseLog();
extern "C" __declspec(dllimport) void PrintLog(const char * sysmessage);

InternalThread::InternalThread()
{
}

/*InternalThread::~InternalThread()
{
	StopInternalThread();
}*/

bool InternalThread::is_started() const
{
	return true;
}

bool InternalThread::must_stop()
{
	return false;
}

void InternalThread::StartInternalThread(char *err)
{
	// 创建事件
	static TCHAR EventName[] = TEXT("NDIobj"); //窗体名
	m_NotifyEvent = CreateEvent(NULL, FALSE, FALSE, EventName);
	if (m_NotifyEvent == NULL) {
		sprintf(err, "Create Event failed");
		return;
	}

	// 挂起的方式创建线程  
	m_hThread = CreateThread(NULL, 0, &InternalThread::entry, this, CREATE_SUSPENDED, NULL);
	if (NULL == m_hThread) {
		sprintf(err, "Create Thread failed");
		return;
	}

	// 唤醒线程  
	ResumeThread(m_hThread);
}

void InternalThread::GetThreadData()
{
	//m_NotifyEvent.Set();    // 通知线程该做事情了！  
}

DWORD CALLBACK InternalThread::entry(LPVOID lpParam)
{
	if (lpParam == NULL) {
		PrintLog("Thread lpParam is NULL");
		return -1;
	}

	InternalThread *lpThis = reinterpret_cast<InternalThread *>(lpParam);
	return lpThis->InternalThreadEntry();
}

void InternalThread::StopInternalThread() 
{
	// 通知线程处理data的数据  
	if (m_NotifyEvent != NULL) 
	{
		ResetEvent(m_NotifyEvent);
		m_NotifyEvent = NULL;
	}

	if (m_hThread != NULL)
	{
		// 预留100ms让线程处理完数据，100ms是个估值  
		WaitForSingleObject(m_hThread, 100);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}