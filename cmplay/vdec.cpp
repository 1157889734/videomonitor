#include "vdec.h"
#include <windows.h>
#include "mutex.h"
#include "display.h"
#include "FisheyeCorrect.h"
#include "debug.h"
#include "ffmpeg_dxva2.h"
//#include "remuxing.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
}


#define STOP_STREAM_PLAY    0
#define START_STREAM_PLAY   1
#define PAUSE_STREAM_PLAY   2

typedef struct _T_DATA_PACKET
{
	char *pcData;
	int iLen;
	unsigned int iPts;
}T_DATA_PACKET, *PT_DATA_PACKET;

typedef struct _T_DATA_PACKET_LIST
{
	T_DATA_PACKET tPkt;
	struct _T_DATA_PACKET_LIST *next;
}T_DATA_PACKET_LIST, *PT_DATA_PACKET_LIST;

typedef struct _T_PACKET_QUEUE {
	T_DATA_PACKET_LIST *first_pkt, *last_pkt;
	int nb_packets;
	CMutexLock *mutex;
}T_PACKET_QUEUE;

typedef struct _T_CODEC_INFO 
{
	AVCodecContext *pAVCodecContext;			// ffmpeg定义的结构
	AVCodec	*pAVCodec;							// ffmpeg定义的结构    
	AVFrame	*pAVFrame;							// ffmpeg定义的结构
}T_CODEC_INFO;

typedef struct _T_FISHEYE_YUV_PACKET
{
	unsigned char *pYData;
	unsigned char *pUData;
	unsigned char *pVData;
	unsigned int iWidth;
	unsigned int iHeight;
	unsigned int iYstride;
}T_FISHEYE_YUV_PACKET, *PT_FISHEYE_YUV_PACKET;

typedef struct _T_FISHEYE_YUV_QUEUE {
	T_FISHEYE_YUV_PACKET tFisheyeYuv;
	BOOL bReadFlag;
	CMutexLock cLock;
}T_FISHEYE_YUV_QUEUE;


typedef struct _T_VIDEO_DEC_INFO
{
	T_CODEC_INFO tCodecInfo;
	HWND hWnd;
	T_PACKET_QUEUE tPacketQueue;
	T_FISHEYE_YUV_QUEUE tFisheyeYuv;
	HANDLE hVideoPlayThread;
	CMutexLock cMute;
	unsigned char *pcCaptureYuv;
	int iInitDDrawFlag;
	int iStartPlayFlag;
	int iVideoExitFlag;
	int iVideoWidth;
	int iVideoHeight;
	DRAW_HANDLE DRAWHandle;
	int nCapturebmp;
	char *pcPictFileName;
	int iDecType;  //CMP_VDEC_TYPE
	Fe_Handle hFisheyehandle; //鱼眼句柄
	HANDLE    hFishThreadHand;
	Fe_Int8 *pCorrect_data ;

}T_VIDEO_DEC_INFO, *PT_VIDEO_DEC_INFO;



static void packet_queue_init(T_PACKET_QUEUE *q)
{
	memset(q, 0, sizeof(T_PACKET_QUEUE));
	q->mutex = new CMutexLock();
}

static void packet_queue_flush(T_PACKET_QUEUE *q)
{
	PT_DATA_PACKET_LIST pkt, pkt1;
	if(NULL==q)
	{
		return;
	}
	q->mutex->Lock();
	for(pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
		pkt1 = pkt->next;
		if((NULL!=pkt) && (NULL!=pkt->tPkt.pcData))
		{
			free(pkt->tPkt.pcData);
			free(pkt);
		}
	}
	q->last_pkt = NULL;
	q->first_pkt = NULL;
	q->nb_packets = 0;
	q->mutex->Unlock();
}

static void packet_queue_uninit(T_PACKET_QUEUE *q)
{
	if(q)
	{
		packet_queue_flush(q);
		delete q->mutex;
		q->mutex = NULL;
		q = NULL;
	}
}

