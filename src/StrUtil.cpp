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