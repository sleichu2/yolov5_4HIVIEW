//
// Created by rikka on 2021/10/19.
//

#include "Yolov5.h"
using namespace std;
int YOLOV5C::init(char *model_path)
{
    stSoftwareParam.u32ClassNum = stModel.stModel.astSeg[0].astDstNode[0].unShape.stWhc.u32Width - 5;
    stSoftwareParam.u32MaxRoiNum = _maxRoiNum;
    stNnieCfg.pszPic = nullptr;
    stNnieCfg.u32MaxInputNum = 1; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0;

    char *pcModelName = const_cast<char *>(model_path);
    auto ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName, &stModel);
    if (ret != HI_SUCCESS)
        return ret;
    paramInit();
    stSoftwareParam.u32ClassNum = stModel.stModel.astSeg[0].astDstNode[0].unShape.stWhc.u32Width - 5;
    cout << "init successfully."<< endl;
    return 0;
}

int YOLOV5C::detect(cv::Mat &image, std::vector<BoxInfo> &bboxs, float threshold)
{
    objThreshold = threshold;
    unsigned char* imageStream;
    cv::Size offset;
    double sizeRatio;
    cvMat2Array(image, &imageStream, offset, sizeRatio);
    insideForward(imageStream);
    insideGetRes(bboxs);
    for(auto& box: bboxs)
    {
        box.box.x -= offset.width;
        box.box.y -= offset.height;
        box.box.x = int(box.box.x / sizeRatio);
        box.box.y = int(box.box.y / sizeRatio);
        box.box.width = int(box.box.width / sizeRatio);
        box.box.height = int(box.box.height / sizeRatio);
    }
    return 0;
}

int YOLOV5C::detect(unsigned char *imageStream, vector<BoxInfo> &bboxs, float threshold)
{
    struct timespec ts1, ts2;
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    objThreshold = threshold;
    insideForward(imageStream);
    insideGetRes(bboxs);
    for(auto& box: bboxs)
    {
        box.box.x = box.box.x < 0? 0: box.box.x;
        box.box.x = box.box.x > 415? 415: box.box.x;
        box.box.y = box.box.y < 0? 0: box.box.y;
        box.box.y = box.box.y > 415? 415: box.box.y;
        box.box.width = box.box.width < 0? 0: box.box.width;
        box.box.width = box.box.x + box.box.width > 415? 415 - box.box.x: box.box.width;
        box.box.height = box.box.height < 0? 0: box.box.height;
        box.box.height = box.box.y + box.box.height > 415? 415 - box.box.y: box.box.height;
        cout << box << endl;
    }
    clock_gettime(CLOCK_MONOTONIC, &ts2);
//    printf("yolov5 cost:%d ms\n", (ts2.tv_sec*1000 + ts2.tv_nsec/1000000) - (ts1.tv_sec*1000 + ts1.tv_nsec/1000000));
    return 0;
}
int YOLOV5C::destroy()
{
    HI_S32 s32Ret = HI_SUCCESS;
    auto pstNnieParam = &stNnieParam;
    auto pstSoftWareParam = &stSoftwareParam;
    auto pstNnieModel = &stModel;
    /*hardware deinit*/
    if(pstNnieParam != nullptr)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*software deinit*/
    if(pstSoftWareParam != nullptr)
    {
        s32Ret = SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit(pstSoftWareParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel != nullptr)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }
    return s32Ret;
}

int YOLOV5C::paramInit()
{
    stNnieParam.pstModel = &stModel.stModel;
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(&stNnieCfg, &stNnieParam);

    if(s32Ret != HI_SUCCESS)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(&stNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
        return -1;
    }
    return 0;
}

int YOLOV5C::insideForward(unsigned char *imageStream)
{
    int ret;
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;
    ret = SAMPLE_SVP_NNIE_FillSrcData_FromMem(&stNnieCfg, &stNnieParam, &stInputDataIdx, imageStream);
    if (ret != HI_SUCCESS)
        return ret;
    stProcSegIdx.u32SegIdx = 0;
    ret = SAMPLE_SVP_NNIE_Forward(&stNnieParam, &stInputDataIdx, &stProcSegIdx, HI_TRUE);
    if (ret != HI_SUCCESS)
        return ret;
    return 0;
}

