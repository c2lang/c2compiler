/* Copyright 2013,2014 Bas van den Berg
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

#ifndef FILEUTILS_FILEMAP_H
#define FILEUTILS_FILEMAP_H

#include <string>

namespace C2 {

class File {
public:
    File(const char* filename_);
    virtual ~File() {}

    virtual void open() = 0;
    virtual void close() = 0;

    std::string filename;
    void* region;
    unsigned size;
    time_t last_change;
    bool write_cache;   // whether compiler needs to write a cache for this file
};


class FileMap : public File {
public:
    FileMap(const char* filename_);
    virtual ~FileMap();

    virtual void open();
    virtual void close();
private:
    FileMap(const FileMap&);
    FileMap& operator= (const FileMap&);
};


class FileString : public File {
public:
    FileString(const char* filename_, const char* text, time_t ts = 0);

    virtual void open() {}
    virtual void close() {}
private:
    FileString(const FileString&);
    FileString& operator= (const FileString&);
};


}

#endif

