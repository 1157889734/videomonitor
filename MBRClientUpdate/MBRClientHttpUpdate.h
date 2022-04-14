#pragma once
#include <windows.h>
#include <string>
#include "MBRClientHttpHandleW.h"

using namespace std;

#define CFG_FILE		 "config.ini"
#define CFG_APPNAME  "ClientUpdate"
#define CFG_NEWFILE	 "file"


class CMBRClientHttpUpdate
{
public:
	CMBRClientHttpUpdate(void);
	~CMBRClientHttpUpdate(void);

	void VersionUpdate();
    double GetFileTotalSize(HWND hWnd);
	bool Download(string strDowloadAddr,string &strNewFile);

private:
    std::string m_strReURL;
	string GetAppPath();
	string GetUpdatePath();
    string GetCfgFile();
    CMBRClientHttpHandleW httpDownload;
    void WriteConfig(string strKeyName, string strValue);
};
