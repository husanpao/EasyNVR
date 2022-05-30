//
// Created by UMA-PC-001 on 2022/2/17.
//

#include "CameraPull.h"

CameraPull::~CameraPull() {
    av_packet_free(&videoPacket);
    avformat_close_input(&fmtContext);
    SPDLOG_INFO("[{}] ~CameraPull", this->id);
}

CameraPull::CameraPull(string url, string id) : url(url), id(id) {
    this->flag = false;
    this->streaminfo.url = url;
//    this->streaminfo.url = "D:\\static\\image\\20220311\\smoke.mp4";
    this->streaminfo.id = id;
    this->fmtContext = avformat_alloc_context();
    this->videoPacket = av_packet_alloc();
    this->frame = av_frame_alloc();
    this->videoIdx = -1;
    this->width = 1280;
    this->height = 720;
}

int CameraPull::start() {
    avformat_network_init();
    AVDictionary *opts = NULL;
    if (avformat_open_input(&this->fmtContext, this->url.c_str(), NULL, &opts) < 0) {
        SPDLOG_ERROR("[{}] avformat_open_input open error", this->id);
        return -1;
    }
    //从媒体文件中读包进而获取流消息
    if (avformat_find_stream_info(this->fmtContext, nullptr) < 0) {
        SPDLOG_ERROR("[{}] avformat_find_stream_info  error", this->id);
        return -1;
    }
    for (int i = 0; i < this->fmtContext->nb_streams; i++) {
        this->video = this->fmtContext->streams[i];
        //筛选视频流和音频流
        if (this->video->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            this->videoIdx = i;
            this->streaminfo.height = height;
            this->streaminfo.width = width;
            this->streaminfo.fps = 25;
            break;
        }
    }
    if (this->videoIdx == -1) {
        SPDLOG_ERROR("[{}] Didn't find a video stream", this->id);
        return -1;
    }
    //找到对应的解码器
    this->videoCodec = avcodec_find_decoder(this->video->codecpar->codec_id);
    //创建解码器对应的结构体
    this->videoCodecContext = avcodec_alloc_context3(this->videoCodec);
    ret = avcodec_parameters_to_context(this->videoCodecContext, this->video->codecpar);
    if (ret < 0) {
        SPDLOG_ERROR("[{}]  Failed to copy in_stream codecpar to codec context", this->id);
        return -1;
    }
    if (avcodec_open2(this->videoCodecContext, this->videoCodec, NULL) < 0) {
        SPDLOG_ERROR("[{}] Could not open codec", this->id);
        return -1;
    }
    SPDLOG_INFO("[{}] start pull stream on {}", this->id, this->url);
    this->flag = true;
    while (this->flag) {
        // 6.读取数据包
        ret = av_read_frame(this->fmtContext, this->videoPacket);
        if (ret < 0) {
            while (!queue.empty()) {
                queue.pop();
                cv::waitKey(100);
            }
            SPDLOG_INFO("[{}] av_read_frame error result:{} queue size:{}", this->id, ret, queue.size());
            break;
        }
        if (this->videoPacket->stream_index == this->videoIdx) {
            ret = avcodec_send_packet(this->videoCodecContext, this->videoPacket);
            if (ret < 0) {
                SPDLOG_ERROR("[{}] avcodec_send_packet", this->id);
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(this->videoCodecContext, this->frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }
                if (ret >= 0) {
                    m_mutex.lock();
                    if (queue.size() > 3 && !StrUtil::startswith(this->id, "offline")) {
                        queue.pop();
                    }
                    queue.push(format2Cv(this->frame, this->videoCodecContext->pix_fmt, this->width,
                                         this->height).clone());
                    m_mutex.unlock();
                    cv::waitKey(1);
                }
            }
        }
        av_packet_unref(this->videoPacket);
    }
    this->flag = false;
    SPDLOG_INFO("[{}] stop pull stream on {}", this->id, this->url);
    return 0;
}

bool CameraPull::stop() {
    this->flag = false;
    return true;
}

cv::Mat CameraPull::get() {
    cv::Mat out;
    m_mutex.lock();
    if (queue.size() > 0) {
        out = queue.front();
        queue.pop();
    }
    m_mutex.unlock();
    return out;
}

StreamInfo CameraPull::info() {
    return this->streaminfo;
}

cv::Mat CameraPull::format2Cv(const AVFrame *frame, AVPixelFormat format, int width, int height) {
    cv::Mat image(height, width, CV_8UC3);
    int cvLinesizes[1];
    cvLinesizes[0] = image.step1();
    switch (format) {
        case AV_PIX_FMT_YUVJ420P :
            format = AV_PIX_FMT_YUV420P;
            break;
        case AV_PIX_FMT_YUVJ422P  :
            format = AV_PIX_FMT_YUV422P;
            break;
        case AV_PIX_FMT_YUVJ444P   :
            format = AV_PIX_FMT_YUV444P;
            break;
        case AV_PIX_FMT_YUVJ440P :
            format = AV_PIX_FMT_YUV440P;
            break;
        default:
            break;
    }
    SwsContext *conversion = sws_getContext(frame->width, frame->height, format, width, height,
                                            AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    sws_scale(conversion, frame->data, frame->linesize, 0, frame->height, &image.data, cvLinesizes);
    sws_freeContext(conversion);
    return image;
}