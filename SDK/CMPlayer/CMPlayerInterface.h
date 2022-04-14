#ifndef _CMPlayerInterface_H_
#define _CMPlayerInterface_H_

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
	CMP_STATE_IDLE = 0,         // ����
	CMP_STATE_PLAY,             // ����
	CMP_STATE_FAST_FORWARD,     // ���
	CMP_STATE_PAUSE,            // ��ͣ
	CMP_STATE_STOP,             // ֹͣ״̬
    CMP_STATE_ERROR,            //����
    CMP_STATE_SLOW_FORWARD      // ����
};

#ifndef _CMP_VDEC_TYPE_
#define _CMP_VDEC_TYPE_
enum CMP_VDEC_TYPE
{
    CMP_VDEC_NORMAL = 0,       // ��������
    CMP_VDEC_FISHEYE,          // ����У��
    CMP_VDEC_DXVA,             // Ӳ�����ٽ���
};
#endif

#ifdef  __cplusplus
extern "C"
{
#endif


/*************************************************
  ��������:     CMP_Init
  ��������:     ����ý����
  �������:     HWND hWnd��ý����ʾ����
                iDecType���������ͣ�enum CMP_VDEC_TYPE
  �������:     ��
  ����ֵ:       ý����
  ���ߣ�        dingjq
  ����:         2015-09-10 
*************************************************/
CMPPlayer_API CMPHandle CMP_Init(HWND hWnd, CMP_VDEC_TYPE eDecType);

/*************************************************
  ��������:     CMP_UnInit
  ��������:     ����ý����
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       0���ɹ��� -1:δ�ҵ���Ӧý����
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_UnInit(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_OpenMediaPreview
  ��������:     ��Ԥ��ý��
  �������:     hPlay��ý������ pcRtspUrl��rtsp��ַ�� iTcpFlag��CMP_TCP or CMP_UDP
  �������:     ��
  ����ֵ:       0���ɹ��� -1:δ�ҵ���Ӧý����
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_OpenMediaPreview(CMPHandle hPlay, const char *pcRtspUrl, int iTcpFlag);

/*************************************************
  ��������:     CMP_OpenMediaFile
  ��������:     �򿪵㲥ý��
  �������:     hPlay��ý������ pcRtspFile:rtsp�ļ���ַ �� iTcpFlag��CMP_TCP or CMP_UDP
  �������:     ��
  ����ֵ:       0���ɹ��� -1:δ�ҵ���Ӧý����
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_OpenMediaFile(CMPHandle hPlay, const char *pcRtspFile, int iTcpFlag);

/*************************************************
  ��������:     CMP_CloseMedia
  ��������:     �ر�ý��
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       0���ɹ��� -1:δ�ҵ���Ӧý����
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_CloseMedia(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_GetPlayStatus
  ��������:     ��ȡ����״̬
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       ����ý�岥��״̬
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API CMPPLAY_STATE CMP_GetPlayStatus(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_PlayMedia
  ��������:     ��ʼ����ý����
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_PlayMedia(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_PauseMedia
  ��������:     ��ͣ����ý����
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_PauseMedia(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_SetPosition
  ��������:     ���ò���λ�ã����룩
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_SetPosition(CMPHandle hPlay, __int64 nPosTime);

/*************************************************
  ��������:     CMP_SetPlaySpeed
  ��������:     ���ò����ٶ�
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_SetPlaySpeed(CMPHandle hPlay, double dSpeed);

/*************************************************
  ��������:     CMP_SetVolume
  ��������:     ���ò�������
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_SetVolume(CMPHandle hPlay, int nVolume);

/*************************************************
  ��������:     CMP_ChangeWnd
  ��������:     �ı䲥�Ŵ��� //ֻ��������
  �������:     hPlay��ý���� hWnd �ı���Ŀ�괰��
  �������:     ��
  ����ֵ:       
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_ChangeWnd(CMPHandle hPlay,HWND hWnd);

/*************************************************
  ��������:     CMP_CaptureBmp
  ��������:     ץ����Ƶ������ͼƬ(�����)
  �������:     pcFileName:ץ�ĺ�ͼƬ�洢λ��
  �������:     ��
  ����ֵ:       0:�ɹ�, ����:ʧ��
  ����:         ������
  ����:         2014-04-20
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_CaptureBmp(CMPHandle hPlay, char *pcFileName);


/*************************************************
  ��������:     CMP_GetPlayRange
  ��������:     ��ȡ����ʱ��
  �������:     hPlay��ý���� 
  �������:     ��
  ����ֵ:       �ļ�������ʱ��������Ϊ��λ
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_GetPlayRange(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_GetPlayTime
  ��������:     ��ȡ��ǰ����ʱ��
  �������:     hPlay��ý���� 
  �������:     ��
  ����ֵ:       �ļ�������ʱ��������Ϊ��λ
  ���ߣ�        dingjq
  ����:         2015-09-10
  �޸�:   
*************************************************/
CMPPlayer_API int CMP_GetPlayTime(CMPHandle hPlay);

CMPPlayer_API int CMP_DebugPrint(unsigned int uiDebugLevel,  const char *format, ...);

CMPPlayer_API int CMP_DebugInit(int iPort);

CMPPlayer_API int CMP_DebugUninit();

#ifdef  __cplusplus
}
#endif

#endif
