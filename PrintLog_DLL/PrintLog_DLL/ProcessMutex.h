#ifndef _PROCESSMUTEX_H_
#define _PROCESSMUTEX_H_

class  CProcessMutex
{
public:
	CProcessMutex(const char* name);
	~CProcessMutex();

	bool Lock();
	bool UnLock();
private:
#ifdef WIN32
	void * m_pMutex;
#endif

#ifdef linux
	set_t * m_pSem;
#endif

	char m_cMutexName[30];
protected:

};
#endif //_PROCESSMUTEX_H_


