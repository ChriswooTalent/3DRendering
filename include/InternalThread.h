#ifndef _INTERNAL_THREAD_H_
#define _INTERNAL_THREAD_H_
#include <iostream>
#include <atlbase.h>
#include <atlsync.h>
#include <vector>
using namespace std;

class InternalThread
{
public:
	InternalThread();
	virtual ~InternalThread(){}

	void GetThreadData();

	void StartInternalThread(char *err);

	void StopInternalThread();

	bool is_started() const;

	static DWORD CALLBACK entry(LPVOID);   // 线程函数，必须是静态函数 

protected:
	virtual DWORD InternalThreadEntry() { return 1; }

	bool must_stop();
	HANDLE  m_NotifyEvent;
private:
	HANDLE m_hThread;
};
#endif // !_INTERNAL_THREAD_H_
