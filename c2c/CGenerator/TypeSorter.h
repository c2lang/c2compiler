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

#ifndef CGENERATOR_TYPE_SORTER_H
#define CGENERATOR_TYPE_SORTER_H

#include <vector>
#include <map>

namespace C2 {

class Decl;
class DeclDep;

class CTypeWriter {
public:
    virtual ~CTypeWriter() {}
    virtual void forwardDecl(const Decl* D) = 0;
    virtual void fullDecl(const Decl* D) = 0;
};

class TypeSorter {
public:
    TypeSorter() {}
    ~TypeSorter();

    void add(const Decl* D);
    void write(CTypeWriter& writer);
private:
    typedef std::vector<DeclDep*> DeclDepList;
    typedef DeclDepList::const_iterator DeclDepListConstIter;
    typedef DeclDepList::iterator DeclDepListIter;
    DeclDepList depmap;

    typedef std::map<const Decl*, const Decl*> FunctionMap;
    typedef FunctionMap::const_iterator FunctionMapConstIter;
    FunctionMap functions;
};

}

#endif

