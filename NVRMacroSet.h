#pragma once
#include <vector>
#include <list>
#include <windows.h>

//CHN 安全清空STL的list列表
#define SAFE_CLEAR_LIST(CLASS_NAME, list_obj)  \
    if (!list_obj.empty())  \
{  \
    for (std::list<CLASS_NAME *>::iterator iter = list_obj.begin(); iter != list_obj.end(); ++iter)  \
    {  \
    if(NULL!=(*iter))\
        {\
        delete (*iter);  \
        (*iter) = NULL; \
        }\
    }  \
    list_obj.clear();  \
}  \


//CHN 安全清空STL的vector列表
#define SAFE_CLEAR_VECTOR(CLASS_NAME, vector_obj)  \
    if (!vector_obj.empty())  \
{  \
    for (std::vector<CLASS_NAME *>::iterator iter = vector_obj.begin(); iter != vector_obj.end(); ++iter)  \
    {  \
    if(NULL!=(*iter))\
        {\
        delete (*iter); \
        (*iter) = NULL; \
        }\
    }  \
    vector_obj.clear();  \
}  \

//CHN 安全后插入STL的list列表避免因插入指针相同的值导致清空异常
#define SAFE_PUSH_LIST(CLASS_NAME, list_obj, push_obj)  \
    if(1)\
{\
    int nFlg=0;\
    for (std::list<CLASS_NAME *>::iterator iter = list_obj.begin(); iter != list_obj.end(); ++iter)  \
    {  \
    if(push_obj==(*iter)) \
    nFlg =1; \
    }  \
    if(nFlg==0) \
    list_obj.push_back(push_obj);  \
}\

//CHN 安全后插入STL的vector列表避免因插入指针相同的值导致清空异常
#define SAFE_PUSH_VECTOR(CLASS_NAME, vector_obj, push_obj)  \
    if(1) \
{\
    int nFlg=0;\
    for (std::vector<CLASS_NAME *>::iterator iter = vector_obj.begin(); iter != vector_obj.end(); ++iter)  \
    {  \
    if(push_obj==(*iter)) \
    nFlg =1; \
    }  \
    if(nFlg==0) \
    vector_obj.push_back(push_obj);  \
}\
    

#pragma pack(1)

#ifndef _NETTOOL_H_

#define MSG_MAGIC_FLAG  0xFF
enum E_CLI2SERV_MSG
{
    MSG_CLI2SERV_BROADCAST_QUERY     = 0x01,
    MSG_CLI2SERV_HEART               = 0x02,
    MSG_CLI2SERV_SET_NET_PARAM       = 0x03,
    MSG_CLI2SERV_GET_NET_PARAM       = 0x04,
    MSG_CLI2SERV_SET_DEV_PARAM       = 0x05,
    MSG_CLI2SERV_GET_DEV_PARAM       = 0x06,
    MSG_CLI2SERV_SET_BOARD_INFO      = 0x07,
    MSG_CLI2SERV_GET_BOARD_INFO      = 0x08,
    MSG_CLI2SERV_SET_IPC_INFO_PARAM  = 0x09,
    MSG_CLI2SERV_GET_IPC_INFO_PARAM  = 0x0A,
    MSG_CLI2SERV_UPLOAD_UPDATE_FILE  = 0x0B,
    MSG_CLI2SERV_START_UPDATE        = 0x0C,
    MSG_CLI2SERV_GET_HDISK_STATUS    = 0x0D,
    MSG_CLI2SERV_REBOOT              = 0x0E,
    MSG_CLI2SERV_RESTORE_DEFAULT     = 0x0F,
    MSG_CLI2SERV_HDISK_FORMAT        = 0x10,
};

enum E_SERV2CLI_MSG
{
    MSG_SERV2CLI_BROADCAST_QUERY_RESP   = 0xA1,
    MSG_SERV2CLI_SET_NET_PARAM_RESP     = 0xA3,
    MSG_SERV2CLI_GET_NET_PARAM_RESP     = 0xA4,
    MSG_SERV2CLI_SET_DEV_PARAM_RESP     = 0xA5,
    MSG_SERV2CLI_GET_DEV_PARAM_RESP     = 0xA6,
    MSG_SERV2CLI_SET_BOARD_PARAM_RESP   = 0xA7,
    MSG_SERV2CLI_GET_BOARD_PARAM_RESP   = 0xA8,
    MSG_SERV2CLI_SET_IPC_INFO_RESP      = 0xA9,
    MSG_SERV2CLI_GET_IPC_INFO_RESP      = 0xAA,
    MSG_CLI2SERV_START_UPDATE_RESP      = 0xAC,
    MSG_SERV2CLI_GET_HDISK_STATUS_RESP  = 0xAD,
};


typedef struct _T_MSG_HEAD
{
    BYTE magic;    // 固定0xFF
    BYTE cmd;     // 消息命令
    INT16 len;      // 消息数据长度，不包括消息头
}T_MSG_HEAD, *PT_MSG_HEAD;

