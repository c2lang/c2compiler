/* Copyright 2022 Bas van den Berg
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

#include "common/file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

static char empty;

bool file_reader_open(FileReader* r, const char* filename) {
    struct stat statbuf;
    int err = stat(filename, &statbuf);
    if (err) return false;

    r->size = (uint32_t)statbuf.st_size;

    if (r->size == 0) {
        r->map = &empty;
        return true;
    }

    int fd = open(filename, O_RDONLY | O_CLOEXEC);
    if (fd == -1) return false;

    r->map = mmap(0, r->size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (r->map == MAP_FAILED) {
        close (fd);
        return false;
    }

    close (fd);
    return true;
}

void file_reader_close(FileReader* r) {
    munmap(r->map, r->size);
}

