#include "ProcessMutex.h"
#include <Windows.h>

#ifdef WIN32
CProcessMutex::CProcessMutex(const char * name)
{
	memset(m_cMutexName, 0, sizeof(m_cMutexName));
	//	int _min = strlen(name)>(sizeof(m_cMutexName)-1)?(sizeof(m_cMutexName)-1):strlen(name);
	strcpy_s(m_cMutexName, name);//, _min);
	m_pMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, name);
	if(m_pMutex == NULL)
		m_pMutex = CreateMutexA(NULL, false, m_cMutexName);
}

CProcessMutex::~CProcessMutex()
{
	CloseHandle(m_pMutex);
}

bool CProcessMutex::Lock()
{
	if(m_pMutex == NULL)
		return false;

	DWORD nRet = WaitForSingleObject(m_pMutex, INFINITE);
	if(nRet != WAIT_OBJECT_0){
		return false;
	}

	return true;
}

bool CProcessMutex::UnLock()
{
	if(ReleaseMutex(m_pMutex))
		return true;
	else
		return false;
}
#endif//WIN32

#ifdef linux
CProcessMutex CProcessMutex(const char* name)
{
	memset(m_cMutexName, 0, sizeof(m_cMutexName));
	int _min = strlen(name)>(sizeof(m_cMutexName)-1)?(sizeof(m_cMutexName)-1):strlen(name);
	strcpy(m_cMutexName, name, _min);
	m_pMutex = sem_open(name, O_CREAT, 0644, 1);
}

CProcessMutex ~CProcessMutex()
{
	int ret = sem_close(m_pSem);
	if(ret != 0){
		printf("sem_close error %d\n", ret);
	}
	sem_unlink(m_cMutexName);
}

bool CProcessMutex::Lock()
{
	int ret = sem_wait(m_pSem);
	if(ret != 0){
		return false;
	}
	return true;
}

bool CProcessMutex::UnLock()
{
	int ret = sem_post(m_pSem);
	if(ret != 0){
		return false;
	}
	return true;
}
#endif //linux