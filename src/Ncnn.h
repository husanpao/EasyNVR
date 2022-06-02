//
// Created by uma-pc001 on 2022/6/2.
//

#ifndef EASYNVR_NCNN_H
#define EASYNVR_NCNN_H

#include "EasyNvr.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "ncnn/layer.h"
#include "ncnn/net.h"


class Ncnn {

public:

    Ncnn();

    std::vector<NcnnObject> detect(const cv::Mat &bgr);


private:
    ncnn::Mat resize(const cv::Mat &bgr);

    void addWeight();

    void generate_proposals(const ncnn::Mat &anchors, int stride, const ncnn::Mat &in_pad, const ncnn::Mat &feat_blob,
                            float prob_threshold, std::vector<NcnnObject> &objects);

    float sigmoid(float x);

    void qsort_descent_inplace(std::vector<NcnnObject> &faceobjects);

    void qsort_descent_inplace(std::vector<NcnnObject> &faceobjects, int left, int right);

    void nms_sorted_bboxes(const std::vector<NcnnObject> &faceobjects, std::vector<int> &picked, float nms_threshold);

    float intersection_area(const NcnnObject &a, const NcnnObject &b);

private:
    vector<ncnn::Net *> weights;

};


#endif //EASYNVR_NCNN_H
