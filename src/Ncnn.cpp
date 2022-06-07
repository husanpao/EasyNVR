//
// Created by uma-pc001 on 2022/6/2.
//


#include <opencv2/freetype.hpp>
#include "Ncnn.h"

Ncnn::Ncnn() {

};

static inline float sigmoid(float x) {
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
                    obj.threshold = confidence;

                    objects.push_back(obj);
                }
            }
        }
    }
}

void Ncnn::qsort_descent_inplace(std::vector<NcnnObject> &faceobjects, int left, int right) {
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].threshold;

    while (i <= j) {
        while (faceobjects[i].threshold > p)
            i++;

        while (faceobjects[j].threshold < p)
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

static inline float intersection_area(const NcnnObject &a, const NcnnObject &b) {
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
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

void Ncnn::qsort_descent_inplace(std::vector<NcnnObject> &faceobjects) {
    if (faceobjects.empty())
        return;
    qsort_descent_inplace(faceobjects, 0, faceobjects.size() - 1);
}

int Ncnn::Detect(const cv::Mat &bgr, std::vector<NcnnObject> &objects) {
    objects.clear();
    const int target_size = 640;
    const float prob_threshold = 0.25f;
    const float nms_threshold = 0.45f;

    int img_w = bgr.cols;
    int img_h = bgr.rows;

    // yolov5/models/common.py DetectMultiBackend
    const int max_stride = 64;

    // letterbox pad to multiple of max_stride
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

    // pad to target_size rectangle
    // yolov5/utils/datasets.py letterbox
    int wpad = (w + max_stride - 1) / max_stride * max_stride - w;
    int hpad = (h + max_stride - 1) / max_stride * max_stride - h;
    ncnn::Mat in_pad;
    ncnn::copy_make_border(in, in_pad, hpad / 2, hpad - hpad / 2, wpad / 2, wpad - wpad / 2, ncnn::BORDER_CONSTANT,
                           114.f);

    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in_pad.substract_mean_normalize(0, norm_vals);
    for (auto net: nets) {
        ncnn::Extractor ex = net.second.net->create_extractor();
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

        // sort all proposals by score from highest to lowest
        qsort_descent_inplace(proposals);

        // apply nms with nms_threshold
        std::vector<int> picked;
        nms_sorted_bboxes(proposals, picked, nms_threshold);
        int count = picked.size();
        for (int i = 0; i < count; i++) {
            NcnnObject &temp = proposals[picked[i]];

            // adjust offset to original unpadded
            float x0 = (temp.rect.x - (wpad / 2)) / scale;
            float y0 = (temp.rect.y - (hpad / 2)) / scale;
            float x1 = (temp.rect.x + temp.rect.width - (wpad / 2)) / scale;
            float y1 = (temp.rect.y + temp.rect.height - (hpad / 2)) / scale;

            // clip
            x0 = std::max(std::min(x0, (float) (img_w - 1)), 0.f);
            y0 = std::max(std::min(y0, (float) (img_h - 1)), 0.f);
            x1 = std::max(std::min(x1, (float) (img_w - 1)), 0.f);
            y1 = std::max(std::min(y1, (float) (img_h - 1)), 0.f);

            temp.rect.x = x0;
            temp.rect.y = y0;
            temp.rect.width = x1 - x0;
            temp.rect.height = y1 - y0;
            temp.label_zh = net.second.labels[temp.label].zh;
            temp.label_en = net.second.labels[temp.label].en;
            objects.push_back(temp);
        }

    }
    return 0;
}

#ifdef _WIN32

#include <windows.h>

string GbkToUtf8(const char *src_str) {
    int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
    wchar_t *wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    string strTemp = str;
    if (wstr) delete[] wstr;
    if (str) delete[] str;
    return strTemp;
}

string Utf8ToGbk(const char *src_str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
    wchar_t *wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char *szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    string strTemp(szGBK);
    if (wszGBK) delete[] wszGBK;
    if (szGBK) delete[] szGBK;
    return strTemp;
}

#else
#include <iconv.h>

int GbkToUtf8(char *str_str, size_t src_len, char *dst_str, size_t dst_len)
{
    iconv_t cd;
    char **pin = &str_str;
    char **pout = &dst_str;

    cd = iconv_open("utf8", "gbk");
    if (cd == 0)
        return -1;
    memset(dst_str, 0, dst_len);
    if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
        return -1;
    iconv_close(cd);
    *pout = '\0';

    return 0;
}

int Utf8ToGbk(char *src_str, size_t src_len, char *dst_str, size_t dst_len)
{
    iconv_t cd;
    char **pin = &src_str;
    char **pout = &dst_str;

    cd = iconv_open("gbk", "utf8");
    if (cd == 0)
        return -1;
    memset(dst_str, 0, dst_len);
    if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
        return -1;
    iconv_close(cd);
    *pout = '\0';

    return 0;
}


#endif

void Ncnn::Draw(cv::Mat bgr, const std::vector<NcnnObject> &objects, bool showProd) {
    for (size_t i = 0; i < objects.size(); i++) {
        const NcnnObject &obj = objects[i];
        cv::rectangle(bgr, obj.rect, cv::Scalar(0, 0, 255), 2);
        int baseLine = 0;
        auto label = GbkToUtf8(obj.label_zh.c_str());
        if (showProd) {
            label = GbkToUtf8(fmt::format("{0}_{1:.2f}", obj.label_zh, obj.threshold).c_str());
        }
        cv::Size label_size = cv::getTextSize(label, cv::FONT_HERSHEY_PLAIN, 0.7, 1,
                                              &baseLine);
        int x = obj.rect.x;
        int y = obj.rect.y - label_size.height - baseLine;
        if (y < 0)
            y = 0;
        if (x + label_size.width > bgr.cols)
            x = bgr.cols - label_size.width;
        cv::rectangle(bgr,
                      cv::Rect(cv::Point(x, y - baseLine * 2),
                               cv::Size(label_size.width + (showProd ? 23 : 0),
                                        label_size.height * 1.5 + baseLine * 2+2)),
                      cv::Scalar(0, 0, 255), -1);

        cv::Ptr<cv::freetype::FreeType2> ft2 = cv::freetype::createFreeType2();
        ft2->loadFontData("./simsun.ttc", 0);
        ft2->putText(bgr, label, cv::Point(x, y + label_size.height * 1.5), 20,
                     CV_RGB(204, 204, 204), -1, 8, true);
    }

}

void Ncnn::AddNet(const char *name, unordered_map<int, Label> labels, const char *param, const char *bin) {
    ncnn::Net *net = new ncnn::Net();
    net->opt.
            use_fp16_arithmetic = false;
    net->opt.
            use_vulkan_compute = true;
    net->load_param(param);
    net->load_model(bin);
    this->nets.insert({name, {net, labels}});
}