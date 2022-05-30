//
// Created by uma-pc001 on 2022/5/27.
//

#ifndef EASYNVR_PLUGINMANAGER_H
#define EASYNVR_PLUGINMANAGER_H

#include <iostream>
#include "EasyNvr.h"
#include "FileUtil.h"
#include "Plugin.h"

class PluginManager {
public:
    ~PluginManager() {
        std::cout << "LuaManager  destructor..." << std::endl;
    }

    PluginManager(const PluginManager &) = delete;

    PluginManager &operator=(const PluginManager &) = delete;

    static PluginManager &GetInStance() {
        static PluginManager instance;
        return instance;
    }

    bool addPlugin(const char *luaScriptName);

    bool removePlugin(const char *luaScriptName);

    Plugin *getPlugin(const char *plugin);

private:
    PluginManager() {
        this->init();
    }

    bool init();


    bool initPlugins();

private:
    char *scriptFoler = "scripts";

    map<string, Plugin *> plugins;
};


#endif //EASYNVR_PLUGINMANAGER_H