static int packet_queue_put(T_PACKET_QUEUE *q, PT_DATA_PACKET pkt)
{
	PT_DATA_PACKET_LIST pktl = NULL;

	if (NULL == pkt)
		return -1;
	if (NULL == q->mutex)
	{
		free(pkt);
		return -1;
	}

	pktl = (PT_DATA_PACKET_LIST)malloc(sizeof(T_DATA_PACKET_LIST));
	if (!pktl)
		return -1;
	pktl->tPkt = *pkt;
	pktl->next = NULL;

	while(1)
	{
		q->mutex->Lock();
		if (q->nb_packets > 100)
		{
			q->mutex->Unlock();
			Sleep(1);
		}
		else
		{
			q->mutex->Unlock();
			break;
		}
	}

	q->mutex->Lock();
	if (!q->last_pkt)

		q->first_pkt = pktl;
	else
		q->last_pkt->next = pktl;
	q->last_pkt = pktl;
	q->nb_packets++;

	q->mutex->Unlock();
	return 0;
}

static int packet_queue_get(T_PACKET_QUEUE *q, PT_DATA_PACKET pkt, int block)
{
	PT_DATA_PACKET_LIST pktl;
	int ret =0;
	if(NULL==q||NULL==q->mutex)
	{
		return ret;
	}
	q->mutex->Lock();
	for(;;) 
	{
		pktl = q->first_pkt;
		if (pktl) {
			q->first_pkt = pktl->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			*pkt = pktl->tPkt;
			free(pktl);
			ret = 1;
			// DebugPrint(3,"[%s %d] pktl!=NULL",__FUNCTION__, __LINE__);
			break;
		} else if (!block) {
			// DebugPrint(3,"[%s %d] block == NULL",__FUNCTION__, __LINE__);
			ret = 0;
			break;
		} else {
			Sleep(1);
		}
	}
	q->mutex->Unlock();
	return ret;
}


/*************************************************
函数功能:     VDEC_Init
函数描述:     初使化视频解码
输入参数:     无
输出参数:     无
返回值:       0:成功, 否则:失败
作者:         丁金奇
日期:         2014-04-20
修改:   
*************************************************/
int VDEC_Init(void)
{
	//avcodec_init();
	av_register_all();
	avcodec_register_all();
	return 0;
}


int VDEC_Uninit()
{
	return 0;
}

static void packet_Fisheye_Yuv_init(T_FISHEYE_YUV_QUEUE *q)
{
	q->tFisheyeYuv.iHeight = 0;
	q->tFisheyeYuv.iWidth = 0;
	q->tFisheyeYuv.iYstride = 0;

	q->tFisheyeYuv.pYData = (unsigned char *)malloc(3072*3072 );
	q->tFisheyeYuv.pUData = (unsigned char *)malloc(3072*3072/4);
	q->tFisheyeYuv.pVData = (unsigned char *)malloc(3072*3072/4);
	q->bReadFlag = TRUE;
}

static void packet_Fisheye_Yuv_uninit(T_FISHEYE_YUV_QUEUE *q)
{
	if(q)
	{
		if(q->tFisheyeYuv.pYData)
		{
			free( q->tFisheyeYuv.pYData);
			q->tFisheyeYuv.pYData = NULL;
		}
		if(q->tFisheyeYuv.pUData)
		{
			free( q->tFisheyeYuv.pUData);
			q->tFisheyeYuv.pUData = NULL;
		}
		if(q->tFisheyeYuv.pVData)
		{
			free( q->tFisheyeYuv.pVData);
			q->tFisheyeYuv.pVData = NULL;
		}

	}
}

static int packet_Fisheye_YUV_get(T_FISHEYE_YUV_QUEUE *q, T_FISHEYE_YUV_PACKET *pkt)
{
	if(!q->bReadFlag)
	{
		q->cLock.Lock();
		memcpy(pkt->pYData,q->tFisheyeYuv.pYData,q->tFisheyeYuv.iHeight*q->tFisheyeYuv.iYstride);
		memcpy(pkt->pUData,q->tFisheyeYuv.pUData,q->tFisheyeYuv.iHeight*q->tFisheyeYuv.iYstride/4);
		memcpy(pkt->pVData,q->tFisheyeYuv.pVData,q->tFisheyeYuv.iHeight*q->tFisheyeYuv.iYstride/4);
		q->bReadFlag = TRUE;
		q->cLock.Unlock();
		return 1;
	}

	return 0;
}

