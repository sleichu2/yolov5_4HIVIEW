//
// Created by rikka on 2020/10/23.
//

#include "ImageResize.h"
#include <bits/stdc++.h>
#include <utility>
using namespace std;
ImageResize::ImageResize(cv::Size size, char* mode)
{
    _size = size;
    dx = size.width;
    dy = size.height;
}

int ImageResize::doResize(cv::Mat &src, cv::Mat &dst)
{
    _scaleW = src.cols * 1.0 / _size.width;
    _scaleH = src.rows * 1.0 / _size.height;
    auto scale = fmax(_scaleH, _scaleW);
//    dst = cv::Mat(_size, CV_8UC3, cv::Scalar(128, 128, 128));
    cv::Mat temp;
//    src += cv::Scalar(32, 32, 32);
    cv::resize(src, temp,cv::Size(0,0), 1/scale, 1 / scale);
    pad_top = int(fmax((_size.height - temp.rows) / 2,  0));
    pad_bot = int(fmax((_size.height - temp.rows - pad_top),  0));
    pad_left = int(fmax((_size.width - temp.cols) / 2,  0));
    pad_right = int(fmax((_size.width - temp.cols - pad_left),  0));
//    cv::copyTo()
    cv::copyMakeBorder(temp, dst, pad_top, pad_bot, pad_left, pad_right, cv::BORDER_CONSTANT, cv::Scalar(128,128,128));
    cv::resize(dst, dst,cv::Size(_size.width,_size.height));
    return 0;
}

int ImageResize::transScale(int &left, int &right, int &top, int &bottom)
{
    auto scale = fmax(_scaleH, _scaleW);
    left = max(0, left - pad_left);
    right = max(left, right - pad_left);
    top = max(0, top - pad_top);
    bottom = max(top, bottom - pad_top);
    left = int(left * 1.0 * scale);
    right = int(right * 1.0 * scale);
    top = int(top * 1.0 * scale);
    bottom = int(bottom * 1.0 * scale);
    return 0;
}
