#ifndef _VDEC_H_

#define _VDEC_H_

#ifndef _CMP_VDEC_TYPE_
#define _CMP_VDEC_TYPE_
enum CMP_VDEC_TYPE
{
	CMP_VDEC_NORMAL = 0,       // 正常解码
	CMP_VDEC_FISHEYE,          // 鱼眼校正
	CMP_VDEC_DXVA,             // 硬件加速解码
};
#endif

#include <Windows.h>

#define MPEG4_CODE_ID   13
#define H264_CODE_ID    28
#define H265_CODE_ID	174


typedef unsigned long VDEC_HADNDLE;

/*************************************************
  函数功能:     VDEC_Init
  函数描述:     初使化视频解码
  输入参数:     无
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
  备注：一个应用程序只需调用一次
*************************************************/
int VDEC_Init(void);

/*************************************************
  函数功能:     VDEC_Uninit
  函数描述:     反初使化视频解码
  输入参数:     无
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
*************************************************/
int VDEC_Uninit();

/*************************************************
  函数功能:     VDEC_CreateVideoDecCh
  函数描述:     创建视频解码通道
  输入参数:     iDecType解码类型（硬解码、软解码...）CMP_VDEC_TYPE
				hWnd 窗口句柄
  输出参数:     无
  返回值:       返回视频解码句柄,失败的话为空
  作者:         丁金奇
  日期:         2015-08-31
  修改:   
*************************************************/
VDEC_HADNDLE VDEC_CreateVideoDecCh(HWND hWnd,  int iWidth, int iHeight,int iDecType,int iCodecID);
/*************************************************
  函数功能:     VDEC_DestroyVideoDecCh
  函数描述:     销毁视频解码通道
  输入参数:     VHandle:视频解码通道句柄
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2015-08-31
  修改:   
*************************************************/
int VDEC_DestroyVideoDecCh(VDEC_HADNDLE VHandle);

/*************************************************
  函数功能:     VDEC_SendStream
  函数描述:     发送解码流到视频解码模块
  输入参数:     pData:视频流
                iLen: 视频流长度
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
*************************************************/
int VDEC_SendStream(VDEC_HADNDLE VHandle, void *pData, int iLen, unsigned int iPts);


/*************************************************
  函数功能:     VDEC_ChangeWindow
  函数描述:     改变显示窗口
  输入参数:     hWnd:新显示窗口
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
*************************************************/
int VDEC_ChangeWindow(VDEC_HADNDLE VHandle, HWND hWnd);

/*************************************************
  函数功能:     VDEC_StartPlayStream
  函数描述:     开始播放解码流
  输入参数:     无
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-30
  修改:   
*************************************************/
int VDEC_StartPlayStream(VDEC_HADNDLE VHandle);

/*************************************************
  函数功能:     VDEC_StopPlayStream
  函数描述:     停止播放解码流
  输入参数:     无
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-30
  修改:   
*************************************************/
int VDEC_StopPlayStream(VDEC_HADNDLE VHandle);

/*************************************************
  函数功能:     VDEC_PausePlayStream
  函数描述:     暂停播放解码流
  输入参数:     无
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-30
  修改:   
*************************************************/
int VDEC_PausePlayStream(VDEC_HADNDLE VHandle);



#endif