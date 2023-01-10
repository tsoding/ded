// Copyright 2021 Alexey Kutepov <reximkut@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// ============================================================
//
// minirent — 0.0.1 — A subset of dirent interface for Windows.
//
// https://github.com/tsoding/minirent
//
// ============================================================
//
// ChangeLog (https://semver.org/ is implied)
//
//    0.0.1 First Official Release

#ifndef MINIRENT_H_
#define MINIRENT_H_

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

struct dirent
{
    char d_name[MAX_PATH+1];
};

typedef struct DIR DIR;

DIR *opendir(const char *dirpath);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif  // MINIRENT_H_

#ifdef MINIRENT_IMPLEMENTATION

struct DIR
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    struct dirent *dirent;
};

DIR *opendir(const char *dirpath)
{
    assert(dirpath);

    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\*", dirpath);

    DIR *dir = (DIR*)calloc(1, sizeof(DIR));

    dir->hFind = FindFirstFile(buffer, &dir->data);
    if (dir->hFind == INVALID_HANDLE_VALUE) {
        // TODO: opendir should set errno accordingly on FindFirstFile fail
        // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        errno = ENOSYS;
        goto fail;
    }

    return dir;

fail:
    if (dir) {
        free(dir);
    }

    return NULL;
}

struct dirent *readdir(DIR *dirp)
{
    assert(dirp);

    if (dirp->dirent == NULL) {
        dirp->dirent = (struct dirent*)calloc(1, sizeof(struct dirent));
    } else {
        if(!FindNextFile(dirp->hFind, &dirp->data)) {
            if (GetLastError() != ERROR_NO_MORE_FILES) {
                // TODO: readdir should set errno accordingly on FindNextFile fail
                // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
                errno = ENOSYS;
            }

            return NULL;
        }
    }

    memset(dirp->dirent->d_name, 0, sizeof(dirp->dirent->d_name));

    strncpy(
        dirp->dirent->d_name,
        dirp->data.cFileName,
        sizeof(dirp->dirent->d_name) - 1);

    return dirp->dirent;
}

int closedir(DIR *dirp)
{
    assert(dirp);

    if(!FindClose(dirp->hFind)) {
        // TODO: closedir should set errno accordingly on FindClose fail
        // https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
        errno = ENOSYS;
        return -1;
    }

    if (dirp->dirent) {
        free(dirp->dirent);
    }
    free(dirp);

    return 0;
}

#endif  // MINIRENT_IMPLEMENTATION
