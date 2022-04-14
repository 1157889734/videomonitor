#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "types.h"
#include "pmsgcli.h"

int PmsgProc(PMSG_HANDLE PHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen)
{
   printf("cmd %x, len %d\n", ucMsgCmd, iMsgDataLen);
   switch(ucMsgCmd)
   {
       case SERV_CLI_MSG_TYPE_HEART:
           break;
       case SERV_CLI_MSG_TYPE_GET_IPC_STATUS_RESP:
           break;
       default:
           break;
   }
}

int main(int argc, char **argv)
{
    PMSG_HANDLE PHandle = 0;
    int iCount = 0;
    
    PMSG_Init();
    PHandle = PMSG_CreateConnect("192.168.103.81", 10100, PmsgProc);
    
    while (1)
    {
        // ·¢ËÍÐÄÌø
        if (0 == (iCount % 100))
        {
            PMSG_SendPmsgData(PHandle, CLI_SERV_MSG_TYPE_HEART, NULL, 0);
        }
        
        if (0 == (iCount % 500))
        {
            PMSG_SendPmsgData(PHandle, CLI_SERV_MSG_TYPE_GET_IPC_STATUS, NULL, 0);
        }
        
        iCount++;
        usleep(10000);
    }

    return 0;
}
