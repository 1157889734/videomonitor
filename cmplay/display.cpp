/******************************************************
  Copyright (C), 2014-2020, Superstring Tech. Co., Ltd.
  �ļ���:      display.cpp
  ����:        DirectDraw��ʾԴ�ļ�
  ����:        ������
  �汾:        V1.0
  ����:        2014-04-20
  ��ʷ�޸�:     
********************************************************/

#include <windows.h>
#include <ddraw.h> 
#include "display.h"
#include "debug.h"
//#include "ddraw.h"
//#pragma comment(lib,"ddraw.lib") 
//#pragma comment(lib, "dxguid.lib") 

/*static LPDIRECTDRAW7            g_lpDD;	         // DirectDraw ����ָ��
static LPDIRECTDRAWSURFACE7     g_lpDDSPrimary;	 // DirectDraw ������ָ��
static LPDIRECTDRAWSURFACE7     g_lpDDSOffScr;	 // DirectDraw ��������ָ��
static DDSURFACEDESC2	        g_ddsd;	         // DirectDraw ��������*/

typedef struct _T_DISPLAY_INFO
{
	LPDIRECTDRAW7            lpDD;	         // DirectDraw ����ָ��
	LPDIRECTDRAWSURFACE7     lpDDSPrimary;	 // DirectDraw ������ָ��
	LPDIRECTDRAWSURFACE7     lpDDSOffScr;	 // DirectDraw ��������ָ��
	DDSURFACEDESC2	         ddsd;	         // DirectDraw ��������
}T_DISPLAY_INFO, *PT_DISPLAY_INFO;





