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
	// �����¼�
	static TCHAR EventName[] = TEXT("NDIobj"); //������
	m_NotifyEvent = CreateEvent(NULL, FALSE, FALSE, EventName);
	if (m_NotifyEvent == NULL) {
		sprintf(err, "Create Event failed");
		return;
	}

	// ����ķ�ʽ�����߳�  
	m_hThread = CreateThread(NULL, 0, &InternalThread::entry, this, CREATE_SUSPENDED, NULL);
	if (NULL == m_hThread) {
		sprintf(err, "Create Thread failed");
		return;
	}

	// �����߳�  
	ResumeThread(m_hThread);
}

void InternalThread::GetThreadData()
{
	//m_NotifyEvent.Set();    // ֪ͨ�̸߳��������ˣ�  
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
	// ֪ͨ�̴߳���data������  
	if (m_NotifyEvent != NULL) 
	{
		ResetEvent(m_NotifyEvent);
		m_NotifyEvent = NULL;
	}

	if (m_hThread != NULL)
	{
		// Ԥ��100ms���̴߳��������ݣ�100ms�Ǹ���ֵ  
		WaitForSingleObject(m_hThread, 100);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}