#ifndef _SAMPLE_SVP_USER_KERNEL_H_
#define _SAMPLE_SVP_USER_KERNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hi_comm_svp.h"
#include "hi_nnie.h"
#include "mpi_nnie.h"
#include "sample_comm_svp.h"
#include "sample_comm_nnie.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define SAMPLE_SVP_NNIE_SIGMOID(x)   (HI_FLOAT)(1.0f/(1+exp(-x)))

#define SAMPLE_SVP_NNIE_QUANT_BASE 4096    /*the base value*/

/*YOLOV2*/
HI_FLOAT SVP_NNIE_GetMaxVal(HI_FLOAT *pf32Val,HI_U32 u32Num,
                                          HI_U32 * pu32MaxValueIndex);

#ifdef __cplusplus
}
#endif

#endif /* _SAMPLE_SVP_USER_KERNEL_H_ */