static int packet_Fisheye_YUV_put(T_FISHEYE_YUV_QUEUE *q, unsigned char *pYdata,unsigned char *pUdata, unsigned char *pVdata,int iWidth, int iHeight, int iYstride)
{
	q->cLock.Lock();
	if((iWidth != q->tFisheyeYuv.iWidth) || (iHeight != q->tFisheyeYuv.iHeight) || (iYstride != q->tFisheyeYuv.iYstride))
	{

		q->tFisheyeYuv.iHeight = iHeight;
		q->tFisheyeYuv.iWidth = iWidth;
		q->tFisheyeYuv.iYstride = iYstride;

	}
	memcpy(q->tFisheyeYuv.pYData,pYdata,iHeight*iYstride);
	memcpy(q->tFisheyeYuv.pUData,pUdata,(iHeight*iYstride)/4);
	memcpy(q->tFisheyeYuv.pVData,pVdata,(iHeight*iYstride)/4);
	q->bReadFlag = FALSE;
	q->cLock.Unlock();
	return 1;
}

static AVPixelFormat GetHwFormat(AVCodecContext *s, const AVPixelFormat *pix_fmts)
{
	InputStream* ist = (InputStream*)s->opaque;
	ist->active_hwaccel_id = HWACCEL_DXVA2;
	ist->hwaccel_pix_fmt = AV_PIX_FMT_DXVA2_VLD;
	return ist->hwaccel_pix_fmt;
}

static int FisheyeProc(void *argc)
{
	PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)argc;
	T_FISHEYE_YUV_PACKET tFisheyeData;
	int iDesWidth  = 0;
	int iDesHeight = 0;
	BOOL iInit = FALSE;
	memset(&tFisheyeData,0,sizeof(T_FISHEYE_YUV_PACKET));

	tFisheyeData.pYData   = (unsigned char *)malloc(3072*3072);
	tFisheyeData.pUData   = (unsigned char *)malloc(3072*3072/4);
	tFisheyeData.pVData   = (unsigned char *)malloc(3072*3072/4);

	while(!ptVideoInfo->iVideoExitFlag)
	{
		int iHeight  = ptVideoInfo->tFisheyeYuv.tFisheyeYuv.iHeight;
		int iYstride = ptVideoInfo->tFisheyeYuv.tFisheyeYuv.iYstride;
		int iWidth   = ptVideoInfo->tFisheyeYuv.tFisheyeYuv.iWidth;
		if(!iInit)
		{
			InitFisheye(&ptVideoInfo->hFisheyehandle,&ptVideoInfo->pCorrect_data, iWidth ,iHeight,iWidth,iHeight);
			InitFishsyePTZ(ptVideoInfo->hFisheyehandle);
			iInit = TRUE;
		}
		if((tFisheyeData.iHeight !=iHeight) || (tFisheyeData.iWidth != iWidth) || (tFisheyeData.iYstride != iYstride)  )
		{
			tFisheyeData.iHeight  = iHeight;
			tFisheyeData.iWidth   = iWidth;
			tFisheyeData.iYstride = iYstride;
		}
		int ret = packet_Fisheye_YUV_get(&ptVideoInfo->tFisheyeYuv,&tFisheyeData);
		if(! ret )
		{
			Sleep(50);
		}
		else
		{

			FishCorrect(ptVideoInfo->hFisheyehandle,&ptVideoInfo->pCorrect_data,tFisheyeData.pYData, tFisheyeData.pUData, tFisheyeData.pVData, tFisheyeData.iYstride, tFisheyeData.iYstride/2, 
				tFisheyeData.iWidth, tFisheyeData.iHeight, iDesWidth, iDesHeight);

			unsigned char *pY = (unsigned char *)ptVideoInfo->pCorrect_data;
			unsigned char *pU = (unsigned char *)(ptVideoInfo->pCorrect_data + iDesWidth*iDesHeight );
			unsigned char *pV = (unsigned char *)(ptVideoInfo->pCorrect_data + iDesWidth*iDesHeight + (((iDesWidth+1)/2)*((iDesHeight+1)/2)));

			if (0 == ptVideoInfo->iInitDDrawFlag)
			{
				ptVideoInfo->DRAWHandle = DDRAW_Init(ptVideoInfo->hWnd, iDesWidth, iDesHeight);
				ptVideoInfo->iVideoWidth = iDesWidth; 
				ptVideoInfo->iVideoHeight = iDesHeight;
				ptVideoInfo->iInitDDrawFlag = 1;
			}
			else if (((ptVideoInfo->iVideoWidth != iDesWidth) || (ptVideoInfo->iVideoHeight != iDesHeight)))
			{
				DebugPrint(DEBUG_VDEC_8,"[%s %d] DDRAW_Uninit", __FUNCTION__, __LINE__);
				DDRAW_Uninit(ptVideoInfo->DRAWHandle);
				ptVideoInfo->DRAWHandle = DDRAW_Init(ptVideoInfo->hWnd, iDesWidth, iDesHeight);
				ptVideoInfo->iVideoWidth = iDesWidth; 
				ptVideoInfo->iVideoHeight = iDesHeight;
			}

			DDRAW_DrawYUV(ptVideoInfo->DRAWHandle, ptVideoInfo->hWnd, pY, pU, pV, iDesWidth, iDesWidth/2, iDesWidth, iDesHeight);
			Sleep(50);
		}
	}

	if(tFisheyeData.pYData)
	{
		free(tFisheyeData.pYData);
		tFisheyeData.pYData = NULL;
	}
	if(tFisheyeData.pUData)
	{
		free(tFisheyeData.pUData);
		tFisheyeData.pUData = NULL;
	}
	if(tFisheyeData.pVData)
	{
		free(tFisheyeData.pVData);
		tFisheyeData.pVData = NULL;
	}

	return 0;
}


