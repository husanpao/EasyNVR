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
        double similarity;
        bool enable;
        std::map<int, std::string> labels;
        string model;
    };
public:
    ~Plugin() {
        std::cout << "LuaManager  destructor..." << std::endl;
    }

    Plugin(const char *script);

    void Hello();

private:
    //lua 虚拟机
    luakit::kit_state *L;
    Weight weight;
    char *test = "name";
};


#endif //EASYNVR_PLUGIN_H
