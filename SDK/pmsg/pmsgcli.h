#ifndef  __PMSG_CLI_H__
#define __PMSG_CLI_H__

#include "types.h"

#define MSG_START_FLAG 0xFF

typedef  unsigned long PMSG_HANDLE;

typedef int (*PF_MSG_PROC_CALLBACK)(PMSG_HANDLE pMsgHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen);

typedef enum _E_SERV_CONNECT_STATUS
{
    E_SERV_STATUS_UNCONNECT	= 0,
    E_SERV_STATUS_CONNECT = 1
}E_SERV_CONNECT_STATUS;

//client -> nvr
typedef enum _E_MSG_CLI2SERV
{
    CLI_SERV_MSG_TYPE_HEART = 0x51,
    CLI_SERV_MSG_TYPE_SET_PIC_ATTRIBUTE = 0x02,
    CLI_SERV_MSG_TYPE_GET_PIC_ATTRIBUTE = 0x03,
    CLI_SERV_MSG_TYPE_CHECK_TIME = 0x05,
    CLI_SERV_MSG_TYPE_GET_NVR_STATUS = 0x06,
    CLI_SERV_MSG_TYPE_GET_IPC_STATUS = 0x07,
    CLI_SERV_MSG_TYPE_GET_RECORD_FILE = 0x10, // 搜索视频报文
    CLI_SERV_MSG_TYPE_SET_CARRIAGE_NUM = 0x11,//设置车号信息报文
    CLI_SERV_MSG_TYPE_SET_PTZ = 0x12,
    CLI_SERV_MSG_TYPE_SET_PRESETS = 0x13,
    CLI_SERV_MSG_TYPE_PVMS_IPC_CTRL = 0x14,
    CLI_SERV_MSG_TYPE_PVMS_LIGHT_CTRL = 0x15,
    CLI_SERV_MSG_TYPE_PVMS_UP_DOWN_RESP = 0x66,
    CLI_SERV_MSG_TYPE_SEND_PVMS_GPS = 0x71,
    CLI_SERV_MSG_TYPE_GET_LIGHT_STATUS = 0x22, /* ��ò����״̬*/
    CLI_SERV_MSG_TYPE_GET_RECORD_TIME_LEN = 0x42,
    CLI_SERV_MSG_TYPE_SET_PVMS_INFO = 0x43,
    CLI_SERV_MSG_TYPE_GET_CARRAGE_NVR_STATUS = 0x44
    
} E_MSG_CLI2SERV;

// nvr -> client
typedef enum _E_MSG_SERV_CLI
{
    SERV_CLI_MSG_TYPE_HEART = 0x01,
    SERV_CLI_MSG_TYPE_GET_PIC_ATTRIBUTE_RESP = 0x53,
    SERV_CLI_MSG_TYPE_GET_NVR_STATUS_RESP = 0x56,
    SERV_CLI_MSG_TYPE_GET_IPC_STATUS_RESP = 0x57,
    SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP = 0x60,
    SERV_CLI_MSG_TYPE_SET_CARRIAGE_NUM_RESP = 0x61,
    SERV_CLI_MSG_TYPE_SET_PTZ_RESP = 0x62,
    SERV_CLI_MSG_TYPE_SET_PRESETS_RESP = 0x63,
    SERV_CLI_MSG_TYPE_PVMS_IPC_CTRL_RESP = 0x64,
    SERV_CLI_MSG_TYPE_PVMS_LIGHT_CTRL_RESP = 0x65,
    SERV_CLI_MSG_TYPE_PVMS_UP_DOWN_CTRL = 0x16,
    SERV_CLI_MSG_TYPE_IPC_ALARM_REPORT = 0x08,//图像报警报文
    SERV_CLI_MSG_TYPE_NVR_ALARM_REPORT = 0x09,//硬盘报警报文
    SERV_CLI_MSG_TYPE_PVMS_GPS_REPORT = 0x21,
    SERV_CLI_MSG_TYPE_GET_LIGHT_STATUS_RESP = 0x72,
    SERV_CLI_MSG_TYPE_GET_RECORD_TIME_LEN_RESP = 0x82,
    SERV_CLI_MSG_TYPE_SET_PVMS_INFO_RESP = 0x83,
    
} E_MSG_SERV_CLI;

enum E_PTZ_CTRL_TYPE
{
    E_PTZ_UP     = 1,
    E_PTZ_DOWN,
    E_PTZ_LEFT,
    E_PTZ_RIGHT,
    E_ZOOM_IN,
    E_ZOOM_OUT,
    E_FOCUS_FAR,
    E_FOCUS_NEAR,
};

