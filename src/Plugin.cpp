//
// Created by uma-pc001 on 2022/5/27.
//

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
        l.name = label.get<string>("name");
        l.render = label.get<bool>("render");
        l.text = label.get<string>("text");
        l.threshold = label.get<float>("threshold");
        l.threshold = l.threshold ? l.threshold : this->weight.threshold;
        l.flag = label.get<bool>("flag");
        this->weight.labels.insert({i++, l});
    }
    kit_state.new_class<LabelClass>(
            "name", &LabelClass::name,
            "render", &LabelClass::render,
            "text", &LabelClass::text,
            "threshold", &LabelClass::threshold,
            "flag", &LabelClass::flag
    );
    kit_state.new_class<EventClass>(
            "weight", &EventClass::weight,
            "hold", &EventClass::hold,
            "event", &EventClass::event,
            "left", &EventClass::left,
            "right", &EventClass::right,
            "top", &EventClass::top,
            "bottom", &EventClass::bottom
    );
    LabelClass *l = new LabelClass();
    l->name = "name";
    l->threshold = 1;
    l->render = false;
    l->text = "text";
    l->flag = true;
    auto e = new EventClass();
    e->hold = 1;
    e->bottom = 1;
    e->event = 1;
    e->weight = l;
    e->top = 1;
    e->left = 1;
    e->right = 1;
    kit_state.set("testEvent", e);
    kit_state.table_call("Plugin", "Test");

}

luakit::lua_table Plugin::Event2luaTab(Event event) {


    return nullptr;
}

bool Plugin::intersect(Event a, Event b) {
    if (abs((a.left + a.right) / 2 - (b.left + b.right) / 2) <
        ((a.right + b.right - a.left - b.left) / 2)
        && abs((a.top + a.bottom) / 2 - (b.top + b.bottom) / 2) <
           ((a.bottom + b.bottom - a.top - b.top) / 2))
        return true;
    return false;
}

float Plugin::overlapRate(Event a, Event b) {
    cv::Rect rc1(a.left, a.top, a.right - a.left,
                 a.bottom - a.top);
    cv::Rect rc2(b.left, b.top, b.right - b.left,
                 b.bottom - b.top);
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
    auto weight = table.get<luakit::lua_table>("weight");
    auto name = weight.get<string>("name");
    auto render = weight.get<bool>("render");
    auto text = weight.get<string>("text");
    auto threshold = weight.get<float>("threshold");
    auto flag = weight.get<bool>("flag");
    auto hold = table.get<float>("hold");
    auto event = table.get<int>("event");
    auto left = table.get<int>("left");
    auto right = table.get<int>("right");
    auto top = table.get<int>("top");
    auto bottom = table.get<int>("bottom");
    return {{name, render, text, threshold, flag}, hold, event, left, right, top, bottom};
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