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
    struct Weight {
        string name;
        double threshold;
        bool enable;
        unordered_map<int, std::string> labels;
        string model;
    };
public:
    ~Plugin() {
        L.close();
        std::cout << "Plugin  destory..." << std::endl;
    }

    Plugin(const char *script);


private:
    //lua 虚拟机
    luakit::kit_state L;
    Weight weight;
};


#endif //EASYNVR_PLUGIN_H
