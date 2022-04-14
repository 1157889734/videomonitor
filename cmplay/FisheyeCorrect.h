
#ifndef _FISHEYE_H_
#define _FISHEYE_H_
typedef int Fe_Int32;
typedef unsigned char Fe_Int8;
typedef void* Fe_Handle;

#include "FileFishEye.h"

int InitFisheye(Fe_Handle *pInstance,Fe_Int8 **pCorrect_data,int src_width, int src_height,int dst_width, int dst_height);
void FishCorrect(Fe_Handle hInstance, Fe_Int8 **pCorrect_data,LPBYTE pY, LPBYTE pU, LPBYTE pV, INT32 iYstride, INT32 iUVstride, INT32 iWidth, INT32 iHeight, INT32 &iOutWidth, INT32 &ioutHeight);
void InitFishsyePTZ(Fe_Handle hInstance);
void ReleaseFisheyeSource(Fe_Handle hFisheye,Fe_Int8 *pCorrect_data);
#endif
