#include "stdafx.h"
#include "MBRClientHttpHandleW.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define RECVPACK_SIZE 10240

CMBRClientHttpHandleW::CMBRClientHttpHandleW(void)
{
	m_pHttpCore = GetHttpCore();
}

CMBRClientHttpHandleW::~CMBRClientHttpHandleW(void)
{
}

bool CMBRClientHttpHandleW::RequestFile(string strURL, string strSaveFile)
{
	string strHeaderSender;
	string strHeaderRecv;

	bool bRet = false;
	if(m_pHttpCore)
	{
		bRet = m_pHttpCore->httpGetFile(strURL.c_str(),strSaveFile.c_str());
	}
	return bRet;
}

double CMBRClientHttpHandleW::GetFileSize(string strURL,HWND hWnd)
{
    double dwFileSize;
    if(m_pHttpCore)
    {
        double *pSize = new double();
        m_pHttpCore->GetLength(strURL.c_str(),pSize,hWnd);
        dwFileSize = *pSize;
        delete pSize;
    }

    return dwFileSize;
}
