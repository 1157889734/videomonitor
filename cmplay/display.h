/******************************************************
  Copyright (C), 2014-2020, Superstring Tech. Co., Ltd.
  文件名:      display.h
  描述:        DirectDraw显示头文件
  作者:        丁金奇
  版本:        V1.0
  日期:        2014-04-20
  历史修改:     
********************************************************/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

typedef unsigned long DRAW_HANDLE;
/*************************************************
  函数功能:     DDRAW_Init
  函数描述:     初使化DirectDraw
  输入参数:     hWnd:   显示窗口
                iWidth: 原始图片宽度
                iHeight:原始图片高度
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
*************************************************/
DRAW_HANDLE DDRAW_Init(HWND hWnd, int iWidth, int iHeight);

/*************************************************
  函数功能:     DDRAW_Uninit
  函数描述:     反初使化DirectDraw
  输入参数:     无
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
*************************************************/
int DDRAW_Uninit(DRAW_HANDLE DRAWHandle);

/*************************************************
  函数功能:     DDRAW_DrawRGB
  函数描述:     DirectDraw显示RGB信号
  输入参数:     hWnd:      显示窗口
				pData:     RGB数据
				iWidth:    原始图片宽度
				iHeight:   原始图片高度
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
*************************************************/
int DDRAW_DrawRGB(DRAW_HANDLE DRAWHandle, HWND hWnd, char *pData, int iWidth, int iHeight);

/*************************************************
  函数功能:     DDRAW_DrawYUV
  函数描述:     DirectDraw显示YUV信号
  输入参数:     hWnd:      显示窗口
                pY:        Y分量
				pU:        U分量
				pV:        V分量
				iYStride:  Y分量大小
                iUVStride: UV分量大小
				iWidth:    原始图片宽度
				iHeight:   原始图片高度
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
*************************************************/
int DDRAW_DrawYUV(DRAW_HANDLE DRAWHandle, HWND hWnd, LPBYTE pY, LPBYTE pU, LPBYTE pV, 
						 INT32 iYStride, INT32 iUVStride, INT32 iWidth, INT32 iHeight);

/*************************************************
  函数功能:     DDRAW_ChangeWindows
  函数描述:     改变显示客口
  输入参数:     hWnd:      显示窗口
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-29
  修改:   
*************************************************/
int DDRAW_ChangeWindows(DRAW_HANDLE DRAWHandle, HWND hWnd);
#endif
