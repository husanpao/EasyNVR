//
// Created by uma-pc001 on 2022/6/2.
//

#include "Ncnn.h"

Ncnn::Ncnn() {
    addWeight();
};

std::vector<NcnnObject> Ncnn::detect(const cv::Mat &bgr) {
    const float prob_threshold = 0.25f;
    const float nms_threshold = 0.45f;
    ncnn::Mat in_pad = resize(bgr);
    int i = 0;
    for (auto net: this->weights) {
        ncnn::Extractor ex = net->create_extractor();
        ex.set_num_threads(1);
        ex.input("in0", in_pad);
        std::vector<NcnnObject> proposals;
        // anchor setting from yolov5/models/yolov5s.yaml

        // stride 8
        {
            ncnn::Mat out;
            ex.extract("out0", out);
            ncnn::Mat anchors(6);
            anchors[0] = 10.f;
            anchors[1] = 13.f;
            anchors[2] = 16.f;
            anchors[3] = 30.f;
            anchors[4] = 33.f;
            anchors[5] = 23.f;

            std::vector<NcnnObject> objects8;
            generate_proposals(anchors, 8, in_pad, out, prob_threshold, objects8);

            proposals.insert(proposals.end(), objects8.begin(), objects8.end());
        }
        // stride 16
        {
            ncnn::Mat out;
            ex.extract("out1", out);
            ncnn::Mat anchors(6);
            anchors[0] = 30.f;
            anchors[1] = 61.f;
            anchors[2] = 62.f;
            anchors[3] = 45.f;
            anchors[4] = 59.f;
            anchors[5] = 119.f;
            std::vector<NcnnObject> objects16;
            generate_proposals(anchors, 16, in_pad, out, prob_threshold, objects16);
            proposals.insert(proposals.end(), objects16.begin(), objects16.end());
        }
        // stride 32
        {
            ncnn::Mat out;
            ex.extract("out2", out);

            ncnn::Mat anchors(6);
            anchors[0] = 116.f;
            anchors[1] = 90.f;
            anchors[2] = 156.f;
            anchors[3] = 198.f;
            anchors[4] = 373.f;
            anchors[5] = 326.f;

            std::vector<NcnnObject> objects32;
            generate_proposals(anchors, 32, in_pad, out, prob_threshold, objects32);

            proposals.insert(proposals.end(), objects32.begin(), objects32.end());
        }
        qsort_descent_inplace(proposals);
        std::vector<int> picked;
        nms_sorted_bboxes(proposals, picked, nms_threshold);
        int count = picked.size();
        SPDLOG_INFO("{}:{}", i++, count);
    }
    return {};
}

void
Ncnn::nms_sorted_bboxes(const std::vector<NcnnObject> &faceobjects, std::vector<int> &picked, float nms_threshold) {
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++) {
        areas[i] = faceobjects[i].rect.area();
    }

    for (int i = 0; i < n; i++) {
        const NcnnObject &a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int) picked.size(); j++) {
            const NcnnObject &b = faceobjects[picked[j]];

            // intersection over union
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            // float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}

float Ncnn::intersection_area(const NcnnObject &a, const NcnnObject &b) {
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

void Ncnn::qsort_descent_inplace(std::vector<NcnnObject> &faceobjects) {
    if (faceobjects.empty())
        return;

    qsort_descent_inplace(faceobjects, 0, faceobjects.size() - 1);
}

void Ncnn::qsort_descent_inplace(std::vector<NcnnObject> &faceobjects, int left, int right) {
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j) {
        while (faceobjects[i].prob > p)
            i++;
        while (faceobjects[j].prob < p)
            j--;
        if (i <= j) {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);
            i++;
            j--;
        }
    }

#pragma omp parallel sections
    {
#pragma omp section
        {
            if (left < j) qsort_descent_inplace(faceobjects, left, j);
        }
#pragma omp section
        {
            if (i < right) qsort_descent_inplace(faceobjects, i, right);
        }
    }
}


float Ncnn::sigmoid(float x) {
    return static_cast<float>(1.f / (1.f + exp(-x)));
}

