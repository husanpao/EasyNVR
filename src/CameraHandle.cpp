//
// Created by uma-pc001 on 2022/5/28.
//

#include "CameraHandle.h"

void CameraHandle::startPrediction() {

}

CameraHandle::CameraHandle(string id, string url) : url(url), id(id) {
    this->cameraPull = new CameraPull(this->url, id);
}

CameraHandle::~CameraHandle() {
    this->cameraPull->start();
}