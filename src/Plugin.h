//
// Created by uma-pc001 on 2022/5/27.
//

#ifndef EASYNVR_PLUGIN_H
#define EASYNVR_PLUGIN_H

#include "EasyNvr.h"
#include "LuaBridge/LuaBridge.h"
#include "iostream"

using namespace std;
using namespace luabridge;

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


private:
    //lua 虚拟机
    lua_State *L;
    Weight weight;
};


#endif //EASYNVR_PLUGIN_H
