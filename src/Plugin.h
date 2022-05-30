//
// Created by uma-pc001 on 2022/5/27.
//

#ifndef EASYNVR_PLUGIN_H
#define EASYNVR_PLUGIN_H

#include "EasyNvr.h"
#include "luakit/lua_kit.h"
#include "iostream"

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

private:
    //lua 虚拟机

    Weight weight;

    string script;

    map<string, luakit::kit_state> luaEngines;
};


#endif //EASYNVR_PLUGIN_H
