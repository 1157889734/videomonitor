#include "FisheyeCorrect.h"
#include  <stdlib.h>

//#pragma comment(lib, "..\\lib\\dhplay.lib")
int InitFisheye(Fe_Handle *pInstance,Fe_Int8 **pCorrect_data,int src_width, int src_height,int dst_width, int dst_height)
{
       //鱼眼矫正库相关
    FISHEYE_StartParam initParam;
    FISHEYE_OPTPARAM  optParam;
    FISHEYE_OUTPUTFORMAT outputParam;
    int ret = 0;

    //鱼眼矫正输入输出图内存
   // Fe_Int32 src_img_size = src_height * src_width;
    Fe_Int32 dst_img_size = dst_height * dst_width;
    //外部申请输出图像内存，需要多申请一点
    *pCorrect_data = (Fe_Int8 *)malloc(3 * dst_img_size*sizeof(Fe_Int32));


    memset(&initParam, 0, sizeof(initParam));
    initParam.nStartType = 0;
    initParam.nStartType = 0;
    initParam.nfuncType = 0;
    initParam.nWidthOut = dst_width; 
    initParam.nHeightOut = dst_height;
    initParam.nWidth = src_width;
    initParam.nHeight = src_height;

    PLAY_CreateInstance(PLAYSDK_FISHEYE, pInstance);
    if (*pInstance == 0)
    {
        return 0;
    }
    IVideoAlgorithm* pFishEye = (IVideoAlgorithm*)*pInstance;
    pFishEye->Start(&initParam, 0);

    //默认下的模式设置
    {
        int radius = 4096,	origin_x = 4096,	origin_y = 4096;
        //鱼眼镜头旋转角度，eptz窗口编号、展开中心、缩放步幅
        int lens_direction = 0;
        optParam.originX = origin_x;
        optParam.originY = origin_y;
        optParam.radius  = radius;
        optParam.lensDirection = lens_direction;
        optParam.mainMountMode = FISHEYEMOUNT_MODE_CEIL;
        optParam.mainCalibrateMode = FISHEYECALIBRATE_MODE_FOUR_EPTZ_REGION;
        outputParam.mainShowSize.w = dst_width;
        outputParam.mainShowSize.h = dst_height;
        //在顶装地装1F+2、1F+4，壁装1PF+4窗口叠加模式下必须配置floatMainShowSize，不可大于dst_width/2 * dst_width/2
        if (optParam.mainCalibrateMode == FISHEYECALIBRATE_MODE_TWO_EPTZ_REGION_WITH_ORIGINAL ||
            optParam.mainCalibrateMode == FISHEYECALIBRATE_MODE_FOUR_EPTZ_REGION_WITH_ORIGINAL ||
            optParam.mainCalibrateMode == FISHEYECALIBRATE_MODE_FOUR_EPTZ_REGION_WITH_PANORAMA)
        {
            outputParam.floatMainShowSize.w = dst_width/3;
            outputParam.floatMainShowSize.h = dst_width/3;
        }
        optParam.mainStreamSize.w = src_width; //如果是主码流;
        optParam.mainStreamSize.h = src_height; //;
        optParam.enableAutoContrast = 0;
        optParam.outputFormat = &outputParam;
        optParam.modeInitParam.useRegionParam = 0;
        optParam.captureSize.w = 0;
        optParam.captureSize.h = 0;
        ret = pFishEye->SetParams(FISHEYE_SetParam, &optParam, 0);
    }
    return 1;
}

void FishCorrect(Fe_Handle hInstance, Fe_Int8 **pCorrect_data,LPBYTE pY, LPBYTE pU, LPBYTE pV, INT32 iYstride, INT32 iUVstride,
    INT32 iWidth, INT32 iHeight, INT32 &iOutWidth, INT32 &ioutHeight)
{
    DEC_OUTPUT_PARAM fr_src;
    DEC_OUTPUT_PARAM fr_dst;
    fr_src.data[0] = pY; //y通道
    fr_src.data[1] = pU; 
    fr_src.data[2] = pV;
#ifdef YUV420SP
    fr_src.data[2] = yuv + src_img_size + 1;
#endif
    fr_src.nWidth[0] = iWidth;
    fr_src.nWidth[1] = iWidth>>1;
    fr_src.nWidth[2] = iWidth>>1;
    fr_src.nHeight[0] =iHeight;
    fr_src.nHeight[1] =iHeight>>1;
    fr_src.nHeight[2] =iHeight>>1;
    fr_src.stride[0] = iYstride;
    fr_src.stride[1] = iUVstride;
    fr_src.stride[2] = iUVstride;
#ifdef YUV420SP
    fr_src.nWidth[1] = src_width;
    fr_src.nWidth[2] = src_width;
    fr_src.stride[1] = src_width;
    fr_src.stride[2] = src_width;
#endif

    fr_dst.data[0] = *pCorrect_data;
    IVideoAlgorithm* pFishEye = (IVideoAlgorithm *)hInstance;

    if (NULL == pFishEye)
    {
        return;
    }

    pFishEye->Process(&fr_src, &fr_dst);
    iOutWidth = fr_dst.stride[0];
    ioutHeight = fr_dst.nHeight[0];
}


void InitFishsyePTZ(Fe_Handle hInstance)
{
    FISHEYE_EPTZPARAM  eptzParam;
    IVideoAlgorithm* pFishEye = (IVideoAlgorithm*)hInstance;

    if (NULL == pFishEye)
    {
        return;
    }

    /*eptzParam.arg1 = 0x02;
    eptzParam.winId = 1;
    eptzParam.ePtzCmd = FISHEYEEPTZ_CMD_GET_CUR_REGION;
    pFishEye->SetParams(FISHEYE_Eptz, &eptzParam, 0);
    eptzParam.arg2 = eptzParam.arg2+2300;
    eptzParam.arg3 = eptzParam.arg3+3200;
    eptzParam.ePtzCmd =FISHEYEEPTZ_CMD_SET_CUR_REGION;
    pFishEye->SetParams(FISHEYE_Eptz, &eptzParam, 0);

    eptzParam.arg1 = 0x2;
    eptzParam.winId = 2;
    eptzParam.ePtzCmd = FISHEYEEPTZ_CMD_GET_CUR_REGION;
    pFishEye->SetParams(FISHEYE_Eptz, &eptzParam, 0);
    eptzParam.arg2 = eptzParam.arg2-1500;
    eptzParam.arg3 = eptzParam.arg3-3011;
    eptzParam.ePtzCmd =FISHEYEEPTZ_CMD_SET_CUR_REGION;
    pFishEye->SetParams(FISHEYE_Eptz, &eptzParam, 0);*/

}

void ReleaseFisheyeSource(Fe_Handle hInstance,Fe_Int8 *pCorrect_data)
{
    IVideoAlgorithm* pFishEye = (IVideoAlgorithm*)hInstance;

    if (pFishEye)
    {
        pFishEye->Stop();
        PLAY_Release(pFishEye);
    }

    if(pCorrect_data)
    {
        free(pCorrect_data);
        pCorrect_data = NULL;
    }
   

}