/* 视频播放线程,流程是从视频队列取视频流,再进行H264解码,然后显示 */
static int PlayVideoProc(void *argv)
{
    PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)argv;
    T_CODEC_INFO *ptCodecInfo = NULL;
    T_DATA_PACKET tPkt;
    int iRet = 0;
    unsigned int uiFrameNum = 0;
    AVPacket tAVPkt;
    int iFrameFinished =0;
    INT64 i64OldTime = 0, i64CurTime = 0;
    unsigned int iIntervalTime = 0, iSleepTime = 0;
    unsigned int iOldPts = 0;
    INT64 i64BaseTime = 0;
    unsigned int uiCacheTime = 0;
    int iDesWidth = 1920 ;
    int iDesHeight = 1080;
    int iPauseFlag = 0;  //0为播放 1为暂停
	int iFrameCount = 0;
	AVFormatContext *pfmt_ctx = NULL;
	DebugPrint(DEBUG_VDEC_8,"[%s %d] ptVideoInfo = %p",__FUNCTION__, __LINE__,ptVideoInfo);
    if (NULL == ptVideoInfo)
    {
        DebugPrint( DEBUG_VDEC_8,"[%s %d] error",__FUNCTION__, __LINE__);
        return 0;
    }
    ptCodecInfo = &ptVideoInfo->tCodecInfo;

    memset(&tPkt, 0, sizeof(tPkt));
    while(!ptVideoInfo->iVideoExitFlag)
    {
        tPkt.pcData = NULL;
        if (PAUSE_STREAM_PLAY == ptVideoInfo->iStartPlayFlag)
        {
            if(0 == iPauseFlag)
            {
                iPauseFlag = 1;  //1为刚暂停 没有读帧
            }
            Sleep(50);
        }

        else if (STOP_STREAM_PLAY == ptVideoInfo->iStartPlayFlag)
        {
            Sleep(5);
            iPauseFlag =0;
            continue;
        }
        else
        {
            iPauseFlag =0;
        }

        if(!iPauseFlag  || (iPauseFlag && 0== iFrameFinished ))//暂停时可能上一帧并不完整，解码不成功。如果此时队列里没有帧了那也没办法了。这只是降低了黑屏的几率
        {
            iRet = packet_queue_get(&ptVideoInfo->tPacketQueue, &tPkt, 0);
			if (0 == iRet )
			{
				Sleep(10);
				continue;
			}
            
        }
        if(tPkt.pcData)
        {
            av_init_packet(&tAVPkt);	
            tAVPkt.data = (unsigned char*)tPkt.pcData;	
            tAVPkt.size = tPkt.iLen;
            if(tPkt.iLen == 0)
            {
                if(tPkt.pcData)
                {
                    free(tPkt.pcData);
                    tPkt.pcData = NULL;                
                }
                continue;
            }
        }
       
        if(iPauseFlag <1 &&( NULL == tAVPkt.data ||NULL == ptCodecInfo->pAVCodecContext||NULL == ptCodecInfo->pAVFrame))
        {
            continue;
        }

        if(tPkt.pcData)
        {
            iRet = avcodec_decode_video2(ptCodecInfo->pAVCodecContext, ptCodecInfo->pAVFrame,
                &iFrameFinished, &tAVPkt);
        }
  
        if (iFrameFinished)
        {
            unsigned char *pY = NULL;
            unsigned char *pU = NULL;
            unsigned char *pV = NULL;
            int iYStride = 0;
            int  iWidth = ptCodecInfo->pAVCodecContext->width;
            int  iHeight = ptCodecInfo->pAVCodecContext->height;

			if( CMP_VDEC_DXVA == ptVideoInfo->iDecType)
			{
				dxva2_retrieve_data_call(ptCodecInfo->pAVCodecContext, ptCodecInfo->pAVFrame);
			}
			else if (CMP_VDEC_NORMAL == ptVideoInfo->iDecType)
            {
                if (0 == ptVideoInfo->iInitDDrawFlag)
                {
                    ptVideoInfo->DRAWHandle = DDRAW_Init(ptVideoInfo->hWnd, iWidth, iHeight);
                    ptVideoInfo->iInitDDrawFlag = 1;
                    ptVideoInfo->iVideoWidth = iWidth;
                    ptVideoInfo->iVideoHeight = iHeight;
                }
                else if((ptVideoInfo->iVideoWidth != iWidth) || (ptVideoInfo->iVideoHeight != iHeight))
                {
                    DDRAW_Uninit(ptVideoInfo->DRAWHandle);
                    ptVideoInfo->DRAWHandle = DDRAW_Init(ptVideoInfo->hWnd, iWidth, iHeight);
                    ptVideoInfo->iVideoWidth = iWidth;
                    ptVideoInfo->iVideoHeight = iHeight;
                    DebugPrint(DEBUG_VDEC_8,"[%s %d] DDRAW_Uninit", __FUNCTION__, __LINE__);
                }

                pY = ptCodecInfo->pAVFrame->data[0];
                pU = ptCodecInfo->pAVFrame->data[1];
                pV = ptCodecInfo->pAVFrame->data[2];
                iYStride = ptCodecInfo->pAVFrame->linesize[0];
                DDRAW_DrawYUV(ptVideoInfo->DRAWHandle, ptVideoInfo->hWnd, pY, pU, pV, iYStride, iYStride/2, iWidth, iHeight);

                // 将解码后的YUV数据COPY到抓拍YUV缓存,为抓拍功能做准备
                //SaveYuv2CaptureYuv(ptVideoInfo, pY, pU, pV, iYStride, iYStride/2, iWidth, iHeight);
            }
            else if(CMP_VDEC_FISHEYE == ptVideoInfo->iDecType && 0 != uiFrameNum%3)
            {
                pY = ptCodecInfo->pAVFrame->data[0];
                pU = ptCodecInfo->pAVFrame->data[1];
                pV = ptCodecInfo->pAVFrame->data[2];
                iYStride = ptCodecInfo->pAVFrame->linesize[0];
                packet_Fisheye_YUV_put(&ptVideoInfo->tFisheyeYuv,pY,pU,pV,iWidth,iHeight,iYStride);

                if (NULL == ptVideoInfo->hFishThreadHand)
                {
                    ptVideoInfo->hFishThreadHand = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FisheyeProc,(void *)ptVideoInfo,0,NULL);
                }
            }
        }

        uiFrameNum ++;
       
        if(tPkt.pcData)
        {
            free(tPkt.pcData);
            tPkt.pcData = NULL;                
        }
    }

    return 0;
}

