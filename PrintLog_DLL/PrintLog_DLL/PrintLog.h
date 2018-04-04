#ifndef _PRINTLOG_H_
#define _PRINTLOG_H_

#include <iostream>
#include <fstream>

#define CarbonmedLogPath "D:\\CarbonMed\\System\\Log\\"

using namespace std;

class CPrintLog
{
public:
	CPrintLog(){}
	virtual ~CPrintLog(){}

	static void OpenLogFile();
	static void CloseLogFile();
	static void PrintLog(const char * sysmessage);

protected:

private:
	static ofstream SysLogFile;

	static char logfilepath[64];
	static char LogfileMessages[1024];

	static int GenLogFile();
	static int SearchLogFile(const char* _LogFilePath);
};
#endif //_PRINTLOG_H_