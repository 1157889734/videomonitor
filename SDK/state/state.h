#ifndef _STATE_H
#define _STATE_H

#ifdef  __cplusplus
extern "C"
{
#endif


#define NVR_MAX_NUM 	8
#define NVR_IPC_MAX_NUM 32

typedef enum _E_FILE_SEARCH_STATE
{
	E_FILE_IDLE =0,
	E_FILE_SEARCHING,
	E_FILE_FINISH
}E_FILE_SEARCH_STATE;

typedef enum _E_FILE_DOWN_STATE
{
	E_FILE_DOWN_IDLE =0,
	E_FILE_DOWNING,
	E_FILE_DOWN_SUCC,
	E_FILE_DOWN_FAIL,
}E_FILE_DOWN_STATE;

typedef enum _E_VIDEO_SOURCE_STATE
{
	E_SOURCE_IDLE = 0x11,
	E_SOURCE_WAIT = 0x12,
	E_SOURCE_SEND_FAIL = 0x13,
	E_SOURCE_SET_SUCC  = 0x14,
	E_SOURCE_ALIVE_FAIL = 0x15,
	E_SOURCE_DIGTV_FAIL = 0x16,
	E_SOURCE_LOCAL_FAIL	= 0x17
}E_VIDEO_SOURCE_STATE;

typedef enum
{
	E_UNKONE_VPLAY =	0,
	E_SINGLE_VPLAY = 1,
	E_FOUR_VPLAY	= 2,
}E_PLAY_STYLE;

typedef enum
{
	E_RES_UPDATE_UNKNOW= 0,
	E_RES_UPDATE_START = 1,
	E_RES_UPDATE_ING   = 2,
	E_RES_UPDATE_SUCC  = 3,
	E_RES_UPDATE_FAIL  = 4	
}E_RES_UPDATE_STATE;


typedef struct _T_DATA_LIST
{
    struct _T_DATA_LIST *pLast;
    void                *pData;
}T_DATA_LIST;

typedef struct _T_USER_LOGIN_INFO
{
	char acUserName[32];
	char acPassword[32];
}T_USER_LOGIN_INFO, *PT_USER_LOGIN_INFO;

typedef struct _T_IPC_ID
{
    char cCarriageNo;
    char cPos;
}  T_IPC_ID, *PT_IPC_ID;

typedef struct _T_IPC_ATTRIBUTE_INFO
{
    char cLightNess;	// 亮度，0～255可调
    char cSaturation; 	// 饱和度，0～255可调
    char cContrast; 	// 对比度，0～255可调
}  T_IPC_ATTRIBUTE_INFO, *PT_IPC_ATTRIBUTE_INFO;

typedef struct _T_IPC_STATE
{
	char cOnlineState;	// 摄像机的在线状态，在线：01；不在线：00
	char cDevType; 		// 摄像机类型，长焦红外摄像机：01；广角摄像机：02；长焦摄像机：03
    char acFactory[10];	// 厂家，英文简写
	short sVersion; 	// 版本，举例：110，对应版本号V1.1.0
}T_IPC_STATE, *PT_IPC_STATE;

typedef struct _T_IPC_WARN_STATE
{
	char cShelter; 		// 遮挡参数：00：无遮挡；01：遮挡
	char cLost; 		// 丢失参数：00：无丢失；01：丢失
    unsigned int nTime;
}T_IPC_WARN_STATE, *PT_IPC_WARN_STATE;

typedef struct _T_NVR_STATE
{
    char cCarriageNo;	// 车厢号
	char cDevType; 		// 服务器：01
    char acFactory[10];	// 厂家，英文简写
    short sVersion; 	// 版本，举例：110，对应版本号V1.1.0
	short sHdiskSize; 	// 硬盘容量，单位G
	short sHdiskUseSize;// 已使用容量，单位G
}T_NVR_STATE, *PT_NVR_STATE;

typedef struct _T_NVR_WARN_STATE
{
    char cHdiskLost;      // bit0:硬盘1,bit1:硬盘2, ... , 0：无丢失；1：丢失
    char cHdiskBad;       // bit0:硬盘1,bit1:硬盘2, ... , 0：无损坏；1：损坏
    unsigned int nTime;
}T_NVR_WARN_STATE, *PT_NVR_WARN_STATE;

typedef struct _T_IPC_INFO
{
	char cPos;
	char cCarriageNo;
	char acChannelName[32];
	char acIp[32];
    char acRtspMaster[256];
    char acRtspSlave[256];
	char bConf;
	T_IPC_STATE 	 	 tState;
	T_IPC_WARN_STATE 	 tWarnState;
	T_IPC_ATTRIBUTE_INFO tAttribute;
    T_DATA_LIST          tWarnStateList;
}T_IPC_INFO, *PT_IPC_INFO;

typedef struct _T_NVR_INFO
{
	int iNo;
	int iIPCNum;
	char acIp[32];
	char acNVRName[256];
	T_NVR_STATE 		tState;
	T_NVR_WARN_STATE 	tWarnState;
	T_USER_LOGIN_INFO 	tUserLogin;
	T_IPC_INFO 			atIpcInfo[NVR_IPC_MAX_NUM];
    T_DATA_LIST         tWarnStateList;
}T_NVR_INFO, *PT_NVR_INFO;

enum _E_DISP_STATE
{
    DISP_STATE_UNKOWN = 0,
    DISP_STATE_DMI,
    DISP_STATE_HMI,
    DISP_STATE_CCTV,
};
typedef void (__stdcall *STATUSCHANGECALLBACK)(int iNvrNo, void* ptId, void *ptState,void* pUserData) ;
typedef void (__stdcall *SEARCHOVERCALLBACK)(void* pUserData) ;
typedef void (__stdcall *NVRDISCONNECTCALLBACK)(int nDisconnect,int iNvrNo,void* pUserData) ;
//设置CCTV的运行目录
int STATE_SetCCTVRunDir(const char *pcData,int iLen);
//获取CCTV的运行目录
int STATE_GetCCTVRunDir(char *pcRunData,int iLen);

//初始化前先设置运行目录
int STATE_Init(void);
int STATE_Uninit(void);


//******************CCTV屏*******************
//******************相机状态******************
//相机的状态
int STATE_SetIpcState(int iNvrNo, PT_IPC_ID ptId, T_IPC_STATE *ptState);
int STATE_GetIpcState(int iNvrNo, PT_IPC_ID ptId, T_IPC_STATE *ptState);
int STATE_CleanIPCState(int iNvrNo);
int STATE_NVROnline(int iNvrNo);
int STATE_SetIpcWarnState(int iNvrNo, PT_IPC_ID ptId, T_IPC_WARN_STATE *ptState);
int STATE_GetIpcWarnState(int iNvrNo, PT_IPC_ID ptId, T_IPC_WARN_STATE *ptState);

int STATE_SetIpcAttribute(int iNvrNo, PT_IPC_ID ptId, T_IPC_ATTRIBUTE_INFO *ptState);
int STATE_GetIpcAttribute(int iNvrNo, PT_IPC_ID ptId, T_IPC_ATTRIBUTE_INFO *ptState);

// 获取车厢对应的NVR编号
int STATE_GeNvrNoByCarriageNo(char cCarriageNo);

// 获取一节车厢的所有相机位置编号
int STATE_GetAllIpcPosCarriageNo(char cCarriageNo, char *pacPos, int nMax);

//
int STATE_GetCarriageNum();


int STATE_GetIpcRtsp(char cCarriageNo,char cPos,char *rtsp, int nMaster);

//********************NVR相关信息*****************

int STATE_GetNvrNum();

int STATE_GetNvrIpAddr(int iNvrNo, char *pIp);

//Nvr硬盘
int STATE_GetNvrInfo(int iNvrNo,T_NVR_INFO *ptNvrInfo);
int STATE_SetNvrState(int iNvrNo,T_NVR_STATE *ptState);
int STATE_GetNvrState(int iNvrNo,T_NVR_STATE *ptState);

int STATE_SetNvrWarnState(int iNvrNo,T_NVR_WARN_STATE *ptState);
int STATE_GetNvrWarnState(int iNvrNo,T_NVR_WARN_STATE *ptState);



//******************文件搜索 文件下载******************
int STATE_SetFileSearchState(E_FILE_SEARCH_STATE iState,int iNotice);
E_FILE_SEARCH_STATE STATE_GetFileSearchState();
int STATE_SetFileDownState(E_FILE_DOWN_STATE eState);
E_FILE_DOWN_STATE STATE_GetFileDownState( );
int STATE_SetFileDownProgress(int iProgress);
int STATE_GetFileDownProgress();
int STATE_FindUsbDev();


//********************音频管理**********************************
int SetASCUMasterSlaveFlag();
int GetASCUMasterSlaveFlag();

int SetDriverRoomMonitorFlag(int iFlag);
int GetDriverRoomMonitorFlag();

int SetNoiseMonitorFlag(int iEnable);
int GetNoiseMonitorFlag();

int SetLcdVolumeValue(int iValue);
int GetLcdVolumeValue();

int SetDriverRoomSpeakerVolume(int iValue);  //司机监控室
int GetDriverRoomSpeakerVolume();

int SetCarriageSpeakerVolume(int iValue);  //客室
int GetCarriageSpeakerVolume();

int SetDynamicMapBrightness(int iValue);
int GetDynamicMapBrightness();

int SetVideoSource(int iVideoType);  //0x11 直播 0x12 数字电视 0x13 录播
int GetVideoSource();


//************************报警管理*********************************
//第iNo个Pecu对应的相机索引          从0开始
int GetPecuVideoIndex(int iPecuIdx);
//Pecu的报警信息（24个PECU报警，最低位代表第1个PECU,依次类推代表24个PECU）

//00:与更新客户端无连接 01:开始更新 02：更新中 03：更新结束 04：更新失败 
int SetResUpdateState(char cState,char cProgress);
int GetResUpdateState(char *pcState,char *pcProgress);

//0 : DISP_STATE_UNKOWN 1:DISP_STATE_DMI 2:DISP_STATE_HMI 3:DISP_STATE_CCTV
void SetDisplayState(int iState);
int  GetDisplayState(void);

//1:只显示DMI 2:只显示HMI 3:CCTV
int GetDevDispMode();
int GetTestEnableFlg(char * pcFlg);

int GetCycTime();
void STATE_SetIPCStatusCallback(STATUSCHANGECALLBACK callbackIPCStatusChange,void* pUserData);
void STATE_SetSearchOverCallback(SEARCHOVERCALLBACK callbackSearchOver,void *pUserData);
void STATE_SetNVRDisconnectCallback(NVRDISCONNECTCALLBACK callbackNVRDisconnect,void* pUserData) ;
#ifdef  __cplusplus
}
#endif
#endif