VDEC_HADNDLE VDEC_CreateVideoDecCh(HWND hWnd,  int iWidth, int iHeight,int iDecType,int iCodecID)
{
	PT_VIDEO_DEC_INFO ptVideoInfo = NULL;
	AVFrame *pFrame = NULL;
	AVCodecContext *pCodecContext = NULL;
	AVCodec *pAVCodec = NULL;
	AVFormatContext *pAVFormat = NULL;
	int iRet = 0;

	ptVideoInfo = new T_VIDEO_DEC_INFO;
	if (NULL == ptVideoInfo)
	{
		DebugPrint( DEBUG_VDEC_8,"[%s %d] error",__FUNCTION__, __LINE__);
		return 0;
	}
	memset(ptVideoInfo,0,sizeof(ptVideoInfo));
	memset(&ptVideoInfo->tCodecInfo, 0, sizeof(ptVideoInfo->tCodecInfo));
	ptVideoInfo->iInitDDrawFlag = 0;
	ptVideoInfo->DRAWHandle = 0;
	ptVideoInfo->pcCaptureYuv = NULL;
	ptVideoInfo->hWnd = hWnd;
	ptVideoInfo->hVideoPlayThread = 0;

	ptVideoInfo->iStartPlayFlag = STOP_STREAM_PLAY;
	ptVideoInfo->nCapturebmp = 0;
	ptVideoInfo->pcPictFileName = NULL;
	ptVideoInfo->iDecType = iDecType;
	ptVideoInfo->pCorrect_data = NULL;
	ptVideoInfo->hFisheyehandle = NULL;
	ptVideoInfo->hFishThreadHand = NULL;
	ptVideoInfo->iVideoWidth  = iWidth;
	ptVideoInfo->iVideoHeight = iHeight;
	packet_queue_init(&ptVideoInfo->tPacketQueue);

   if(CMP_VDEC_FISHEYE == ptVideoInfo->iDecType)
	{
		packet_Fisheye_Yuv_init(&ptVideoInfo->tFisheyeYuv);
	}

	ptVideoInfo->iVideoExitFlag = 0;
	ptVideoInfo->hVideoPlayThread = NULL;

	ptVideoInfo->tCodecInfo.pAVCodecContext = NULL;
	ptVideoInfo->tCodecInfo.pAVCodec = NULL;
	ptVideoInfo->tCodecInfo.pAVFrame = NULL;


	
	pCodecContext = avcodec_alloc_context3(NULL);
	if (NULL == pCodecContext)
	{
		DebugPrint( DEBUG_VDEC_8,"[%s %d] avcodec_alloc_context3 error",__FUNCTION__, __LINE__);
		return -1; 
	}
	pAVCodec  = avcodec_find_decoder((AVCodecID)iCodecID);
	if (NULL == pAVCodec)
	{
		avcodec_close(pCodecContext);
		av_free(pCodecContext);
		DebugPrint( DEBUG_VDEC_8,"[%s %d] avcodec_find_decoder error",__FUNCTION__, __LINE__);
		return -1; 
	}
	ptVideoInfo->iVideoWidth  = iWidth?iWidth:1920;
	ptVideoInfo->iVideoHeight = iHeight?iHeight:1080;
	pCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecContext->codec_id	= (AVCodecID)iCodecID;
	pCodecContext->width	= iWidth?iWidth:1920;
	pCodecContext->height	= iHeight?iHeight:1080;
	pCodecContext->pix_fmt  = AV_PIX_FMT_YUV420P;

	if(CMP_VDEC_DXVA == ptVideoInfo->iDecType)
	{
		pCodecContext->thread_count = 1;
		InputStream *ist = new InputStream();
		ist->hwaccel_id = HWACCEL_AUTO;
		ist->active_hwaccel_id = HWACCEL_AUTO;
		ist->hwaccel_device = "dxva2";
		ist->dec = pAVCodec;
		ist->dec_ctx = pCodecContext;

		pCodecContext->opaque = ist;
		if (0 == dxva2_init(pCodecContext, ptVideoInfo->hWnd)) //失败的话这里面已经对dxva的相关资源进行了释放
		{
			pCodecContext->get_buffer2 = ist->hwaccel_get_buffer;
			pCodecContext->get_format = GetHwFormat;
			pCodecContext->thread_safe_callbacks = 1;
		}
		else
		{
			ptVideoInfo->iDecType = CMP_VDEC_NORMAL;
		}
	}

	//通过解码器对象描述与解码器ID打开编解码器
	iRet = avcodec_open2(pCodecContext, pAVCodec, NULL);
	if (iRet < 0)
	{
		av_free(pCodecContext);
		av_free(pAVCodec);
		DebugPrint(DEBUG_VDEC_8,"[%s %d] avcodec_open error",__FUNCTION__, __LINE__);
		return -1;
	}		
	pFrame = av_frame_alloc();
	if (NULL == pFrame)
	{
		av_free(pCodecContext);
		av_free(pAVCodec);
		DebugPrint(DEBUG_VDEC_8,"[%s %d] avcodec_alloc_frame error",__FUNCTION__, __LINE__);
		return -1;
	}
	ptVideoInfo->tCodecInfo.pAVCodecContext = pCodecContext;
	ptVideoInfo->tCodecInfo.pAVCodec = pAVCodec;
	ptVideoInfo->tCodecInfo.pAVFrame = pFrame;

	ptVideoInfo->iVideoExitFlag = 0;
	ptVideoInfo->hVideoPlayThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)PlayVideoProc,(void *)ptVideoInfo,0,NULL);
	return (VDEC_HADNDLE)ptVideoInfo;

}


