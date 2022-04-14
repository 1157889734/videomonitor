#pragma once

#include <string>
using namespace std;

#include "../include/IHttpCore.h"
#pragma comment( lib , "../lib/HttpCore.lib")

class CMBRClientHttpHandleW
{
public:
	CMBRClientHttpHandleW(void);
	~CMBRClientHttpHandleW(void);
    double GetFileSize(string strURL,HWND hWnd);
	bool RequestFile(string strURL, string strSaveFile);

private:
	IHttpCore* m_pHttpCore;
};