void Ncnn::generate_proposals(const ncnn::Mat &anchors, int stride, const ncnn::Mat &in_pad, const ncnn::Mat &feat_blob,
                              float prob_threshold, std::vector<NcnnObject> &objects) {
    const int num_grid_x = feat_blob.w;
    const int num_grid_y = feat_blob.h;

    const int num_anchors = anchors.w / 2;

    const int num_class = feat_blob.c / num_anchors - 5;

    const int feat_offset = num_class + 5;

    for (int q = 0; q < num_anchors; q++) {
        const float anchor_w = anchors[q * 2];
        const float anchor_h = anchors[q * 2 + 1];

        for (int i = 0; i < num_grid_y; i++) {
            for (int j = 0; j < num_grid_x; j++) {
                // find class index with max class score
                int class_index = 0;
                float class_score = -FLT_MAX;
                for (int k = 0; k < num_class; k++) {
                    float score = feat_blob.channel(q * feat_offset + 5 + k).row(i)[j];
                    if (score > class_score) {
                        class_index = k;
                        class_score = score;
                    }
                }

                float box_score = feat_blob.channel(q * feat_offset + 4).row(i)[j];

                float confidence = sigmoid(box_score) * sigmoid(class_score);

                if (confidence >= prob_threshold) {
                    // yolov5/models/yolo.py Detect forward
                    // y = x[i].sigmoid()
                    // y[..., 0:2] = (y[..., 0:2] * 2. - 0.5 + self.grid[i].to(x[i].device)) * self.stride[i]  # xy
                    // y[..., 2:4] = (y[..., 2:4] * 2) ** 2 * self.anchor_grid[i]  # wh

                    float dx = sigmoid(feat_blob.channel(q * feat_offset + 0).row(i)[j]);
                    float dy = sigmoid(feat_blob.channel(q * feat_offset + 1).row(i)[j]);
                    float dw = sigmoid(feat_blob.channel(q * feat_offset + 2).row(i)[j]);
                    float dh = sigmoid(feat_blob.channel(q * feat_offset + 3).row(i)[j]);

                    float pb_cx = (dx * 2.f - 0.5f + j) * stride;
                    float pb_cy = (dy * 2.f - 0.5f + i) * stride;

                    float pb_w = pow(dw * 2.f, 2) * anchor_w;
                    float pb_h = pow(dh * 2.f, 2) * anchor_h;

                    float x0 = pb_cx - pb_w * 0.5f;
                    float y0 = pb_cy - pb_h * 0.5f;
                    float x1 = pb_cx + pb_w * 0.5f;
                    float y1 = pb_cy + pb_h * 0.5f;

                    NcnnObject obj;
                    obj.rect.x = x0;
                    obj.rect.y = y0;
                    obj.rect.width = x1 - x0;
                    obj.rect.height = y1 - y0;
                    obj.label = class_index;
                    obj.prob = confidence;

                    objects.push_back(obj);
                }
            }
        }
    }
}

ncnn::Mat Ncnn::resize(const cv::Mat &bgr) {
    const int target_size = 640;
    int img_w = bgr.cols;
    int img_h = bgr.rows;
    const int max_stride = 64;
    int w = img_w;
    int h = img_h;
    float scale = 1.f;
    if (w > h) {
        scale = (float) target_size / w;
        w = target_size;
        h = h * scale;
    } else {
        scale = (float) target_size / h;
        h = target_size;
        w = w * scale;
    }
    ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR2RGB, img_w, img_h, w, h);
    int wpad = (w + max_stride - 1) / max_stride * max_stride - w;
    int hpad = (h + max_stride - 1) / max_stride * max_stride - h;
    ncnn::Mat in_pad;
    ncnn::copy_make_border(in, in_pad, hpad / 2, hpad - hpad / 2, wpad / 2, wpad - wpad / 2, ncnn::BORDER_CONSTANT,
                           114.f);
    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in_pad.substract_mean_normalize(0, norm_vals);
    return in_pad;
}

void Ncnn::addWeight() {
    ncnn::Net *net = new ncnn::Net();
    net->opt.use_vulkan_compute = true;

    net->load_param("yolov5s.ncnn.param");
    net->load_model("yolov5s.ncnn.bin");
    ncnn::Net *net2 = new ncnn::Net();
    net2->opt.use_vulkan_compute = true;

    net2->load_param("helmet.ncnn.param");
    net2->load_model("helmet.ncnn.bin");

    this->weights.push_back(net);
    this->weights.push_back(net2);
}