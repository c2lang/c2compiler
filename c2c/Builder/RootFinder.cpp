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

#include "Builder/RootFinder.h"
#include "Builder/BuilderConstants.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace C2;

RootFinder::RootFinder() : path_prefix(0) {
    base_path[0] = 0;
    rel_path[0] = 0;
}

void RootFinder::findTopDir() {
    char* path = getcwd(base_path, PATH_MAX);
    char buffer[PATH_MAX];
    while (1) {
        path = getcwd(buffer, PATH_MAX);
        if (path == NULL) {
            perror("getcwd");
            exit(-1);
        }
        struct stat buf;
        int error = stat(RECIPE_FILE, &buf);
        if (error) {
            if (buffer[0] == '/' && buffer[1] == 0) {
                fprintf(stderr, "c2c: error: cannot find C2 root dir\n");
                fprintf(stderr, "c2c requires a %s file for compilation of targets\n", RECIPE_FILE);
                fprintf(stderr, "Use argument -h for a list of available options and usage of c2c\n");
                exit(-1);
            }
            if (errno != ENOENT) {
                perror("stat");
                exit(-1);
            }
        } else {
            // must be file, not dir
            if (S_ISREG(buf.st_mode)) {
                path_prefix = base_path + strlen(buffer);
                if (*path_prefix == '/') path_prefix++;
                return;
            }
        }
        error = chdir("..");
        if (error) {
            perror("chdir");
            exit(-1);
        }
        strcat(rel_path, "../");
    }
}

std::string RootFinder::orig2Root(const std::string& filename) const {
    char fullname[PATH_MAX];
    if (path_prefix[0] != 0) {
        sprintf(fullname, "%s/%s", path_prefix, filename.c_str());
    } else {
        strcpy(fullname, filename.c_str());
    }
    return fullname;
}

std::string RootFinder::root2Orig(const std::string& filename) const {
    char fullname[PATH_MAX];
    sprintf(fullname, "%s%s", rel_path, filename.c_str());
    return fullname;
}

