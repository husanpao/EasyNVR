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