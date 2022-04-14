#ifndef _FILE_FISHEYE_H_
#define _FILE_FISHEYE_H_
#ifdef _WIN32

#include "dhplay.h"
#include <InitGuid.h>

/*�������Ͷ���*/
#define FISHEYE_ERROR_MAP_AVAILABLE      2         /**< map��Ч*/ 
#define FISHEYE_ERROR_ALG_RUN            1         /**< �㷨������*/ 
#define FISHEYE_ERROR_NONE               0         /**< �ɹ�*/
#define FISHEYE_ERROR_UNKNOWN           -1         /**< ʧ��*/
#define FISHEYE_ERROR_PARA_NULL         -2         /**< ����ΪNULL*/
#define FISHEYE_ERROR_MEM_NULL          -3         /**< �����ڴ�Ϊ��*/
#define FISHEYE_ERROR_PARA_VAL          -4         /**< ����Ĳ���ֵ*/
#define FISHEYE_ERROR_MEM_ALLOC         -5         /**< �ڴ����ʧ��*/ 
#define FISHEYE_ERROR_OPS_SUPPORT       -6         /**< ��֧�ָò���*/ 
#define FISHEYE_ERROR_MAP_UNAVAILABLE   -7         /**< map��Ч*/ 
#define FISHEYE_ERROR_FPTZ_FAIL         -8         /**< ����������ʧ��*/ 

typedef enum
{
	FISHEYE_SetParam,
	FISHEYE_GetParam,		   
	FISHEYE_Eptz
}FISHEYE_ParamCmd;

typedef struct FISHEYE_StartParam
{
	int nWidth;		 /*IN ����ͼ��ֱ���*/
	int nHeight;
	int nStartType;
	int nfuncType;
	void* ptzChannelParam;
	int	  ptzChannelNum;
	int nWidthOut;   /*IN ���ͼ�����ķֱ���*/
	int nHeightOut;
} FISHEYE_StartParam;

typedef struct DEC_OUTPUT_PARAM
{
	unsigned char*	data[3];		/**< yuv����ָ��*/
	int		stride[3];				/**< yuv���*/
	int		nWidth[3];				/**< ֡��*/
	int		nHeight[3];				/**< ֡��*/
}DEC_OUTPUT_PARAM;

class IVideoAlgorithm
{
public:
	virtual ~IVideoAlgorithm() {}

	virtual int IsStart() = 0;

	virtual int Start(void * wParam/*FISHEYE_StartParam*/, void* lParam/*0*/) = 0;

	virtual int Process(DEC_OUTPUT_PARAM* pSrc, DEC_OUTPUT_PARAM* pDest) = 0;

	virtual int Stop() = 0;

	virtual int SetParams(int paramCmd/*FISHEYE_ParamCmd*/, void * wParam, void* lParam) = 0;

	virtual int Reset(int nWidth, int nHeight, int nStride) = 0;
};

// {E7689DE3-968D-4791-916C-B4722517A955}
DEFINE_GUID(PLAYSDK_FISHEYE, 0xe7689de3, 0x968d, 0x4791, 0x91, 0x6c, 0xb4, 0x72, 0x25, 0x17, 0xa9, 0x55);

PLAYSDK_API int CALLMETHOD PLAY_CreateInstance(__in  REFGUID Guid, __out void ** ppInstance);
PLAYSDK_API void CALLMETHOD PLAY_Release(void * pInstance);

#endif // _WIN32
#endif // _FILE_FISHEYE_H_