int YOLOV5C::insideGetRes(vector<BoxInfo> &boxes)
{
/*
 * (0,3,7,169) -> (0,169,3,7)
 *
 */
    static int OutputNum = 3;
    static int cls = stSoftwareParam.u32ClassNum;
    int Side[] = {52, 26, 13};
    int channels = 3;
    int dataLen = cls + 5;
    float dataBlob[dataLen];
    int Offset = 0;
    int* cInputBlob[3];
    int Stride[3];
    float Scale[] = {8, 16, 32};
    float anchor[] = {10, 13, 16, 30, 33, 23, 30, 61, 62, 45, 59, 119, 116, 90, 156, 198, 373, 326};
    vector<vector<BoxInfo>> mBBox(cls, vector<BoxInfo>{});
    for(int i = 0; i < OutputNum; i ++)
    {
        cInputBlob[i] = SAMPLE_SVP_NNIE_CONVERT_64BIT_ADDR(HI_S32,stNnieParam.astSegData[0].astDst[i].u64VirAddr);
        Stride[i] = stNnieParam.astSegData[0].astDst[i].u32Stride;
    }
    for(int s = 0; s < OutputNum; s ++)
    {
        int ChnOffset = channels * Stride[s] / (int)sizeof(int);
        int HeightOffset = Stride[s] / (int)sizeof(int);
        for(int g = 0; g < Side[s] * Side[s]; g ++) // Traverse channel 2704 676 169
        {
            int gridIdX = g % Side[s];
            int gridIdY = g / Side[s];
            for(int h = 0; h < channels; h ++)
            {
                int* data = &cInputBlob[s][g * ChnOffset + h * HeightOffset];
                if(SAMPLE_SVP_NNIE_SIGMOID((HI_FLOAT)data[4] / SAMPLE_SVP_NNIE_QUANT_BASE) < ignThreshold)
                    continue;
                for(int i = 0; i < dataLen; i ++)
                {
                    dataBlob[i] = (HI_FLOAT)data[i] / SAMPLE_SVP_NNIE_QUANT_BASE;
                }
                unsigned int MaxValueIndex;
                auto cls_conf = SVP_NNIE_GetMaxVal(dataBlob + 5, cls, &MaxValueIndex);
                auto conf = SAMPLE_SVP_NNIE_SIGMOID(dataBlob[4]) * SAMPLE_SVP_NNIE_SIGMOID(cls_conf);
                if(conf > objThreshold)
                {
                    int x = int((SAMPLE_SVP_NNIE_SIGMOID(dataBlob[0]) * 2 - 0.5 + gridIdX) * Scale[s]);
                    int y = int((SAMPLE_SVP_NNIE_SIGMOID(dataBlob[1]) * 2 - 0.5 + gridIdY) * Scale[s]);
                    int width = int(pow(SAMPLE_SVP_NNIE_SIGMOID(dataBlob[2]) * 2, 2) * anchor[s * 6 + h * 2]);
                    int height = int(pow(SAMPLE_SVP_NNIE_SIGMOID(dataBlob[3]) * 2, 2) * anchor[s * 6 + h * 2 + 1]);
                    BoxInfo boxInfo;
                    boxInfo.score = conf;
                    boxInfo.label = (int)MaxValueIndex;
                    boxInfo.box = cv::Rect(int(x - 0.5 * width), int(y - 0.5 * height), width, height);
                    mBBox[MaxValueIndex].emplace_back(boxInfo);
                }
            }
        }
    }
    nms(mBBox, boxes);
    return 0;
}

