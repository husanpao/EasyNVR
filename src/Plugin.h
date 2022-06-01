//
// Created by uma-pc001 on 2022/5/27.
//

#ifndef EASYNVR_PLUGIN_H
#define EASYNVR_PLUGIN_H

#include "EasyNvr.h"
#include "luakit/lua_kit.h"
#include "iostream"
#include <opencv2/opencv.hpp>
using namespace std;


class Plugin {
public:
    ~Plugin() {
        std::cout << "Plugin  destory..." << std::endl;
    }

    Plugin(const char *script);

    string Name();

    Weight WeightInfo();

    luakit::kit_state luaEngine(string id);

    string labelText(int idx);

    Event luaTab2Event(luakit::lua_table table);

    luakit::lua_table Event2luaTab(Event event);

    bool intersect(Event a, Event b);

    float overlapRate(Event a, Event b);

private:
    //lua 虚拟机

    Weight weight;

    string script;

    map<string, luakit::kit_state> luaEngines;
};


#endif //EASYNVR_PLUGIN_H
