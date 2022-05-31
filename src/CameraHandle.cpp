//
// Created by uma-pc001 on 2022/5/28.
//

#include "CameraHandle.h"

luakit::lua_table CameraHandle::formatEvent(vector<Event> events, luakit::kit_state lua) {
    auto eventsTab = lua.new_table("events");
    int i = 0;
    for (Event event: events) {
        auto label = lua.new_table(fmt::format("label_{}", i).c_str());
        label.set("name", event.weight.name, "render", event.weight.render, "text", event.weight.text, "threshold",
                  event.weight.threshold, "flag", event.weight.flag);
        auto eventTab = lua.new_table(fmt::format("event_{}", i).c_str());
        eventTab.set("event", event.event, "left", event.left, "hold", event.hold, "right", event.right, "top",
                     event.top, "bottom", event.bottom, "weight", label);
        eventsTab.set(fmt::format("{}", i).c_str(), eventTab);
        i++;
    }
    return eventsTab;
}


void CameraHandle::Handle(cv::Mat frame) {
    if (frame.empty()) {
        return;
    }
    if (this->frameCount++ % 5 == 0) {
        const vector<Event> &events = this->yolo->prediction(frame, this->algorithm_list);
        for (auto plugin: this->plugins) {
            auto lua = plugin.second->luaEngine(this->id);
            formatEvent(events, lua);
            lua.table_call("Plugin", "Run", nullptr);
            SPDLOG_INFO("{}", events.at(0).weight.text);
        }
        SPDLOG_INFO("[{}] events count:{} ", this->id, events.size());
    }
}

void CameraHandle::startPrediction() {
    SPDLOG_INFO("startPrediction");
    this->prediction = true;
    thread handle([this] {
        cv::waitKey(100);
        SPDLOG_INFO("[{}] Handle start ", this->id);
        while (this->prediction) {
            if (this->cameraPull->flag) {
                this->Handle(this->cameraPull->get());
            }
        }
        SPDLOG_INFO("[{}] Handle end ", this->id);
    });


    this->cameraPull->start();
}

CameraHandle::CameraHandle(string id, string url) : url(url), id(id) {
    this->cameraPull = new CameraPull(this->url, id);
    string pluginName = "helmet";
    auto plugin = PluginManager::GetInStance().getPlugin(pluginName.c_str());
    if (plugin == nullptr) {
        SPDLOG_ERROR("[{}] init {} plugin error.", this->id, pluginName);
    }
    this->plugins.insert({plugin->Name(), plugin});

    this->algorithm_list.insert(pluginName);
    this->yolo = new YoloV5(this->plugins, true);
    this->frameCount = 0;
}

CameraHandle::~CameraHandle() {
    SPDLOG_INFO("~CameraHandle");
}