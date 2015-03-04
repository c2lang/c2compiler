/* Copyright 2013,2014,2015 Bas van den Berg
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

#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/FileSystem.h>

#include "FileUtils/FileUtils.h"
#include "Utils/StringBuilder.h"

using namespace C2;
using namespace llvm;

void FileUtils::writeFile(const char* pathstr, const std::string& filename, const StringBuilder& content) {
    bool existed = true;
    llvm::Twine path(pathstr);
    if (std::error_code ec = llvm::sys::fs::create_directories(path, existed)) {
        llvm::errs() << "warning: could not create directory '"
                     << path << "': " << ec.message() << '\n';
        return;
    }

    std::string ErrorInfo;
    llvm::raw_fd_ostream OS(filename.c_str(), ErrorInfo, sys::fs::F_None);
    if (!ErrorInfo.empty()) {
        fprintf(stderr, "%s\n", ErrorInfo.c_str());
        return;
    }
    OS << content;
}

