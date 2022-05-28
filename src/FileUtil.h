//
// Created by uma-pc001 on 2022/5/27.
//

#ifndef EASYNVR_FILEUTIL_H
#define EASYNVR_FILEUTIL_H
#ifdef _WIN32

#include <windows.h>

#endif

#include "iostream"
#include "vector"
#include "scope_guard.hpp"

using namespace std;

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


#endif //EASYNVR_FILEUTIL_H
