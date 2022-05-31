//
// Created by uma-pc001 on 2022/5/28.
//

#ifndef EASYNVR_CAMERAHANDLE_H
#define EASYNVR_CAMERAHANDLE_H

#include "CameraPull.h"
#include "YoloV5.h"
#include "PluginManager.h"

class CameraHandle {
public:
    ~CameraHandle();

    void startPrediction();

    CameraHandle(string id, string url);

private:
    void Handle(cv::Mat frame);

    luakit::lua_table formatEvent(vector<Event> events, luakit::kit_state lua);

private:
    string url;
    string id;
    bool prediction;
    bool render;
    int frameCount;//帧数

    CameraPull *cameraPull;//拉取器
    YoloV5 *yolo;
    set<string> algorithm_list;

    map<string, Plugin *> plugins;
};


#endif //EASYNVR_CAMERAHANDLE_H
