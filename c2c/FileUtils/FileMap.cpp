/* Copyright 2013-2017 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#include "FileUtils/FileMap.h"

using namespace C2;

File::File(const char* filename_)
    : filename (filename_)
    , region(0)
    , size(0)
    , last_change(0)
    , write_cache(false)
{}


FileMap::FileMap(const char* filename_)
    : File(filename_)
{
    struct stat statbuf;
    int err = stat(filename_, &statbuf);
    if (err) {
        perror("stat");
        exit(-1);
    }
    size = statbuf.st_size;
    last_change = statbuf.st_mtime;
}


FileMap::~FileMap() {
    if (region) close();
}

void FileMap::open() {
    int fd = ::open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "error opening '%s': %s\n", filename.c_str(), strerror(errno));
        exit(-1);
    }

    region = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (region == MAP_FAILED) {
        perror("mmap");
        exit(-1);
    }
    ::close (fd);
    write_cache = true;
}

void FileMap::close() {
    munmap(region, size);
    region = 0;
}

FileString::FileString(const char* filename_, const char* text, time_t ts)
    : File(filename_)
{
    region = (void*)text;
    size = strlen(text);
    last_change = ts;
}

