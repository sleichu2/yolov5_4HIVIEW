//
// Created by rikka on 2021/10/19.
//

#pragma once

#include <ostream>
#include "hi_type.h"
#include "hi_nnie.h"
#include "sample_comm_nnie.h"
#include "sample_nnie.h"
#include "sample_svp_nnie_software.h"
#include "opencv2/opencv.hpp"
#include "ImageResize.h"

struct BoxInfo
{
    int label;
    float score;
    cv::Rect2f box;

    friend std::ostream &operator<<(std::ostream &os, const BoxInfo &info);

};

class YOLOV5C
{
public:
    int init(char* model_path);
    int detect(cv::Mat &image, std::vector<BoxInfo> &bboxs, float threshold=0.3);
    int detect(unsigned char* imageStream, std::vector<BoxInfo> &bboxs, float threshold=0.3);
    int destroy();
private:
    int paramInit();
    int insideForward(unsigned char *imageStream);
    int insideGetRes(std::vector<BoxInfo>& boxes);
    int nms(std::vector<std::vector<BoxInfo>>& mBBox, std::vector<BoxInfo>& uBoxes) const;
    int cvMat2Array(cv::Mat &input, unsigned char** data, cv::Size& offset, double& ratio);
    int _maxRoiNum = 100;
    float nmsThreshold = 0.3f;
    float objThreshold = 0.3f;
    float ignThreshold = 0.05f;
    SAMPLE_SVP_NNIE_CFG_S stNnieCfg = {};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {};
    SAMPLE_SVP_NNIE_MODEL_S stModel = {};
    SAMPLE_SVP_NNIE_PARAM_S stNnieParam = {};
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S stSoftwareParam = {};
};

