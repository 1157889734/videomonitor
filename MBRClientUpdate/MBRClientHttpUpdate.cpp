#include "stdafx.h"
#include "AEyeDBOperator.h"
#include "MBRClientHttpUpdate.h"
#include "paramW.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CMBRClientHttpUpdate::CMBRClientHttpUpdate(void)
{
}

CMBRClientHttpUpdate::~CMBRClientHttpUpdate(void)
{
}

string CMBRClientHttpUpdate::GetAppPath()
{   
	string strFilePath;
	char cFilePath[MAX_PATH] = {0};
	GetModuleFileNameA(NULL,cFilePath,MAX_PATH);
	strFilePath = cFilePath;

	int iPosIndex;
	iPosIndex = strFilePath.rfind('\\');
	strFilePath = strFilePath.substr(0,iPosIndex+1);
	return strFilePath;
}

string CMBRClientHttpUpdate::GetUpdatePath()
{   
	string UpdatePath;
	UpdatePath = GetAppPath()+"Update\\";
	return UpdatePath;
}

string CMBRClientHttpUpdate::GetCfgFile()
{
    string strCfgFile = GetUpdatePath()+ CFG_FILE;
    return strCfgFile;
}

void CMBRClientHttpUpdate::WriteConfig(string strKeyName, string strValue)
{
    string strCfgFile = GetCfgFile();
    WritePrivateProfileStringA(CFG_APPNAME,strKeyName.c_str(),strValue.c_str(),strCfgFile.c_str());
}

void CMBRClientHttpUpdate::VersionUpdate()
{
    //string strRemoteURL = "http://192.168.10.200:7011/updates/MBRClientIn.msi";
    string strNewFile;
    if(!Download(m_strReURL,strNewFile))
    {
        
    }
    WriteConfig(CFG_NEWFILE,strNewFile);
    AEYEDBOPERATOR->setVersionFlag(1);
}

bool CMBRClientHttpUpdate::Download(string strDowloadAddr,string &strNewFile)
{
	string strFile = strDowloadAddr.substr(strDowloadAddr.rfind("/")+1
		,strDowloadAddr.length()-strDowloadAddr.rfind("/")-1);
	string strNewVersion = GetUpdatePath()+strFile;

	bool bRet = httpDownload.RequestFile(strDowloadAddr,strNewVersion);

	if(bRet)
	{
		strNewFile = strNewVersion;
	}
	else
	{
	}
	return bRet;
}

 double CMBRClientHttpUpdate::GetFileTotalSize(HWND hWnd)
 {
      //string strRemoteURL = "http://192.168.10.200:7011/updates/MBRClientIn.msi";
      AEYEDBOPERATOR->getUpdateData(m_strReURL);
      return httpDownload.GetFileSize(m_strReURL,hWnd);
 }