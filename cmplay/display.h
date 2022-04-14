/******************************************************
  Copyright (C), 2014-2020, Superstring Tech. Co., Ltd.
  �ļ���:      display.h
  ����:        DirectDraw��ʾͷ�ļ�
  ����:        ������
  �汾:        V1.0
  ����:        2014-04-20
  ��ʷ�޸�:     
********************************************************/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

typedef unsigned long DRAW_HANDLE;
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
DRAW_HANDLE DDRAW_Init(HWND hWnd, int iWidth, int iHeight);

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
int DDRAW_Uninit(DRAW_HANDLE DRAWHandle);

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
int DDRAW_DrawRGB(DRAW_HANDLE DRAWHandle, HWND hWnd, char *pData, int iWidth, int iHeight);

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
						 INT32 iYStride, INT32 iUVStride, INT32 iWidth, INT32 iHeight);

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
int DDRAW_ChangeWindows(DRAW_HANDLE DRAWHandle, HWND hWnd);
#endif
