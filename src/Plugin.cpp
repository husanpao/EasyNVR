//
// Created by uma-pc001 on 2022/5/27.
//

#include <chrono>
#include "Plugin.h"

Plugin::Plugin(const char *script) {
    SPDLOG_INFO("script:{} load start.", script);
    this->script = script;
    auto kit_state = luakit::kit_state();
    DEFER {
        kit_state.close();
        SPDLOG_INFO("script:{} load end.", script);
    };
    kit_state.run_file(script);
    auto plugin = kit_state.get<luakit::lua_table>("Plugin");
    this->weight.name = plugin.get<string>("name");
    this->weight.pt = plugin.get<string>("pt");
    this->weight.threshold = plugin.get<float>("threshold");
    this->weight.enable = plugin.get<bool>("enable");
    vector<luakit::lua_table> labels = plugin.get<luakit::reference>(
            "labels").to_sequence<vector<luakit::lua_table>, luakit::lua_table>();
    int i = 0;
    for (luakit::lua_table label: labels) {
        Label l;
        l.zh = label.get<string>("zh");
        l.render = label.get<bool>("render");
        l.en = label.get<string>("en");
        l.threshold = label.get<float>("threshold");
        l.threshold = l.threshold ? l.threshold : this->weight.threshold;
        l.flag = label.get<bool>("flag");
        this->lables.insert({l.en, l});
        this->weight.labels.insert({i++, l});
    }

}

luakit::lua_table Plugin::Event2luaTab(Event event) {


    return nullptr;
}

bool Plugin::intersect(Event a, Event b) {
    if (abs((a.x1 + a.x2) / 2 - (b.x1 + b.x2) / 2) <
        ((a.x2 + b.x2 - a.x1 - b.x1) / 2)
        && abs((a.y1 + a.y2) / 2 - (b.y1 + b.y2) / 2) <
           ((a.y2 + b.y2 - a.y1 - b.y1) / 2))
        return true;
    return false;
}

float Plugin::overlapRate(Event a, Event b) {
    cv::Rect rc1(a.x1, a.y1, a.x2 - a.x1,
                 a.y2 - a.y1);
    cv::Rect rc2(b.x1, b.y1, b.x2 - b.x1,
                 b.y2 - b.y1);
    CvPoint p1, p2;                 //p1为相交位置的左上角坐标，p2为相交位置的右下角坐标
    p1.x = std::max(rc1.x, rc2.x);
    p1.y = std::max(rc1.y, rc2.y);

    p1.x = std::max(rc1.x, rc2.x);
    p1.y = std::max(rc1.y, rc2.y);

    p2.x = std::min(rc1.x + rc1.width, rc2.x + rc2.width);
    p2.y = std::min(rc1.y + rc1.height, rc2.y + rc2.height);

    float AJoin = 0;
    if (p2.x > p1.x && p2.y > p1.y)            //判断是否相交
    {
        AJoin = (p2.x - p1.x) * (p2.y - p1.y);    //求出相交面积
    }
    float A1 = rc1.width * rc1.height;
    float A2 = rc2.width * rc2.height;
    float AUnion = A1 > A2 ? A1 : A2;                 //两者组合的面积
    if (AUnion > 0)
        return (AJoin / AUnion);                  //相交面积与组合面积的比例
    else
        return 0;
}


Event Plugin::luaTab2Event(luakit::lua_table table) {
    auto threshold = table.get<float>("threshold");
    auto label = table.get<int>("label");
    auto label_zh = table.get<string>("label_zh");
    auto label_en = table.get<string>("label_en");
    auto x1 = table.get<int>("x1");
    auto y1 = table.get<int>("y1");
    auto x2 = table.get<int>("x2");
    auto y2 = table.get<int>("y2");
    return {threshold, label, label_zh, label_en, x1, y1, x2, y2};
}

NcnnObject Plugin::luaTab2Ncnn(luakit::lua_table table) {
    auto event = luaTab2Event(table);
    NcnnObject ncnnObject;
    ncnnObject.label = event.label;
    ncnnObject.label_zh = event.label_zh;
    ncnnObject.label_en = event.label_en;
    ncnnObject.threshold = event.threshold;
    cv::Rect_ rect(event.x1, event.y1, event.x2 - event.x1, event.y2 - event.y1);
    ncnnObject.rect = rect;
    return ncnnObject;
}

luakit::kit_state Plugin::luaEngine(string id) {
    if (this->luaEngines.count(id) == 0) {
        auto kit_state = luakit::kit_state();
        kit_state.run_file(script);
        kit_state.set_function("INFO", [this, id](string log) {
            SPDLOG_INFO("[{}] [{}] :{}", id, this->Name(), log);
        });
        kit_state.set_function("WARN", [this, id](string log) {
            SPDLOG_WARN("[{}] [{}] :{}", id, this->Name(), log);
        });
        kit_state.set_function("ERROR", [this, id](string log) {
            SPDLOG_ERROR("[{}] [{}] :{}", id, this->Name(), log);
        });

        kit_state.set_function("Intersect", [this, id](luakit::lua_table a, luakit::lua_table b) {
            SPDLOG_INFO("[{}] [{}] :Intersect", id, this->Name());
            auto ae = this->luaTab2Event(a);
            auto be = this->luaTab2Event(b);
            return intersect(ae, be);

        });
        kit_state.set_function("OverlapRate", [this, id](luakit::lua_table a, luakit::lua_table b) {
            SPDLOG_INFO("[{}] [{}] :OverlapRate", id, this->Name());
            auto ae = this->luaTab2Event(a);
            auto be = this->luaTab2Event(b);
            return overlapRate(ae, be);
        });
        kit_state.set_function("MS", [this, id]() {
            std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tpMicro
                    = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
            return tpMicro.time_since_epoch().count();
        });
        auto lablesTab = kit_state.new_table("LABLES");
        for (auto label: lables) {
            auto labelTab = kit_state.new_table(label.first.c_str());
            labelTab.set("en", label.second.en, "render", label.second.render, "zh", label.second.zh, "threshold",
                         label.second.threshold, "flag", label.second.flag);
            lablesTab.set(label.first.c_str(), labelTab);
        }
        
        this->luaEngines.insert({id, kit_state});
    }
    return this->luaEngines[id];
}

string Plugin::Name() {
    return this->weight.name;
}


Weight Plugin::WeightInfo() {
    return this->weight;
}