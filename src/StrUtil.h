//
// Created by uma-pc001 on 2022/5/28.
//

#ifndef EASYNVR_STRUTIL_H
#define EASYNVR_STRUTIL_H

#include "iostream"
#include "vector"
#include "scope_guard.hpp"

using namespace std;

class StrUtil {
public:
    static bool startswith(string src, string find);

    static bool startswith(const char *src, const char *find);


    static vector<string> split(string &src, char split);
};


#endif //EASYNVR_STRUTIL_H
