//
// Created by uma-pc001 on 2022/6/2.
//

#ifndef EASYNVR_NCNN_H
#define EASYNVR_NCNN_H

#include "EasyNvr.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <map>
#include "ncnn/layer.h"
#include "ncnn/net.h"


class Ncnn {
    struct MyNet {
        ncnn::Net *net;
        unordered_map<int, Label> labels;
    };
public:

    Ncnn();

    ~Ncnn();

    int Detect(const cv::Mat &bgr, std::vector<NcnnObject> &objects);

    void Draw(cv::Mat bgr, const std::vector<NcnnObject> &objects, bool showProd=false);

    void AddNet(const char *name, unordered_map<int, Label> labels, const char *param, const char *bin);

private:


    void generate_proposals(const ncnn::Mat &anchors, int stride, const ncnn::Mat &in_pad, const ncnn::Mat &feat_blob,
                            float prob_threshold, std::vector<NcnnObject> &objects);

    void qsort_descent_inplace(std::vector<NcnnObject> &faceobjects, int left, int right);

    void nms_sorted_bboxes(const std::vector<NcnnObject> &faceobjects, std::vector<int> &picked, float nms_threshold);

    void qsort_descent_inplace(std::vector<NcnnObject> &faceobjects);

private:
    std::map<string, MyNet> nets;
};


#endif //EASYNVR_NCNN_H
