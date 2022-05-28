//
// Created by uma-pc001 on 2022/5/27.
//

#include <io.h>
#include "FileUtil.h"

#ifdef _WIN32

#define FILETIME_EPOCH_DIFF     11644473600 // s

time_t FileTime2Epoch(FILETIME filetime) {
    uint64_t ll = (((uint64_t) filetime.dwHighDateTime) << 32) | filetime.dwLowDateTime;
    ll /= 1e7; // s
    return ll - FILETIME_EPOCH_DIFF;
}

string FileUtil::readString(const char *file) {
    char filepath[MAX_PATH];
    strncpy(filepath, file, MAX_PATH);
    return "";
}

bool FileUtil::mkdir(string path) {
    return mkdir(path.c_str());
}

bool FileUtil::mkdir(const char *path) {
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

bool FileUtil::copy(string src, string dest, bool isOverride) {
    return copy(src.c_str(), dest.c_str(), isOverride);
}

bool FileUtil::copy(const char *src, const char *dest, bool isOverride) {
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

bool FileUtil::del(string path) {
    return del(path.c_str());
}

bool FileUtil::del(const char *path) {
    if (!exist(path)) {
        return true;
    }
    size_t len = strlen(path) + 12;
    char *f = new char[len];
    sprintf(f, "rd /s /q \"%s\"", path);
    return system(f) != -1;
}

bool FileUtil::exist(string path) {
    return exist(path.c_str());
}

bool FileUtil::exist(const char *path) {
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

vector<string> FileUtil::ls(string dir) {
    return ls(dir.c_str());
}

vector<FileUtil::File> FileUtil::ls_file(string dir, bool onlyFile) {
    return ls_file(dir.c_str(), onlyFile);
}

vector<FileUtil::File> FileUtil::ls_file(const char *dir, bool onlyFile) {
    vector<FileUtil::File> dirlist = {};
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

vector<string> FileUtil::ls(const char *dir) {
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
bool FileUtil::exist(string path) {
     return true;
}

bool FileUtil::exist(const char *path) {
    return true;
}

vector<string> FileUtil::ls(string path) {
   return {};
}

vector<string> FileUtil::ls(const char *path) {
 return {};
}
#endif
