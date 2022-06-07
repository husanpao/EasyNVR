//
// Created by uma-pc001 on 2022/5/27.
//

#ifndef EASYNVR_EASYNVR_H
#define EASYNVR_EASYNVR_H
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE//必须定义这个宏,才能输出文件名和行号

#include <iostream>
#include <rapidjson/document.h>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "spdlog/spdlog.h"
#include "scope_guard.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;


struct Label {
    string en;
    bool render;
    string zh;
    float threshold;
    bool flag;
};
struct Weight {
    string name;
    float threshold;
    bool enable;
    unordered_map<int, Label> labels;
    string pt;
};
struct Event {
    float threshold;
    int label;
    string label_zh;
    string label_en;
    int x1;
    int y1;
    int x2;
    int y2;
};
struct NcnnObject {
    cv::Rect_<float> rect;
    int label;
    string label_zh;
    string label_en;
    float threshold;
};


#endif //EASYNVR_EASYNVR_H
