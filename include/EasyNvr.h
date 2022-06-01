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

using namespace std;
struct Label {
    string name;
    bool render;
    string text;
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
    Label weight;
    float hold;
    int event;
    int left;
    int right;
    int top;
    int bottom;
};

class LabelClass {
public:
    string name;
    bool render;
    string text;
    float threshold;
    bool flag;
};

class EventClass {
public:
    LabelClass *weight;
    float hold;
    int event;
    int left;
    int right;
    int top;
    int bottom;
};

#endif //EASYNVR_EASYNVR_H
