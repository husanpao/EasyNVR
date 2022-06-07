//
// Created by uma-pc001 on 2022/5/28.
//

#include "CameraHandle.h"

void CameraHandle::formatEvent(map<string, vector<Event>> classifyEvent, luakit::kit_state lua) {
    auto eventsTab = lua.new_table("events");
    for (auto eventList: classifyEvent) {
        auto classifyTab = lua.new_table(eventList.first.c_str());
        int i = 0;
        for (Event event: eventList.second) {
            auto eventTab = lua.new_table(fmt::format("event_{}", i).c_str());
            eventTab.set("threshold", event.threshold, "label", event.label, "x1", event.x1, "y1", event.y1, "x2",
                         event.x2, "y2",
                         event.y2, "label_zh", event.label_zh, "label_en", event.label_en);
            classifyTab.set(fmt::format("{}", i).c_str(), eventTab);
            i++;
        }
        eventsTab.set(eventList.first.c_str(), classifyTab);
    }
}

void CameraHandle::drawFrame(cv::Mat frame, Event event) {
//    this->yolo->drawRectangle(frame, event.left, event.top, event.right, event.bottom, event.weight.text);
}


void CameraHandle::Handle(cv::Mat frame) {
    if (frame.empty()) {
        return;
    }
    if (this->frameCount++ % 5 == 0) {
        this->events.clear();
        std::vector<NcnnObject> tempEvents;
        ncnn->Detect(frame, tempEvents);
        if (tempEvents.size() > 0) {
            map<string, vector<Event>> classifyEvent;
            for (NcnnObject event: tempEvents) {
                Event eventStruct = {event.threshold, event.label, event.label_zh, event.label_en, (int) event.rect.x,
                                     (int) event.rect.y,
                                     (int) (event.rect.x + event.rect.width), (int) (event.rect.y + event.rect.height)};
                if (classifyEvent.count(event.label_en) == 0) {
                    vector<Event> tempEvents;
                    tempEvents.push_back(eventStruct);
                    classifyEvent.insert({event.label_en, tempEvents});
                } else {
                    classifyEvent[event.label_en].push_back(eventStruct);
                }
            }
            thread_pool *taskPool = new thread_pool(this->plugins.size());
            for (auto plugin: this->plugins) {
                taskPool->push_task([plugin, this, classifyEvent, frame]() {
                    auto lua = plugin.second->luaEngine(this->id);
                    formatEvent(classifyEvent, lua);
                    lua.table_call("Plugin", "Run");
                    vector<luakit::lua_table> events = lua.get<luakit::lua_table>(
                            "RS").to_sequence<vector<luakit::lua_table>, luakit::lua_table>();
                    for (luakit::lua_table event: events) {
                        this->events.push_back(plugin.second->luaTab2Ncnn(event));
                    }
                });
            }
            taskPool->wait_for_tasks();
            delete taskPool;
        }
    }
    ncnn->Draw(frame, events,true);
    cv::imshow(id, frame);
    cv::waitKey(1);
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
    handle.detach();
    this->cameraPull->start();
    this->prediction = false;
    SPDLOG_INFO("endPrediction");
}

CameraHandle::CameraHandle(string id, string url, string plugins) : url(url), id(id) {
    this->cameraPull = new CameraPull(this->url, id);
    this->ncnn = new Ncnn();
    auto pluginsStr = StrUtil::split(plugins, ',');
    for (string pluginName: pluginsStr) {
        auto plugin = PluginManager::GetInStance().getPlugin(pluginName.c_str());
        if (plugin == nullptr) {
            SPDLOG_ERROR("[{}] init {} plugin error.", this->id, pluginName);
        } else {
            this->ncnn->AddNet(pluginName.c_str(), plugin->WeightInfo().labels,
                               fmt::format("{}.ncnn.param", plugin->Name()).c_str(),
                               fmt::format("{}.ncnn.bin", plugin->Name()).c_str());
            this->plugins.insert({plugin->Name(), plugin});
        }
    }
    this->frameCount = 0;
}

CameraHandle::~CameraHandle() {
    SPDLOG_INFO("~CameraHandle");
}