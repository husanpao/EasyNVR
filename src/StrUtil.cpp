//
// Created by uma-pc001 on 2022/5/28.
//

#include "StrUtil.h"

bool StrUtil::startswith(const char *src, const char *find) {
    int scrlen = strlen(src);
    int findlen = strlen(find);
    if (scrlen == 0 || findlen == 0) {
        return false;
    }
    return findlen <= scrlen && memcmp(src, find, findlen) == 0;
}

bool StrUtil::startswith(string src, string find) {
    return startswith(src.c_str(), find.c_str());
}


vector<string> StrUtil::split(string &src, char split) {
    vector<string> res;
    const char *p = src.c_str();
    const char *value = p;
    while (*p != '\0') {
        if (*p == split) {
            res.push_back(std::string(value, p - value));
            value = p + 1;
        }
        ++p;
    }
    res.push_back(value);
    return res;
}