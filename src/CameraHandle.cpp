//
// Created by uma-pc001 on 2022/5/28.
//

#include "CameraHandle.h"

void CameraHandle::startPrediction() {
    SPDLOG_INFO("startPrediction");
    this->cameraPull->start();
}

CameraHandle::CameraHandle(string id, string url) : url(url), id(id) {
    YoloV5 *yolo = new YoloV5(
            "[{\"path\":\"./weights/helmet\",\"name\":\"helmet\",\"label\":\"person,head,helmet\",\"similarity\":0.7,\"enable\":true}]",
            true);
    this->cameraPull = new CameraPull(this->url, id);
}

CameraHandle::~CameraHandle() {
    SPDLOG_INFO("~CameraHandle");
}