/*************************************************
  ��������:     DDRAW_Init
  ��������:     ��ʹ��DirectDraw
  �������:     hWnd:   ��ʾ����
                iWidth: ԭʼͼƬ���
                iHeight:ԭʼͼƬ�߶�
  �������:     ��
  ����ֵ:       0:�ɹ�, ����:ʧ��
  ����:         ������
  ����:         2014-04-20
  �޸�:   
*************************************************/
DRAW_HANDLE DDRAW_Init(HWND hWnd, int iWidth, int iHeight)
{
	PT_DISPLAY_INFO ptDisplayInfo = NULL;

	ptDisplayInfo = new T_DISPLAY_INFO;
    ptDisplayInfo->lpDD = NULL;
    ptDisplayInfo->lpDDSPrimary = NULL;
    ptDisplayInfo->lpDDSOffScr = NULL;
    DebugPrint(DEBUG_DISPLAY_7,"[%s %d] enter", __FUNCTION__, __LINE__);
	if (NULL == ptDisplayInfo)
	{
        DebugPrint(DEBUG_DISPLAY_7,"[%s %d] Error", __FUNCTION__, __LINE__);
		return 0;
	}
	//memset(ptDisplayInfo, 0, sizeof(T_DISPLAY_INFO));
	// ����DirectCraw����
	if (DirectDrawCreateEx(NULL, (VOID**)&ptDisplayInfo->lpDD, IID_IDirectDraw7, NULL) != DD_OK) 
	//if (DirectDrawCreate(NULL, &g_lpDD, NULL) != DD_OK) 
	{
		delete ptDisplayInfo;
		ptDisplayInfo = NULL;
        DebugPrint(DEBUG_DISPLAY_7,"[%s %d]CreateDirectDrawFail",__FUNCTION__, __LINE__);
		return 0;
	}

	// ����Э����
	if (ptDisplayInfo->lpDD->SetCooperativeLevel(hWnd,DDSCL_NORMAL | DDSCL_NOWINDOWCHANGES) != DD_OK)
	{
		delete ptDisplayInfo;
		ptDisplayInfo = NULL;
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] SetCooperativeLevel Error",__FUNCTION__, __LINE__);
		return 0;
	}

	// ����������
	ZeroMemory(&ptDisplayInfo->ddsd, sizeof(ptDisplayInfo->ddsd));
	ptDisplayInfo->ddsd.dwSize = sizeof(ptDisplayInfo->ddsd);
	ptDisplayInfo->ddsd.dwFlags = DDSD_CAPS;
	ptDisplayInfo->ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (ptDisplayInfo->lpDD->CreateSurface(&ptDisplayInfo->ddsd, &ptDisplayInfo->lpDDSPrimary, NULL) != DD_OK)
	{
		delete ptDisplayInfo;
		ptDisplayInfo = NULL;
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] CreateSurface Error",__FUNCTION__, __LINE__);
		return 0;
	}

	LPDIRECTDRAWCLIPPER pcClipper;
	if(FAILED(ptDisplayInfo->lpDD->CreateClipper(0, &pcClipper, NULL)))
	{
		DDRAW_Uninit((DRAW_HANDLE)ptDisplayInfo);
		ptDisplayInfo = NULL;
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] CreateClipper Error",__FUNCTION__, __LINE__);
		return 0;
	}
	if(FAILED(pcClipper->SetHWnd(0, hWnd)))
	{
		delete ptDisplayInfo;
		ptDisplayInfo = NULL;
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] SetHWnd Error",__FUNCTION__, __LINE__);
		return 0;
	}
	if(FAILED(ptDisplayInfo->lpDDSPrimary->SetClipper( pcClipper )))
	{
		free(ptDisplayInfo);
		ptDisplayInfo = NULL;
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] SetClipper Error",__FUNCTION__, __LINE__);
		return 0;
	}
	pcClipper->Release();

	// ���������������
	ZeroMemory(&ptDisplayInfo->ddsd, sizeof(ptDisplayInfo->ddsd));
	ptDisplayInfo->ddsd.dwSize = sizeof(ptDisplayInfo->ddsd);
	ptDisplayInfo->ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN/*|DDSCAPS_OVERLAY*//*| DDSCAPS_VIDEOMEMORY*/ /*| DDSCAPS_VIDEOMEMORY*//*DDSCAPS_OFFSCREENPLAIN*/ /*| DDSCAPS_VIDEOMEMORY|DDSCAPS_LOCALVIDMEM*/;//| DDSCAPS_VIDEOMEMORY;//|DDSCAPS_NONLOCALVIDMEM; //DDSCAPS_OVERLAY DDSCAPS_OFFSCREENPLAIN;
	ptDisplayInfo->ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ptDisplayInfo->ddsd.dwWidth = iWidth;
	ptDisplayInfo->ddsd.dwHeight = iHeight;
	ptDisplayInfo->ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ptDisplayInfo->ddsd.ddpfPixelFormat.dwFlags  = DDPF_FOURCC | DDPF_YUV ;
	ptDisplayInfo->ddsd.ddpfPixelFormat.dwFourCC = MAKEFOURCC('Y','V','1','2');
	ptDisplayInfo->ddsd.ddpfPixelFormat.dwYUVBitCount = 8;
	DWORD dw = ptDisplayInfo->lpDD->CreateSurface(&ptDisplayInfo->ddsd, &ptDisplayInfo->lpDDSOffScr, NULL);
	if (dw != DD_OK)
	{
		//char chError[1024];
		DWORD dwErrorLen = 1024;
		//DXGetErrorString(dw, dwErrorLen, &chError[0]);
		//DirectDrawCreateEx()
		//DXGetErrorString
        
		if (FAILED(dw))
		{
            DebugPrint( DEBUG_DISPLAY_7,"[%s %d] CreateSurface2 Error %lx",__FUNCTION__, __LINE__, dw);
			return 0;
		}
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] CreateSurface3 Error",__FUNCTION__, __LINE__);
		return 0;
	}

	return (DRAW_HANDLE)ptDisplayInfo;
}

/*************************************************
  ��������:     DDRAW_Uninit
  ��������:     ����ʹ��DirectDraw
  �������:     ��
  �������:     ��
  ����ֵ:       0:�ɹ�, ����:ʧ��
  ����:         ������
  ����:         2014-04-20
  �޸�:   
*************************************************/
int DDRAW_Uninit(DRAW_HANDLE DRAWHandle)
{
	PT_DISPLAY_INFO ptDisplayInfo = (PT_DISPLAY_INFO)DRAWHandle;

	if (NULL == ptDisplayInfo)
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] Error",__FUNCTION__, __LINE__);
		return -1;
	}

	// �ͷ�DirectDraw����
	if(ptDisplayInfo->lpDD != NULL)
	{
		if(ptDisplayInfo->lpDDSPrimary != NULL)
		{
			ptDisplayInfo->lpDDSPrimary->Release();
			ptDisplayInfo->lpDDSPrimary = NULL;
		}
		if(ptDisplayInfo->lpDDSOffScr != NULL)
		{
			ptDisplayInfo->lpDDSOffScr->Release();
			ptDisplayInfo->lpDDSOffScr = NULL;
		}
		ptDisplayInfo->lpDD->Release();
		ptDisplayInfo->lpDD = NULL;
	}

	delete ptDisplayInfo;
    ptDisplayInfo = NULL;

	return 0;
}

