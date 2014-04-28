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

#ifndef FILEUTILS_FILE_DB_H
#define FILEUTILS_FILE_DB_H

#include <string>
#include <vector>

namespace C2 {

class FileDb {
public:
    FileDb() {}
    unsigned add(const std::string& name) {
        files.push_back(name);
        return files.size()-1;
    }
    const std::string& resolve(unsigned id) const {
        return files[id];
    }
private:
    typedef std::vector<std::string> Filenames;
    Filenames files;

    FileDb(const FileDb&);
    FileDb& operator= (const FileDb&);
};

}

#endif

