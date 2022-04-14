#ifndef GLOBLEDEFINE_H
#define GLOBLEDEFINE_H

#include<string.h>
#include<QMutex>
#include<QVariant>
#include<QDateTime>

#define FAIL_COUNT_MAX 1
#define MAX_LEN 256

//最大车厢数
#define MAX_CARRIAGE_NUM 8

//每车厢的设备数(相机)
#define MAX_CARRIAGE_PRE_CAMERA_NUM		4

//每车厢相机总有页数
#define MAX_CARRIAGE_PAGE_NUM			2


#define STATE_ONLINE    1
#define STATE_OUTONLINE 0

#define CHAN_ORIGINAL		 6   //no preview, no record
#define CHAN_PLAY		 	 7   //preview
#define CHAN_RECORD			 8   //record
#define CHAN_PLAY_RECORD	 9   //preview and record

#define CHAN_ALARM		 	 10   //no preview, no record, only alarm
#define CHAN_PLAY_ALARM			 11   //review, no record, with alarm info
#define CHAN_PLAY_RECORD_ALARM	 12   //preview & record & alarm
#define CHAN_OFF_LINE			 13	 //channel off-line

#define VIDEO_WINDOWS_COUNT		4

enum  VIDEO_FORMAT
{
    VIDEO_1024_768,
    VIDEO_800_600,

};

enum  THREAD_STATUS
{
    THREAD_INIT,
    THREAD_RUNNING,
    THREAD_STOP,
};

enum  VIDEO_WINDOW_POWER
{
    POWER_VISTOR,
    POWER_USER,
};


enum  BUTTON_CONTROL_MAINSTATE
{
   VIDEO_REALMONITOR,
   VIDEO_PLAYBACK,
   VIDEO_DOWNLOAD,
   DEVICE_STATE,
   SOFTWARE_UPDATE,
};

enum  BUTTON_CONTROL_SUBSTATE
{
    PLAYBACK_START,
    PLAYBACK_STOP,
    PLAYBACK_FASTFORWARD,
    PLAYBACK_BACKOFF,
    PLAYBACK_DAYPLAY,
    PLAYBACK_MONTHPLAY
};

enum BUTTON_CONTROL_SUBSTATE_POLL
{
	REALMONITOR_POLLON =1,
	REALMONITOR_POLLOFF,
};

enum  DEMO_CHANNEL_TYPE
{
    DEMO_CHANNEL_TYPE_INVALID = -1,
    DEMO_CHANNEL_TYPE_ANALOG  = 0,
    DEMO_CHANNEL_TYPE_IP = 1,
    DEMO_CHANNEL_TYPE_MIRROR = 2
};




//设备数
enum MMS_DEVICE_TYPE
{
    //车厢1
    MMS_DEVICE_TYPE_CARRIAGE1,
    //车厢2
    MMS_DEVICE_TYPE_CARRIAGE2,
    //车厢3
    MMS_DEVICE_TYPE_CARRIAGE3,
    //车厢4
    MMS_DEVICE_TYPE_CARRIAGE4,
    //车厢5
    MMS_DEVICE_TYPE_CARRIAGE5,
    //车厢6
    MMS_DEVICE_TYPE_CARRIAGE6,
    //车厢7
    MMS_DEVICE_TYPE_CARRIAGE7,
    //车厢8
    MMS_DEVICE_TYPE_CARRIAGE8,

    MMS_DEVICE_TYPE_COUNT,
};


typedef struct _SingleVideoParams
{

    _SingleVideoParams():iCurCarriageIdx(0),
           iCameraIdx(0),
           iCurPage(0),
           iMainStatus(VIDEO_REALMONITOR),
           iPlayStatus(REALMONITOR_POLLON),
           iPower(POWER_VISTOR),
           PlayBackFile("")
    {

    }

   int iCurCarriageIdx;
   int iCameraIdx;
   int iCurPage;
   int iMainStatus;
   int iPlayStatus;
   int iPower;
   QDateTime StartDataTime;
   QDateTime StopDataTime;
   QString PlayBackFile;

}SingleVideoParams;
Q_DECLARE_METATYPE(SingleVideoParams);

typedef struct _VideoArea
{
    int X;
    int Y;
    int Width;
    int Height;

    _VideoArea()
    {
        Clear();
    }

    void Clear()
    {
        X=0;
        Y=0;
        Width =0;
        Height =0;
    }

}VideoArea;



//device index info
typedef struct _ChannelInfo
{

}ChannelInfo;

enum
{
    //最大通道
    MMS_MAX_CHANNUM				= 32,
    //IP组数
    MMS_MAX_IP_GROUPNUM			= 4,
};

#ifndef WIN32
typedef void* HWND;
typedef void* HANDLE;
#endif
//设备信息
typedef struct _DeviceParamInfo
{

}DeviceParamInfo;



typedef struct _CarriageInfo
{
    char cNvrNo;
    char cCarriageNo;
    char cIpcNum;
    char acIpcPos[MAX_CARRIAGE_PRE_CAMERA_NUM];

}CarriageInfo;



//下载信息
typedef struct _DownloadFileInfo
{
    _DownloadFileInfo()
        :port(0)
        ,bDownloadComplete(false)
    {
        memset(srcFileName, 0, sizeof(srcFileName));
        memset(dstFileName, 0, sizeof(dstFileName));
        memset(IP, 0, sizeof(IP));
    }
    //要下载的文件名
    char				srcFileName[256];
    //要保存的文件名
    char				dstFileName[256];
    //要下载的文件名IP地址
    char				IP[16];
    //要下载的文件名端口
    unsigned short		port;

    //是否已经下载成功
    bool				bDownloadComplete;
    //已下载文件个数
    static int			iDownloadedCount;

    //下载pos
    static int			iDownloadedPos;
}DownloadFileInfo;




extern CarriageInfo     g_atCarriages[MAX_CARRIAGE_NUM];
extern int				g_iCarriageNum;
extern int				g_iTurnTime;		//<=0，不轮询 30
extern int				g_iPlaybackSecond; //默认回放3分钟
extern QMutex           mutex;

#endif // GLOBLEDEFINE_H
