#include "GlobleDefine.h"
#include<QDebug>

//定义所有车厢信息
CarriageInfo	g_atCarriages[MAX_CARRIAGE_NUM];
int				g_iCarriageNum = 8;
int				g_iTurnTime = 5;		//<=0，不轮询 30
int				g_iPlaybackSecond =180; //默认回放3分钟
 QMutex         mutex;


int			DownloadFileInfo::iDownloadedCount=0;
int         DownloadFileInfo::iDownloadedPos =0;

