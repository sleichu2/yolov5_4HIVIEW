//
// Created by bruce on 2020/8/28.
//

#pragma once

#ifdef __cplusplus

#include "hi_type.h"
#include "sample_comm_nnie.h"

extern "C"
{
#endif /* __cplusplus */
HI_S32 SAMPLE_SVP_NNIE_Yolov3_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
                                     SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S *pstSoftWareParam,
                                     SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel);

HI_S32 SAMPLE_SVP_NNIE_Forward(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
                               SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S *pstInputDataIdx,
                               SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S *pstProcSegIdx, HI_BOOL bInstant);

HI_S32 SAMPLE_SVP_NNIE_FillSrcData_FromMem(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
                                           SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
                                           SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S *pstInputDataIdx,
                                           HI_U8 *resizedPicAddr);

/******************************************************************************
* function : Yolov3 software deinit
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit(SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam);

#ifdef __cplusplus
}


#endif /* __cplusplus */