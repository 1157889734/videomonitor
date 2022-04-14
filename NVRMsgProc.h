#ifndef _NVR_MSG_PROC_H_
#define _NVR_MSG_PROC_H_

#include "./SDK/state/state.h"
#include "pmsgcli.h"
#include "debug.h"

typedef struct _T_CMD_PACKET
{
    int iMsgCmd;
    char *pData;
	int iDataLen;
}T_CMD_PACKET, *PT_CMD_PACKET;

int NVR_init(int iNvrNum);
int NVR_Uninit();
int NVR_Connect(int iNvrNo);
int NVR_DisConnect(int iNvrNo);
int NVR_ConnectStatus(int iNvrNo);
int NVR_DisConnectStatus(int iNvrNo);
int NVR_SendCmdInfo(int iNvrNo,int iCmd,char *pData,int iDataLen);
int NVR_GetFileInfo(int iNvrNo,T_CMD_PACKET *ptPkt);
int NVR_CleanFileInfo(int iNvrNo);
int NVR_GetFileQueueSize(int iNvrNo);
int NVR_GetConnectStatus(int iNvrNo);  //E_SERV_STATUS_CONNECT连接 其余非连接

#endif
