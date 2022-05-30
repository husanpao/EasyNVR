//
// Created by uma-pc001 on 2022/5/28.
//

#ifndef EASYNVR_CAMERAHANDLE_H
#define EASYNVR_CAMERAHANDLE_H

#include "CameraPull.h"
#include "YoloV5.h"

class CameraHandle {
public:
    ~CameraHandle();

    void startPrediction();

    CameraHandle(string id, string url);

private:
    CameraPull *cameraPull;//拉取器
    string url;
    string id;
};


#endif //EASYNVR_CAMERAHANDLE_H
