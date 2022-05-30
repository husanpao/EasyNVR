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

struct Weight {
    std::string name;
    double threshold;
    bool enable;
    std::unordered_map<int, std::string> labels;
    std::string model;
};
#endif //EASYNVR_EASYNVR_H
