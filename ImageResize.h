//
// Created by rikka on 2020/10/23.
//
#pragma once
#include <opencv2/opencv.hpp>
class ImageResize
{
public:
    explicit ImageResize(cv::Size size, char* mode=(char*)"fix");
    int doResize(cv::Mat& src, cv::Mat& dst);
    int transScale(int& left, int& right, int& top, int& bottom);
private:
    double _scaleW, _scaleH; // scale of w, h
    cv::Size _size;
    double dx, dy;
    int pad_top, pad_bot, pad_left, pad_right;
};
