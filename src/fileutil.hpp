
//
// Created by uma-pc001 on 2022/5/12.
//
#ifndef Enjoy_Kit_FileUtil_H
#define Enjoy_Kit_FileUtil_H

#include "iostream"
#include "vector"

#include "io.h"
#include "scope_guard.hpp"

#ifdef _WIN32

#include <windows.h>

#endif

using namespace std;
namespace Enjoy {
    namespace Kit {
        class FileUtil {
            struct File {
                char name[256];
                char type;
                unsigned short mode;
                size_t size;
                time_t atime;
                time_t mtime;
                time_t ctime;
            };
        public:
            static bool exist(string path);

            static bool exist(const char *path);

            static bool del(string path);

            static bool del(const char *path);

            static bool copy(const char *src, const char *dest, bool isOverride = false);

            static bool copy(string src, string dest, bool isOverride = false);

            static bool mkdir(string path);

            static bool mkdir(const char *path);

            static vector<string> ls(string dir);

            static vector<string> ls(const char *dir);

            static vector<File> ls_file(string dir, bool onlyFile = true);

            static vector<File> ls_file(const char *dir, bool onlyFile = true);

            static string readString(string file);

            static string readString(const char *file);

            static vector<string> readLines(string file);

            static vector<string> readLines(const char *file);
        };
    }
}

#ifdef _WIN32

#define FILETIME_EPOCH_DIFF     11644473600 // s

time_t FileTime2Epoch(FILETIME filetime) {
    uint64_t ll = (((uint64_t) filetime.dwHighDateTime) << 32) | filetime.dwLowDateTime;
    ll /= 1e7; // s
    return ll - FILETIME_EPOCH_DIFF;
}

string Enjoy::Kit::FileUtil::readString(const char *file) {
    char filepath[MAX_PATH];
    strncpy(filepath, file, MAX_PATH);
    return "";
}

bool Enjoy::Kit::FileUtil::mkdir(string path) {
    return mkdir(path.c_str());
}

bool Enjoy::Kit::FileUtil::mkdir(const char *path) {
    if (exist(path)) {
        return true;
    }
    const char *s = strrchr(path, '/');
    if (s == 0) s = strrchr(path, '\\');
    if (s == 0) return CreateDirectoryA(path, 0);

    unsigned long r = s - path;
    char *parent = (char *) malloc(sizeof(char) * r + 1);
    DEFER {
        free(parent);
        parent = NULL;
    };
    memcpy(parent, path, r);
    parent[r] = '\0';
    if (exist(parent)) {
        return CreateDirectoryA(path, 0);
    } else {
        return mkdir(parent) && CreateDirectoryA(path, 0);
    }
}

bool Enjoy::Kit::FileUtil::copy(string src, string dest, bool isOverride) {
    return copy(src.c_str(), dest.c_str(), isOverride);
}

bool Enjoy::Kit::FileUtil::copy(const char *src, const char *dest, bool isOverride) {
    if (!exist(src)) {
        return false;
    }
    if (exist(dest)) {
        if (isOverride) {
            del(dest);
        } else {
            return false;
        }
    }
    return MoveFileA(src, dest);
}

bool Enjoy::Kit::FileUtil::del(string path) {
    return del(path.c_str());
}

bool Enjoy::Kit::FileUtil::del(const char *path) {
    if (!exist(path)) {
        return true;
    }
    size_t len = strlen(path) + 12;
    char *f = new char[len];
    sprintf(f, "rd /s /q \"%s\"", path);
    return system(f) != -1;
}

bool Enjoy::Kit::FileUtil::exist(string path) {
    return exist(path.c_str());
}

bool Enjoy::Kit::FileUtil::exist(const char *path) {
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

vector<string> Enjoy::Kit::FileUtil::ls(string dir) {
    return ls(dir.c_str());
}

vector<Enjoy::Kit::FileUtil::File> Enjoy::Kit::FileUtil::ls_file(string dir, bool onlyFile) {
    return ls_file(dir.c_str(), onlyFile);
}

vector<Enjoy::Kit::FileUtil::File> Enjoy::Kit::FileUtil::ls_file(const char *dir, bool onlyFile) {
    vector<Enjoy::Kit::FileUtil::File> dirlist = {};
    int dirlen = strlen(dir);
    if (dirlen > 256) {
        return {};
    }
    char path[512];
    strcpy(path, dir);
    if (dir[dirlen - 1] != '/') {
        strcat(path, "/");
        ++dirlen;
    }
    strcat(path, "*");
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA(path, &data);
    if (h == NULL) {
        return {};
    }
    File tmp;
    do {
        memset(&tmp, 0, sizeof(File));
        strncpy(tmp.name, data.cFileName, sizeof(tmp.name));
        tmp.type = 'f';
        if (data.dwFileAttributes & _A_SUBDIR) {
            tmp.type = 'd';
            if (onlyFile) {
                continue;
            }
        }
        tmp.mode = 0777;
        tmp.size = (((uint64_t) data.nFileSizeHigh) << 32) | data.nFileSizeLow;
        tmp.atime = FileTime2Epoch(data.ftLastAccessTime);
        tmp.mtime = FileTime2Epoch(data.ftLastWriteTime);
        tmp.ctime = FileTime2Epoch(data.ftCreationTime);
        dirlist.push_back(tmp);

    } while (FindNextFileA(h, &data));
    FindClose(h);
    return dirlist;
}

vector<string> Enjoy::Kit::FileUtil::ls(const char *dir) {
    vector<File> tmp = ls_file(dir, true);
    vector<string> files;
    for (auto file: tmp) {
        files.push_back(file.name);
    }
    vector<File>().swap(tmp);
    return files;
}


#endif
#ifndef _WIN32
bool Enjoy::Kit::FileUtil::exist(string path) {
     return true;
}

bool Enjoy::Kit::FileUtil::exist(const char *path) {
    return true;
}

vector<string> Enjoy::Kit::FileUtil::ls(string path) {
   return {};
}

vector<string> Enjoy::Kit::FileUtil::ls(const char *path) {
 return {};
}
#endif
#endif