int VDEC_DestroyVideoDecCh(VDEC_HADNDLE VHandle)
{
	PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

	if (NULL == ptVideoInfo)
	{
		DebugPrint( DEBUG_VDEC_8,"[%s %d] error",__FUNCTION__, __LINE__);
		return -1;
	}

	ptVideoInfo->iVideoExitFlag = 1;
	
	if (ptVideoInfo->hVideoPlayThread)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(ptVideoInfo->hVideoPlayThread, INFINITE))
		{
			CloseHandle(ptVideoInfo->hVideoPlayThread);
		} 
		else
		{
			DWORD nExitCode;
			GetExitCodeThread(ptVideoInfo->hVideoPlayThread, &nExitCode);
			if (STILL_ACTIVE == nExitCode)
			{
				TerminateThread(ptVideoInfo->hVideoPlayThread, nExitCode);

			}	
			CloseHandle(ptVideoInfo->hVideoPlayThread);
		}
		ptVideoInfo->hVideoPlayThread = NULL;
	}
	avcodec_close(ptVideoInfo->tCodecInfo.pAVCodecContext);
	if(CMP_VDEC_DXVA == ptVideoInfo->iDecType)
	{
		dxva2_uninit(ptVideoInfo->tCodecInfo.pAVCodecContext);
	}

	av_free(ptVideoInfo->tCodecInfo.pAVCodec);
	av_free(ptVideoInfo->tCodecInfo.pAVFrame);
	av_free(ptVideoInfo->tCodecInfo.pAVCodecContext);

	ptVideoInfo->tCodecInfo.pAVCodecContext = NULL;
	ptVideoInfo->tCodecInfo.pAVFrame = NULL;

	if(CMP_VDEC_FISHEYE == ptVideoInfo->iDecType)
	{
		if(ptVideoInfo->hFishThreadHand)
		{
			if (WAIT_OBJECT_0 == WaitForSingleObject(ptVideoInfo->hFishThreadHand, INFINITE))
			{
				CloseHandle(ptVideoInfo->hFishThreadHand);
			} 
			else
			{
				DWORD nExitCode;
				GetExitCodeThread(ptVideoInfo->hFishThreadHand, &nExitCode);
				if (STILL_ACTIVE == nExitCode)
				{
					TerminateThread(ptVideoInfo->hFishThreadHand, nExitCode);

				}	
				CloseHandle(ptVideoInfo->hFishThreadHand);
			}
			ptVideoInfo->hFishThreadHand = NULL;

		}
		packet_Fisheye_Yuv_uninit(&ptVideoInfo->tFisheyeYuv);
		ReleaseFisheyeSource(ptVideoInfo->hFisheyehandle,ptVideoInfo->pCorrect_data);
	}
	packet_queue_uninit(&ptVideoInfo->tPacketQueue);
	if(ptVideoInfo->iInitDDrawFlag)
	{
		DDRAW_Uninit(ptVideoInfo->DRAWHandle);
		ptVideoInfo->DRAWHandle = NULL;
		ptVideoInfo->iInitDDrawFlag = 0;
	}

	if (ptVideoInfo->pcCaptureYuv)
	{
		free(ptVideoInfo->pcCaptureYuv);
		ptVideoInfo->pcCaptureYuv = NULL;
	}
	delete(ptVideoInfo);
	ptVideoInfo = NULL;

	return 0;
}

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
int VDEC_SendStream(VDEC_HADNDLE VHandle, void *pData, int iLen, unsigned int iPts)
{
	T_DATA_PACKET tPkt;
	PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;


	if(iLen > 750000)
	{
		DebugPrint( DEBUG_VDEC_8,"[%s %d] iLen Biger 750000",__FUNCTION__, __LINE__);
	}

	if (NULL == ptVideoInfo)
	{
		DebugPrint( DEBUG_VDEC_8,"[%s %d] error",__FUNCTION__, __LINE__);
		return -1;
	}

	if (NULL == pData || 0 == iLen)
	{
		DebugPrint( DEBUG_VDEC_8,"[%s %d] NULL == pData || 0 == iLen",__FUNCTION__, __LINE__);
		return -1;
	}

	if(ptVideoInfo->iStartPlayFlag != START_STREAM_PLAY)
	{
		return 0;
	}

	if(ptVideoInfo->tPacketQueue.mutex)
	{
		tPkt.pcData = (char *)malloc(iLen+32);
		tPkt.iLen = iLen;
		tPkt.iPts = iPts;
		memcpy(tPkt.pcData, pData, iLen);
		packet_queue_put(&ptVideoInfo->tPacketQueue, &tPkt);
	}

	return 0;
}


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

