//
// Created by uma-pc001 on 2022/5/28.
//

#ifndef EASYNVR_CAMERAHANDLE_H
#define EASYNVR_CAMERAHANDLE_H

#include "CameraPull.h"
#include "Ncnn.h"
#include "PluginManager.h"
#include"thread_pool.hpp"

class CameraHandle {
public:
    ~CameraHandle();

    void startPrediction();

    CameraHandle(string id, string url, string plugins);

private:
    void Handle(cv::Mat frame);

    void formatEvent(map<string, vector<Event>> classifyEvent, luakit::kit_state lua);

    void drawFrame(cv::Mat frame, Event event);

private:
    string url;
    string id;
    bool prediction;
    bool render;
    int frameCount;//帧数

    std::vector<NcnnObject> events;


    CameraPull *cameraPull;//拉取器
    Ncnn *ncnn;
    map<string, Plugin *> plugins;
};


#endif //EASYNVR_CAMERAHANDLE_H