/*************************************************
  ��������:     DDRAW_DrawRGB
  ��������:     DirectDraw��ʾRGB�ź�
  �������:     hWnd:      ��ʾ����
				pData:     RGB����
				iWidth:    ԭʼͼƬ���
				iHeight:   ԭʼͼƬ�߶�
  �������:     ��
  ����ֵ:       0:�ɹ�, ����:ʧ��
  ����:         ������
  ����:         2014-04-20
  �޸�:   
*************************************************/
int DDRAW_DrawRGB(DRAW_HANDLE DRAWHandle, HWND hWnd, char *pData, int iWidth, int iHeight)
{
	PT_DISPLAY_INFO ptDisplayInfo = (PT_DISPLAY_INFO)DRAWHandle;
	HRESULT	 ddRval;	 // DirectDraw ��������ֵ
	RECT	 tDstRect;	 // Ŀ������
	RECT	 tSrcRect;	 // Դ����
	POINT p = {0, 0};
	LPBYTE lpSurf = NULL;

	if (NULL == ptDisplayInfo)
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] Error",__FUNCTION__, __LINE__);
		return -1;
	}

	lpSurf = (LPBYTE)ptDisplayInfo->ddsd.lpSurface;

	ddRval = ptDisplayInfo->lpDDSOffScr->Lock(NULL,&ptDisplayInfo->ddsd,DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
	while(ddRval == DDERR_WASSTILLDRAWING)
	{
		//g_lpDDSOffScr->Restore();
	}
	if(ddRval != DD_OK)
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] Lock Error",__FUNCTION__, __LINE__);
		return -1;
	}


	// �����������
	if(lpSurf)
	{
		int lineBytes = iWidth*3;
		memset(ptDisplayInfo->ddsd.lpSurface, 0, iHeight*iWidth*4);
		for(int i = 0; i< iHeight;i++)
		{
			for(int j = 0; j<iWidth; j++)
			{
				memcpy((char*)ptDisplayInfo->ddsd.lpSurface + ptDisplayInfo->ddsd.lPitch*i + j*4, pData +  i*lineBytes + j*3, 3);
			}
		}
	}

	ptDisplayInfo->lpDDSOffScr->Unlock(NULL);

	GetClientRect(hWnd, &tDstRect);
	ClientToScreen(hWnd, &p);
	OffsetRect(&tDstRect, p.x, p.y);

	tSrcRect.left = 0;
	tSrcRect.top = 0;
	tSrcRect.right = iWidth;
	tSrcRect.bottom = iHeight;

	ddRval = ptDisplayInfo->lpDDSPrimary->Blt(&tDstRect, ptDisplayInfo->lpDDSOffScr, &tSrcRect, DDBLT_ASYNC | DDBLT_WAIT, NULL);
	while(ddRval == DDERR_WASSTILLDRAWING);
	if(ddRval != DD_OK)
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] Blt Error",__FUNCTION__, __LINE__);
		return -1;
	}

	return 0;
}

