//
// Created by uma-pc001 on 2022/5/27.
//

#include "Plugin.h"

Plugin::Plugin(const char *script) {
    SPDLOG_INFO("script:{}", script);
    auto kit_state = luakit::kit_state();
    kit_state.run_file(script);
    auto flag = kit_state.call("Run", nullptr);
    auto name = kit_state.get<luakit::lua_table>("Plugin").get<string>("Name");
    printf("Name from Lua:%s \n", name.c_str());
    flag = kit_state.get<luakit::lua_table>("Plugin").get_function("Run");
    printf("Run Flag:%d \n", flag);
    flag = kit_state.table_call("Plugin", "Run", nullptr);
    printf("Call Run Flag:%d \n", flag);
    kit_state.close();
}