int VDEC_ChangeWindow(VDEC_HADNDLE VHandle, HWND hWnd)
{
	PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

	if (NULL == ptVideoInfo)
	{
		DebugPrint( DEBUG_VDEC_8,"[%s %d] error",__FUNCTION__, __LINE__);
		return -1;
	}
	ptVideoInfo->hWnd = hWnd;

	if (1 == ptVideoInfo->iInitDDrawFlag  && ptVideoInfo->iDecType != CMP_VDEC_DXVA)
	{

		DDRAW_ChangeWindows(ptVideoInfo->DRAWHandle, hWnd);
	}
	return 0;
}

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
int VDEC_StartPlayStream(VDEC_HADNDLE VHandle)
{
	PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

	if (NULL == ptVideoInfo)
	{
		DebugPrint( DEBUG_VDEC_8,"[%s %d] error",__FUNCTION__, __LINE__);
		return -1;
	}

	ptVideoInfo->iStartPlayFlag = START_STREAM_PLAY;

	return 0;
}

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
int VDEC_StopPlayStream(VDEC_HADNDLE VHandle)
{
	PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

	if (NULL == ptVideoInfo)
	{
		DebugPrint( DEBUG_VDEC_8,"[%s %d] error",__FUNCTION__, __LINE__);
		return -1;
	}
	ptVideoInfo->iStartPlayFlag = STOP_STREAM_PLAY;
	Sleep(10);
	//g_iVideoExitFlag = 1;
	packet_queue_flush(&ptVideoInfo->tPacketQueue);
	return 0;
}

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
int VDEC_PausePlayStream(VDEC_HADNDLE VHandle)
{
	PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

	if (NULL == ptVideoInfo)
	{
		DebugPrint( DEBUG_VDEC_8,"[%s %d] error",__FUNCTION__, __LINE__);
		return -1;
	}
	ptVideoInfo->iStartPlayFlag = PAUSE_STREAM_PLAY;

	return 0;
}
