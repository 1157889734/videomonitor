#ifndef _CMPlayerInterface_H_
#define _CMPlayerInterface_H_
#include <Windows.h>
#pragma once
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")

#ifndef  CMPPlayer_API
#ifdef   CMPLAY_EXPORTS
#define  CMPPlayer_API __declspec(dllexport)
#else
#define  CMPPlayer_API __declspec(dllimport)
#pragma comment(lib,"CmPlay.lib") 
#endif
#endif

typedef void* CMPHandle;

enum STREAM_TYPE
{
    CMP_TCP = 0,
	CMP_UDP
};

enum CMPPLAY_STATE
{
	CMP_STATE_IDLE = 0,         // 空闲
	CMP_STATE_PLAY,             // 播放
	CMP_STATE_FAST_FORWARD,     // 快进
	CMP_STATE_PAUSE,            // 暂停
	CMP_STATE_STOP,             // 停止状态
    CMP_STATE_ERROR,            //错误
    CMP_STATE_SLOW_FORWARD      // 慢进
};

#ifndef _CMP_VDEC_TYPE_
#define _CMP_VDEC_TYPE_
enum CMP_VDEC_TYPE
{
    CMP_VDEC_NORMAL = 0,       // 正常解码
    CMP_VDEC_FISHEYE,          // 鱼眼校正
    CMP_VDEC_DXVA,             // 硬件加速解码
};
#endif

#ifdef  __cplusplus
extern "C"
{
#endif


/*************************************************
  函数功能:     CMP_Init
  函数描述:     创建媒体句柄
  输入参数:     HWND hWnd：媒体显示窗口
                iDecType：解码类型，enum CMP_VDEC_TYPE
  输出参数:     无
  返回值:       媒体句柄
  作者：        dingjq
  日期:         2015-09-10 
*************************************************/
CMPPlayer_API CMPHandle CMP_Init(HWND hWnd, CMP_VDEC_TYPE eDecType);

/*************************************************
  函数功能:     CMP_UnInit
  函数描述:     销毁媒体句柄
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_UnInit(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_OpenMediaPreview
  函数描述:     打开预览媒体
  输入参数:     hPlay：媒体句柄， pcRtspUrl：rtsp地址， iTcpFlag：CMP_TCP or CMP_UDP
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_OpenMediaPreview(CMPHandle hPlay, const char *pcRtspUrl, int iTcpFlag);

/*************************************************
  函数功能:     CMP_OpenMediaFile
  函数描述:     打开点播媒体
  输入参数:     hPlay：媒体句柄， pcRtspFile:rtsp文件地址 ， iTcpFlag：CMP_TCP or CMP_UDP
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_OpenMediaFile(CMPHandle hPlay, const char *pcRtspFile, int iTcpFlag);

/*************************************************
  函数功能:     CMP_CloseMedia
  函数描述:     关闭媒体
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_CloseMedia(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_GetPlayStatus
  函数描述:     获取播放状态
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       返回媒体播放状态
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API CMPPLAY_STATE CMP_GetPlayStatus(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_PlayMedia
  函数描述:     开始播放媒体流
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_PlayMedia(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_PauseMedia
  函数描述:     暂停播放媒体流
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_PauseMedia(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_SetPosition
  函数描述:     设置播放位置（毫秒）
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_SetPosition(CMPHandle hPlay, __int64 nPosTime);

/*************************************************
  函数功能:     CMP_SetPlaySpeed
  函数描述:     设置播放速度
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_SetPlaySpeed(CMPHandle hPlay, double dSpeed);

/*************************************************
  函数功能:     CMP_SetVolume
  函数描述:     设置播放音量
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_SetVolume(CMPHandle hPlay, int nVolume);

/*************************************************
  函数功能:     CMP_ChangeWnd
  函数描述:     改变播放窗口 //只针对软解码
  输入参数:     hPlay：媒体句柄 hWnd 改变后的目标窗口
  输出参数:     无
  返回值:       
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_ChangeWnd(CMPHandle hPlay,HWND hWnd);

/*************************************************
  函数功能:     CMP_CaptureBmp
  函数描述:     抓拍视频解码后的图片(软解码)
  输入参数:     pcFileName:抓拍后图片存储位置
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
*************************************************/
CMPPlayer_API int CMP_CaptureBmp(CMPHandle hPlay, char *pcFileName);


/*************************************************
  函数功能:     CMP_GetPlayRange
  函数描述:     获取播放时长
  输入参数:     hPlay：媒体句柄 
  输出参数:     无
  返回值:       文件播放总时长，以秒为单位
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_GetPlayRange(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_GetPlayTime
  函数描述:     获取当前播放时间
  输入参数:     hPlay：媒体句柄 
  输出参数:     无
  返回值:       文件播放总时长，以秒为单位
  作者：        dingjq
  日期:         2015-09-10
  修改:   
*************************************************/
CMPPlayer_API int CMP_GetPlayTime(CMPHandle hPlay);

CMPPlayer_API int CMP_DebugPrint(unsigned int uiDebugLevel,  const char *format, ...);

CMPPlayer_API int CMP_DebugInit(int iPort);

CMPPlayer_API int CMP_DebugUninit();

#ifdef  __cplusplus
}
#endif

#endif