/*************************************************
  ��������:     DDRAW_DrawYUV
  ��������:     DirectDraw��ʾYUV�ź�
  �������:     hWnd:      ��ʾ����
                pY:        Y����
				pU:        U����
				pV:        V����
				iYStride:  Y������С
                iUVStride: UV������С
				iWidth:    ԭʼͼƬ���
				iHeight:   ԭʼͼƬ�߶�
  �������:     ��
  ����ֵ:       0:�ɹ�, ����:ʧ��
  ����:         ������
  ����:         2014-04-20
  �޸�:   
*************************************************/
int DDRAW_DrawYUV(DRAW_HANDLE DRAWHandle, HWND hWnd, LPBYTE pY, LPBYTE pU, LPBYTE pV, 
						 INT32 iYStride, INT32 iUVStride, INT32 iWidth, INT32 iHeight)
{
	HRESULT	 ddRval;	 // DirectDraw ��������ֵ
	RECT	 tDstRect;	 // Ŀ������
	RECT	 tSrcRect;	 // Դ����
	POINT p = {0, 0};
	LPBYTE lpSurf = NULL;
	PT_DISPLAY_INFO ptDisplayInfo = (PT_DISPLAY_INFO)DRAWHandle;

	if (NULL == ptDisplayInfo)
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] Error",__FUNCTION__, __LINE__);
		return -1;
	}

	ddRval = ptDisplayInfo->lpDDSOffScr->Lock(NULL,&ptDisplayInfo->ddsd,DDLOCK_WAIT | DDLOCK_WRITEONLY,NULL);
	while(ddRval == DDERR_WASSTILLDRAWING)

	{
		
	}

	if (ddRval == DDERR_SURFACELOST)
	{
		ddRval = ptDisplayInfo->lpDDSOffScr->Restore();
		ddRval = ptDisplayInfo->lpDDSOffScr->Lock(NULL,&ptDisplayInfo->ddsd,DDLOCK_WAIT | DDLOCK_WRITEONLY,NULL);
	}
	if(ddRval != DD_OK)
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] Lock Error",__FUNCTION__, __LINE__);
		return -1;
	}

	lpSurf = (LPBYTE)ptDisplayInfo->ddsd.lpSurface;
	
	// �����������
	if(lpSurf)
	{
		int i = 0;

		// fill Y data
		for(i=0; i<iHeight; i++)
		{
			memcpy(lpSurf, pY, iWidth);
			pY += iYStride;
			lpSurf += ptDisplayInfo->ddsd.lPitch;
		}

		// fill V data
		for(i=0; i<iHeight/2; i++)
		{
			memcpy(lpSurf, pV, iWidth / 2);
			pV += iUVStride;
			lpSurf += ptDisplayInfo->ddsd.lPitch / 2;
		}

		// fill U data
		for(i=0; i<iHeight/2; i++)
		{
			memcpy(lpSurf, pU, iWidth / 2);
			pU += iUVStride;
			lpSurf += ptDisplayInfo->ddsd.lPitch / 2;
		}

	}

	ptDisplayInfo->lpDDSOffScr->Unlock(NULL);

	GetClientRect(hWnd, &tDstRect);
	ClientToScreen(hWnd, &p);
	OffsetRect(&tDstRect, p.x, p.y);

	tSrcRect.left = 0;
	tSrcRect.top = 0;
	tSrcRect.right = iWidth;
	tSrcRect.bottom = iHeight;

	ddRval = ptDisplayInfo->lpDDSPrimary->Blt(&tDstRect, ptDisplayInfo->lpDDSOffScr, &tSrcRect, DDBLT_ASYNC | DDBLT_WAIT, NULL);
	while(ddRval == DDERR_WASSTILLDRAWING);

	if (ddRval == DDERR_SURFACELOST)
	{
		ddRval = ptDisplayInfo->lpDDSPrimary->Restore();
	}

	if(ddRval != DD_OK)
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] Restore Error",__FUNCTION__, __LINE__);
		return -1;
	}
	return 0;
}

/*************************************************
  ��������:     DDRAW_ChangeWindows
  ��������:     �ı���ʾ�Ϳ�
  �������:     hWnd:      ��ʾ����
  �������:     ��
  ����ֵ:       0:�ɹ�, ����:ʧ��
  ����:         ������
  ����:         2014-04-29
  �޸�:   
*************************************************/
int DDRAW_ChangeWindows(DRAW_HANDLE DRAWHandle, HWND hWnd)
{
	LPDIRECTDRAWCLIPPER pcClipper;
	PT_DISPLAY_INFO ptDisplayInfo = (PT_DISPLAY_INFO)DRAWHandle;

	if (NULL == ptDisplayInfo)
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] Error",__FUNCTION__, __LINE__);
		return -1;
	}
	if(FAILED(ptDisplayInfo->lpDD->CreateClipper(0, &pcClipper, NULL)))
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] CreateClipper Error",__FUNCTION__, __LINE__);
		return -2;
	}
	if(FAILED(pcClipper->SetHWnd(0, hWnd)))
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] SetHWnd Error",__FUNCTION__, __LINE__);
		return -3;
	}
	if(FAILED(ptDisplayInfo->lpDDSPrimary->SetClipper( pcClipper )))
	{
        DebugPrint( DEBUG_DISPLAY_7,"[%s %d] SetClipper Error",__FUNCTION__, __LINE__);
		return -4;
	}
	pcClipper->Release();

	return 0;
}
