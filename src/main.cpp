
#include "EasyNvr.h"
#include "PluginManager.h"
#include "CameraHandle.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>

void FFMPEG_LOG(void *ptr, int level, const char *fmt, va_list vl) {
    if (level < AV_LOG_WARNING) {
        char line[1024];
        static int print_prefix = 1;
        va_list vl2;
        va_copy(vl2, vl);
        av_log_format_line(ptr, level, fmt, vl2, line, sizeof(line), &print_prefix);
                va_end(vl2);
        SPDLOG_ERROR("FFMPEG :{}", line);
    }
}

void initLog() {
    av_log_set_callback(FFMPEG_LOG);  // 设置自定义的日志输出方法
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
    sinks.push_back(
            std::make_shared<spdlog::sinks::daily_file_sink_st>("logs/EasyNvr.log", 00, 01));
    auto logger = std::make_shared<spdlog::logger>("name", begin(sinks), end(sinks));
    spdlog::register_logger(logger);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%s:%#] %v ");
    logger->set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);
    spdlog::set_default_logger(logger);
}

int main(int argc, char *argv[]) {
    initLog();
    PluginManager::GetInStance();
//    CameraHandle *camera = new CameraHandle("1000", "rtsp://127.0.0.1:554/live/test/");
//    camera->startPrediction();
    return 0;
}
