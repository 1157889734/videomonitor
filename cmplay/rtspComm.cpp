#include "stdafx.h"
#include "rtspComm.h"


void RTSP_Close(int iSocket)
{
    if (iSocket > 0)
    {
    #ifdef WIN
        closesocket(iSocket);
    
    #else
        close(iSocket);
    
    #endif
    }
}

int RTSP_ThreadJoin(THREAD_HANDLE threadHandle)
{
#ifdef WIN
	if (WAIT_OBJECT_0 == WaitForSingleObject(threadHandle, 2000))
	{
		CloseHandle(threadHandle);
	} 
	else
	{
		DWORD nExitCode;
		GetExitCodeThread(threadHandle, &nExitCode);
		if (STILL_ACTIVE == nExitCode)
		{
			TerminateThread(threadHandle, nExitCode);
			CloseHandle(threadHandle);
		}						
	}
#else
    pthread_join(threadHandle);
#endif

    return 0;
}


void RTSP_MSleep(int iMsec)
{
#ifdef WIN
    Sleep(iMsec);
#else
    usleep(iMsec * 1000);
#endif
}