enum E_PTZ_MOVE_TYPE
{
    E_START_MOVE = 1,
    E_STOP_MOVE
};

enum E_PRESET_CTRL_TYPE
{
    E_PRESET_SET = 1,
    E_PRESET_GET
};

typedef struct _T_PMSG_HEAD
{
    BYTE cMsgStartFLag;
    BYTE cMsgType;
    UINT16 sMsgLen;	
} T_PMSG_HEAD, *PT_PMSG_HEAD;

typedef struct _T_TIME_INFO
{
    INT16 year;
    INT8 mon;
    INT8 day;
    INT8 hour;
    INT8 min;
    INT8 sec; 	
} __attribute__((packed)) T_TIME_INFO;

typedef struct _T_NVR_STATUS
{
    INT8 cCarriageNo;	// 车厢号
	INT8 cDevType; 		// 服务器：01
	INT8 acFactory[10];	// 厂家，英文简写
	INT16 sVersion; 	// 版本，举例：110，对应版本号V1.1.0
	INT32 iState;		// 硬盘状态
	INT16 sHardSize; 	// 硬盘容量，单位G
	INT16 sHardUseSize;	// 已使用容量，单位G
} __attribute__((packed)) T_NVR_STATUS, *PT_NVR_STATUS;

typedef struct _T_IPC_STATUS
{
    INT8 cCarriageNo;	// 车厢号
	INT8 cPos; 			// 摄像机位置号
	INT8 cOnlineState;	// 摄像机的在线状态，在线：01；不在线：00
	INT8 cDevType; 		// 摄像机类型，长焦红外摄像机：01；广角摄像机：02；长焦摄像机：03
	INT8 acFactory[10];	// 厂家，英文简写
	INT16 sVersion; 	// 版本，举例：110，对应版本号V1.1.0

} __attribute__((packed)) T_IPC_STATUS, *PT_IPC_STATUS;	

typedef struct T_IPC_ALARM_STATUS
{
    INT8 cCarriageNo;	// 车厢号
    INT8 cPos; 			// 摄像机位置号
    INT8 cShelter;	    // 遮挡参数：00：无遮挡；01：遮挡
    INT8 cLost; 	    // 丢失参数：00：无丢失；01：丢失
} __attribute__((packed)) T_IPC_ALARM_STATUS, *PT_IPC_ALARM_STATUS;

typedef struct T_NVR_ALARM_STATUS
{
    INT8 cHdiskLost;      // 0:normal, 1:lost
    INT8 cHdiskBad;       // 0:normal, 1:bad
} __attribute__((packed)) T_NVR_ALARM_STATUS, *PT_NVR_ALARM_STATUS;

typedef struct _T_IPC_ATTRIBUTE
{
    INT8 cCarriageNo;	// 车厢号
    INT8 cPos; 			// 摄像机位置号
    INT8 cLightNess;	// 亮度，0～255可调
    INT8 cSaturation; 	// 饱和度，0～255可调
    INT8 cContrast; 	// 对比度，0～255可调
} __attribute__((packed)) T_IPC_ATTRIBUTE;

typedef struct _T_NVR_SEARCH_RECORD
{
    T_TIME_INFO tStartTime;
    T_TIME_INFO tEndTime;
    INT8 iCarriageNo;
    INT8 iIpcPos;
}  __attribute__((packed)) T_NVR_SEARCH_RECORD, *PT_NVR_SEARCH_RECORD;

typedef struct _T_PTZ_OPT
{
    INT8 i8CtrlType;
    INT8 i8MoveType;
} __attribute__((packed)) T_PTZ_OPT, *PT_PTZ_OPT;

typedef struct _T_PRESET_OPT
{
    INT8 i8CtrlType;
    INT8 i8PresetNo;
} __attribute__((packed)) T_PRESET_OPT, *PT_PRESET_OPT;

#ifdef __cplusplus
extern "C" {
#endif
int PMSG_Init(void);
int PMSG_Uninit(void);
PMSG_HANDLE PMSG_CreateConnect(char *pcIpAddr, int iPort, PF_MSG_PROC_CALLBACK pfMsgProcFunc);
int PMSG_DestroyConnect(PMSG_HANDLE pMsgHandle);
int PMSG_GetConnectStatus(PMSG_HANDLE pMsgHandle);
int PMSG_SendRawData(PMSG_HANDLE pMsgHandle, char *pcData, int iDataLen);
int PMSG_SendPmsgData(PMSG_HANDLE pMsgHandle, unsigned char ucMsgCmd, char *pcData, int iDataLen);
#ifdef __cplusplus
}
#endif
#endif
