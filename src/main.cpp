
#include "EasyNvr.h"
#include "PluginManager.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>


void initLog() {
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
    return 0;
}