typedef struct _T_MSG_INFO
{
    T_MSG_HEAD tMsgHead;
    char acMsgData[10240];
}T_MSG_INFO, *PT_MSG_INFO;

typedef struct _T_MSG_BROADCAST_QUREY
{
    char acFactory[32];
}T_MSG_BROADCAST_QUREY, *PT_MSG_BROADCAST_QUREY;

typedef struct _T_MSG_BROADCAST_DEV_INFO
{
    int iSessionId;                   /* 为一随机数，用来区分设备 */
  BYTE acDevType[24];
  BYTE IpAddr[20];
  BYTE NetMask[20];
  BYTE MacAddr[20];
} T_MSG_BROADCAST_DEV_INFO, *PT_MSG_BROADCAST_DEV_INFO;

typedef struct _T_MSG_DEV_PARAM
{
    BYTE acTrainNo[32];            //车号
    BYTE acUser[32];               // 用户名（相机）
    BYTE acPasswd[32];             // 密码
    BYTE cCarrageNo;               // NVR车厢号
    BYTE cChannelNum;              // IPC个数
    BYTE cHdiskNum;                // 硬盘个数
    BYTE cUnused;
    INT32 iVersionConf;   //       可配置版本
    INT32 iTimeZone;      //       时区

} T_MSG_DEV_PARAM, *PT_MSG_DEV_PARAM;

typedef struct _T_MSG_BOARD_INFO
{
    BYTE acFactoryName[32];
    BYTE acSerialNo[32];
    BYTE acVersion[32];   // 
    char cAgingTestEnable;  //  0:disable, 1:enable
    char unreserved[3];
}T_MSG_BOARD_INFO, *PT_MSG_BOARD_INFO;

/* 网卡信息 */
typedef struct _T_NET_INFO
{
    char acEthName[8];
    char acIPAddr[20];
    char acNetMask[20];
    char acGateWay[20];
    char acMacAddr[20];
    char acDNS1Addr[20];   /* DNS1地址 */
} T_NET_INFO, *PT_NET_INFO;

typedef struct _T_IPC_INFO_MACRO_SET
{
    char cCh;             // 1-16
    char cStreamMode;     // 1: onvif, 2:rtsp
    char cCarriageNo;     // IPC车厢号
    char cPos;            // 位置号
    char acIpcAddr[20];   // IPC地址
    char acRtspAddr[128]; // RTSP地址
    char acChName[32];    // 通道名称
}T_IPC_INFO_MACRO_SET;

typedef struct _T_DEV_IPC_INFO
{
    char cIpcNum;
    T_IPC_INFO_MACRO_SET tIpcInfo[1];
} T_DEV_IPC_INFO;

typedef struct _T_FILE_INFO
{
    int iFileLen;
    char acFileName[128];
}T_FILE_INFO, *PT_FILE_INFO;


typedef struct _T_MSG_UPDATE_SYS
{
    T_FILE_INFO tFile;
}T_MSG_UPDATE_SYS, *PT_MSG_UPDATE_SYS;

typedef struct _T_HDISK_INFO
{
    char acHdName[32];
    int iHdTotalSize;    // GByte
    int iHdFreeSize;     // GByte
    char cHdiskStatus;   // 0:ok, 1:formating, 2:error
    char unused[3];
} T_HDISK_INFO, *PT_HDISK_INFO;

typedef struct _T_HDISK_DATA
{
    int iHdiskNum;
    T_HDISK_INFO atHdiskInfo[4];
} T_HDISK_DATA, *PT_HDISK_DATA;

typedef struct _T_MSG_STATUS
{
    T_HDISK_DATA tHdiskData;
}T_MSG_STATUS, *PT_MSG_STATUS;

typedef struct _T_MSG_HDISK_FORMAT
{
    char acHdiskName[12];
    char cHdiskPartNO;
    char unused[3];
} T_MSG_HDISK_FORMAT;


#pragma pack()


#define  KEY_FCT_FILE_PATH  (".\\ConfigInfo.ini")
#define  KEY_FCT_CATEGORY ("ConfigInfo")
#define  KEY_FCT_KEY  ("Name")

#define MSG_BROADCAST_PORT  11003
#define MSG_TCP_PORT        11004


#define  SOCKET_INITIATION_FAIL  (-1001)
#define  SOCKET_IP_INITIATION_FAIL  (-1002)
#define  SOCKET_CONNECTION_FAIL  (-1003)
#define  SOCKET_RESTART_SUCCESS  (-1004)
#define  OPENING_FILE_FAIL (-1005)
#define  GETING_FILEPATH_FAIL (-1006)
#define  SEND_MESSAGE_FAIL  (-1007)
#define  NET_IS_BUSYING  (-1008)
#define  SHOW_RECORDE_FILE_CONTENT  (-1010)

#endif


