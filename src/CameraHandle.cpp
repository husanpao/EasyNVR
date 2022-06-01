//
// Created by uma-pc001 on 2022/5/28.
//

#include "CameraHandle.h"

luakit::lua_table CameraHandle::formatEvent(map<string, vector<Event>> classifyEvent, luakit::kit_state lua) {
    auto eventsTab = lua.new_table("events");

    map<string, vector<Event>>::iterator eventIterator;
    for (eventIterator = classifyEvent.begin();
         eventIterator != classifyEvent.end(); eventIterator++) {
        auto classifyTab = lua.new_table(eventIterator->first.c_str());
        int i = 0;
        for (Event event: eventIterator->second) {
            auto label = lua.new_table(fmt::format("label_{}", i).c_str());
            label.set("name", event.weight.name, "render", event.weight.render, "text", event.weight.text, "threshold",
                      event.weight.threshold, "flag", event.weight.flag);
            auto eventTab = lua.new_table(fmt::format("event_{}", i).c_str());
            eventTab.set("event", event.event, "left", event.left, "hold", event.hold, "right", event.right, "top",
                         event.top, "bottom", event.bottom, "weight", label);
            classifyTab.set(fmt::format("{}", i).c_str(), eventTab);
            i++;
        }
        eventsTab.set(eventIterator->first.c_str(), classifyTab);
    }
    return eventsTab;
}

void CameraHandle::drawFrame(cv::Mat frame) {

}


void CameraHandle::Handle(cv::Mat frame) {
    if (frame.empty()) {
        return;
    }
    if (this->frameCount++ % 5 == 0) {
        const vector<Event> &events = this->yolo->prediction(frame, this->algorithm_list);
        if (events.size() > 0) {
            map<string, vector<Event>> classifyEvent;
            for (Event event: events) {
                if (classifyEvent.count(event.weight.name) == 0) {
                    vector<Event> tempEvents;
                    tempEvents.push_back(event);
                    classifyEvent.insert({event.weight.name, tempEvents});
                } else {
                    classifyEvent[event.weight.name].push_back(event);
                }
            }
            thread_pool *taskPool = new thread_pool(this->plugins.size());
            for (auto plugin: this->plugins) {
                taskPool->push_task([plugin, this, classifyEvent, frame]() {
                    auto lua = plugin.second->luaEngine(this->id);
                    formatEvent(classifyEvent, lua);
                    lua.table_call("Plugin", "Run");
                    vector<luakit::lua_table> rs = lua.get<luakit::lua_table>(
                            "Events").to_sequence<vector<luakit::lua_table>, luakit::lua_table>();
                    this->drawFrame(frame);
                });
            }
            taskPool->wait_for_tasks();
            delete taskPool;
        }
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