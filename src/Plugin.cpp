//
// Created by uma-pc001 on 2022/5/27.
//

#include "Plugin.h"

//递归压入可变参数
template<typename T0>
void push_luabridge(lua_State *lua_state, T0 &&param) {
    luabridge::Stack<T0>::push(lua_state, std::forward<decltype(param)>(param));
}

//C++调用lua 可变参数模板
template<typename R, typename ...T>
R call(lua_State *lua_state, const char *name, T ... args) {
    int params_num = sizeof...(args);
    lua_getglobal(lua_state, name);
    if (lua_isfunction(lua_state, -1)) {
        int arr[] = {(push_luabridge(lua_state, args), 0)...};
        lua_pcall(lua_state, params_num, 1, 0);
    } else {
        SPDLOG_ERROR("call lua function by c++ template fail, case func {} not function or no define.", name);
    }
    return luabridge::Stack<R>::get(lua_state, -1);
}


Plugin::Plugin(const char *script) {
    SPDLOG_INFO("script:{}", script);
    L = luaL_newstate();
    luaL_dofile(L, script);
    luaL_openlibs(L);
    lua_pcall(L, 0, 0, 0);
    auto Plugin = getGlobal(L, "Plugin");
    auto run = Plugin["Run"];
    call<bool>(L, "Run", 1);
    run("123");

}