static double get_iou_value(BoxInfo& rect1, BoxInfo& rect2)
{
    float xx1, yy1, xx2, yy2;

    xx1 = max(rect1.box.x, rect2.box.x);
    yy1 = max(rect1.box.y, rect2.box.y);
    xx2 = min(rect1.box.x + rect1.box.width - 1, rect2.box.x + rect2.box.width - 1);
    yy2 = min(rect1.box.y + rect1.box.height - 1, rect2.box.y + rect2.box.height - 1);
    float insection_width, insection_height;
    insection_width = fmax(0, xx2 - xx1 + 1);
    insection_height = fmax(0, yy2 - yy1 + 1);
    float insection_area, union_area;
    insection_area = insection_width * insection_height;
    union_area = rect1.box.width * rect1.box.height + rect2.box.width * rect2.box.height - insection_area;

    return double(insection_area) / union_area;
}

int YOLOV5C::nms(vector<std::vector<BoxInfo>> &mBBox, vector<BoxInfo> &uBoxes) const
{
    uBoxes.clear();
    uBoxes.shrink_to_fit();
    for(auto& boxes: mBBox)
    {
        if(boxes.empty())
            continue;
        sort(boxes.begin(), boxes.end(), [](BoxInfo& rect1, BoxInfo& rect2){return rect1.score > rect2.score;});
        int updated_size = boxes.size();
        for (int i = 0; i < updated_size; i++)
        {
            uBoxes.emplace_back(boxes[i]);
            for (int j = i + 1; j < updated_size; j++)
            {
                double iou = get_iou_value(boxes[i], boxes[j]);
                if (iou > nmsThreshold)
                {
                    boxes.erase(boxes.begin() + j);
                    j = j - 1;
                    updated_size = boxes.size();
                }
            }
        }
    }
    return 0;
}

static int letterBox(cv::Mat& img, cv::Size& dstSize, cv::Size& offset, double& r, const cv::Scalar& fill = cv::Scalar(114, 114, 114))
{
    int in_w = img.cols;
    int in_h = img.rows;
    int tar_w = dstSize.width;
    int tar_h = dstSize.height;
    r = min(double(tar_h) / in_h, double(tar_w) / in_w);
    int inside_w = (int)round(in_w * r);
    int inside_h = (int)round(in_h * r);
    int pad_w = tar_w - inside_w;
    int pad_h = tar_h - inside_h;
    //内层图像resize
    resize(img, img, cv::Size(inside_w, inside_h));

    pad_w = pad_w / 2;
    pad_h = pad_h / 2;
    offset = cv::Size(pad_w, pad_h);
    //外层边框填充灰色
    int top = int(round(pad_h - 0.1));
    int bottom = int(round(pad_h + 0.1));
    int left = int(round(pad_w - 0.1));
    int right = int(round(pad_w + 0.1));
    copyMakeBorder(img, img, top, bottom, left, right, cv::BORDER_CONSTANT, fill);
    return 0;
}

static int Mat2Array(cv::Mat& image, cv::Size& size, unsigned char* array)
{
    int channelsStride = size.width * size.height;
    for(int i = 0; i < image.rows; i ++)
    {
        for(int j = 0; j < image.cols; j ++)
        {
            array[channelsStride * 0 + i * image.cols + j] = image.at<cv::Vec3b>(i, j)[0];
            array[channelsStride * 1 + i * image.cols + j] = image.at<cv::Vec3b>(i, j)[1];
            array[channelsStride * 2 + i * image.cols + j] = image.at<cv::Vec3b>(i, j)[2];
        }
    }
}

int YOLOV5C::cvMat2Array(cv::Mat &input, unsigned char** data, cv::Size& offset, double& ratio)
{
    cv::Size dstSize(416,416);
    size_t fileSize = dstSize.width * dstSize.height * 3;
    static auto array = new unsigned char[fileSize];
    auto mat = input.clone();
    letterBox(mat,dstSize, offset,ratio);
    Mat2Array(mat, dstSize, array);
    *data = array;
    return 0;
}


ostream &operator<<(ostream &os, const BoxInfo &info)
{
    os << "label: " << info.label << " score: " << info.score << " box: " << info.box;
    return os;
}
