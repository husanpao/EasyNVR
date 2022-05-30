//
// Created by uma-pc001 on 2022/5/27.
//

#include "PluginManager.h"


bool PluginManager::init() {
    this->initPlugins();
    return true;
}


bool PluginManager::addPlugin(const char *luaScriptName) {
    string script(this->scriptFoler);
    script.append("/").append(luaScriptName);
    Plugin *plugin = new Plugin(script.c_str());
    this->plugins.insert({plugin->Name(), plugin});
    return true;
}

Plugin *PluginManager::getPlugin(const char *plugin) {
    if (this->plugins.count(plugin) != 0) {
        return this->plugins[plugin];
    } else {
        return nullptr;
    }
}

bool PluginManager::initPlugins() {
    const vector<string> &scripts = FileUtil::ls(this->scriptFoler);
    for (string script: scripts) {
        if (script.find(".lua") != 0) {
            addPlugin(script.c_str());
        }
    }
    return true;
}
