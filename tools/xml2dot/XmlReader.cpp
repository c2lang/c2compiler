/* Copyright 2011,2012 Bas van den Berg
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
#include <string.h>
#include <stdarg.h>
#include <deque>

#include "XmlReader.h"

using namespace C2;
using namespace std;

static const string empty = "";

static inline char* itoa(unsigned int value) {
    static char str[11];
    str[10] = '\0';
    int  pos = 9;
    do {
        str[pos--] = (char) ((value % 10) + 0x30);
        value /= 10;
    } while (value != 0);
    return str + pos + 1;
}


namespace C2 {

const char* Error::what() const throw() { return msg.c_str(); }

class FileHandle {
public:
    FileHandle(FILE* handle_) : handle(handle_) {}
    FileHandle() { fclose(handle); }
private:
    FILE* handle;
};


class MemHandle {
public:
    MemHandle(char** memory_) : memory(memory_) {}
    MemHandle() {
        if (*memory) free(*memory);
    }
private:
    char** memory;
};


class XPath {
public:
    XPath (const std::string& path) {
        char buffer[256];
        strcpy(buffer, path.c_str());
        char* tok = strtok(buffer, "/");
        while (tok) {
            elements.push_back(tok);
            tok = strtok(NULL, "/");
        }
    }
    ~XPath () {}
    const char* operator[] (int i) const {
        return elements[i].c_str();
    }
    int size() const { return elements.size(); }

private:
    typedef std::deque<std::string> Elements;
    Elements elements;
};
}



XmlNode::XmlNode(const string& name_, unsigned int line_)
    : name(name_)
    , text("")
    , type(TBD)
    , line(line_)
{}

XmlNode::XmlNode(const string& name_, const string& text_, unsigned int line_)
    : name(name_)
    , text(text_)
    , type(TBD)
    , line(line_)
{}

XmlNode::~XmlNode() {
    for (ChildrenConstIter iter = children.begin(); iter != children.end(); ++iter) {
        delete *iter;
    }
}


int XmlNode::getChildLine(const string& key) const {
    for (ChildrenConstIter iter = children.begin(); iter != children.end(); ++iter) {
        if ((*iter)->name == key) return (*iter)->getLine();
    }
    return 0;
}


const string& XmlNode::getChildValue(const string& key) const {
    for (ChildrenConstIter iter = children.begin(); iter != children.end(); ++iter) {
        if ((*iter)->name == key) return (*iter)->text;
    }
    throw Error("tag [" + name + "] on line " + itoa(line) + " has no child '" + key + "'");
}


unsigned int XmlNode::getChildUIntValue(const string& key) const {
    const string& value = getChildValue(key);
    return atoi(value.c_str());
}


const string& XmlNode::getAttribute(const string& key) const {
    AttributesConstIter iter = attributes.find(key);
    if (iter == attributes.end()) throw Error("tag [" + name + "] on line " + itoa(line) + " has no attribute '" + key + "'");
    return iter->second;
}


const string& XmlNode::getOptionalAttribute(const string& key) const {
    AttributesConstIter iter = attributes.find(key);
    if (iter == attributes.end()) return empty;
    return iter->second;
}



unsigned int XmlNode::getUIntAttribute(const string& key) const {
	const string& value = getAttribute(key);
	return atoi(value.c_str());
}


bool XmlNode::hasAttribute(const std::string& key) const {
    AttributesConstIter iter = attributes.find(key);
    return  (iter != attributes.end());
}


bool XmlNode::hasChild(const std::string& key) const {
    for (ChildrenConstIter iter = children.begin(); iter != children.end(); ++iter) {
        if ((*iter)->name == key) return true;
    }
    return false;
}


const XmlNode* XmlNode::getChild(const std::string& key) const {
    for (ChildrenConstIter iter = children.begin(); iter != children.end(); ++iter) {
        if ((*iter)->name == key) return *iter;
    }
    return 0;
}


void XmlNode::addChild(XmlNode* child) {
    children.push_back(child);
}


void XmlNode::addAttribute(const string& key, const string& value) {
    attributes[key] = value;
}


void XmlNode::visitChildren(const std::string& name, XmlNodeVisitor& visitor) const {
    for (ChildrenConstIter iter = children.begin(); iter != children.end(); ++iter) {
        if ((*iter)->name == name) visitor.handle(*iter);
    }
}


#define LF 10
#define CR 13
#define TAB 0x09
#define SPACE 0x20

XmlReader::XmlReader(const char* filename)
        : root(0), line(1), inData(false), inComment(false)
{
    FILE* file = fopen(filename, "r");
    if (file == 0) throw Error(string("error opening file: ") + filename);
    FileHandle handle(file);

    char* buffer = 0;
    size_t bufsize = 0;
    MemHandle mem(&buffer);

    while (true) {
        ssize_t size = getline(&buffer, &bufsize, file);
        if (size == -1) break;
        if (buffer[size-2] == CR) buffer[size-2] = 0;
        if (buffer[size-1] == LF) buffer[size-1] = 0;
        readLine(&buffer[0]);
        ++line;
    }
    if (!stack.empty()) error("missing closing tag: " + stack.top()->name + " at end of file");
}


// for testing
XmlReader::XmlReader(const char* input, int)
        : root(0), line(1), inComment(false)
{
    char lineBuf[400];
    memset(lineBuf, 0, 400);

    const char* lineStart = input;
    const char* lineEnd = lineStart;
    while(true) {
        while (*lineEnd != CR && *lineEnd != LF && *lineEnd != 0) {
            ++lineEnd;
        }
        int size = lineEnd - lineStart;
        strncpy(lineBuf, lineStart, size);
        lineBuf[size] = 0;
        readLine(lineBuf); 
        ++line;
        while (*lineEnd == CR || *lineEnd == LF) ++lineEnd; //NOTE skips empty lines
        if (*lineEnd == 0) break;
        lineStart = lineEnd;
    }
    if (!stack.empty()) error("missing closing tag: " + stack.top()->name + " at end of file");
}


XmlReader::~XmlReader() {
    delete root;
}


void XmlReader::foreach(const std::string& path, XmlNodeVisitor& visitor) const {
    XPath xpath(path);
    if (xpath.size() < 2) {
        throw Error("invalid path [" +  path + "]");
    }
    if (root->getName() != xpath[0]) {
        throw Error(string("no such root [") + xpath[0] + "]");
    }
    const XmlNode* node = root;
    for (int i=1; i<xpath.size()-1; i++) {
        const XmlNode* child = node->getChild(xpath[i]);
        if (child == NULL) {
            throw Error("node [" + node->getName() + "] has no child [" + xpath[i] + "]");
        }
        node = child;
    }
    node->visitChildren(xpath[xpath.size()-1], visitor);
}


void XmlReader::error(const string& msg) {
    delete root;
    root = 0;
    throw Error(msg + " (line: " + itoa(line) + ")");
}


void XmlReader::openXmlNode(const char* name) {
    if (stack.empty()) {
        if (root) error("can only have one tag at root level");
        root = new XmlNode(name, line);
        stack.push(root);
        return;
    }
    XmlNode* current = stack.top();
    switch (current->type) {
    case XmlNode::TBD:
        current->type = XmlNode::NODE;
        break;
    case XmlNode::NODE:
        break;
    case XmlNode::LEAF:
        error("tag [" + current->name + "] cannot have children.");
    }
    XmlNode* child = new XmlNode(name, line);
    current->addChild(child);
    stack.push(child);
}


void XmlReader::closeXmlNode(const char* name) {
    if (stack.empty()) error("closing tag without opening tag");
    if (strcmp(stack.top()->name.c_str(), name)!=0) error("wrong closing tag, expected: [" + stack.top()->name + "]");
    stack.pop();
}


void XmlReader::handleData(char* data) {
    if (stack.empty()) error("expected root tag");
    XmlNode* current = stack.top();
    switch (current->type) {
    case XmlNode::TBD:
        current->type = XmlNode::LEAF;
    case XmlNode::LEAF:		//FALLTHROUGH
        if (inData) current->text += "\n";
        current->text += data;
        break;
    case XmlNode::NODE:
        error("tag [" + current->name + "] cannot have text");
    }
}


void XmlReader::readEnv(char** ipp) {
    char* ip = *ipp;
    while (*ip != 0) {
        if (*ip == '?' && ip[1] == '>') {
            ip += 2;
            *ipp = ip;
            return;
        }
        ++ip;
    }
    error("env does not end on one line");
}


void XmlReader::readComment(char** ipp) {
    char* ip = *ipp;
    while (*ip != 0) {
        if (*ip == '-' && ip[1] == '-' && ip[2] == '>') {
            inComment = false;
            ip += 3;
            break;
        }
        ++ip;
    }
    *ipp = ip;
}


void XmlReader::skipWhiteSpace(char** ipp) {
    char* ip = *ipp;
    while (*ip != 0 && (*ip == SPACE || *ip == TAB)) ++ip;
    *ipp = ip;
}


void XmlReader::readAttributeName(char** ipp, char* op) {
    char* ip = *ipp;
    while (*ip != 0) {
        switch (*ip) {
        case '<':
        case '>':
        case SPACE:
        case '/':
            error("syntax error in tag attribute");
        case '=': {
            int size = ip - *ipp;
            memcpy(op, *ipp, size);
            op[size] = 0;
            *ipp = ip;
            return;
            }
        default:
            ++ip;
        }
    }
    error("attribute reaches end of line");
}


void XmlReader::readAttributeValue(char** ipp, char* op, char quote) {
    char* ip = *ipp;
    while (*ip != 0) {
        switch (*ip) {
        case '"':
        case '\'':  // FALLTHROUGH
            if (*ip == quote) {
                int size = ip - *ipp;
                memcpy(op, *ipp, size);
                op[size] = 0;
                *ipp = ip;
                return;
            }
        default:
            ++ip;
        }
    }
    error("attribute reaches end of line");
}


void XmlReader::readAttribute(char** ipp) {
    char* ip = *ipp;

    readAttributeName(&ip, attrName);
    char quote = ip[1];
    if (*ip != '=' || (quote != '\'' && quote != '"')) error("error in tag attribute, expected ' or \"");
    ip += 2;
    readAttributeValue(&ip, attrValue, quote);
    if (*ip != quote) 
        error(string("error in tag value, expected: ") + quote);
    ++ip;

    XmlNode* current = stack.top();
    if (current->hasAttribute(attrName)) error(string("duplicate attribute: ") + attrName);
        
    current->addAttribute(attrName, attrValue);
    *ipp = ip;
}


void XmlReader::readXmlNodeName(char** ipp, char* op) {
    char* ip = *ipp;
    while (*ip != 0) {
        switch (*ip) {
        case '<':
            error("cannot have '<' in tag name");
        case '>':
        case '/':
        case SPACE: {
            int size = ip - *ipp;
            memcpy(op, *ipp, size);
            op[size] = 0;
            *ipp = ip;
            return;
        }
        default:
            ++ip;
        }
    }
    error("tag reaches end of line");
}


void XmlReader::readXmlNode(char** ipp) {
    char* ip = *ipp;
    bool isStartXmlNode = true;
    if (*ip == '/') {
        isStartXmlNode = false;
        ++ip;
    }
    readXmlNodeName(&ip, nameBuf);
    if (isStartXmlNode) openXmlNode(nameBuf);
    else closeXmlNode(nameBuf);

    while (*ip != 0) {
        switch (*ip) {
        case SPACE:
            ++ip;
            break;
        case '/':
            if (!isStartXmlNode || ip[1] != '>') error("wrong closing tag");
            closeXmlNode(nameBuf);
            ++ip;
        case '>':	// FALLTHROUGH
            ++ip;
            *ipp = ip;
            return;
        case '<':
            error("already inside tag");
        default:
            if (!isStartXmlNode) error("end tag cannot have attributes");
            readAttribute(&ip);
        }
    }
    error("multi-line tags not supported yet");
}


void XmlReader::readLine(char* ip) {
    char* dataStart = ip;
    if (inComment) {
        readComment(&ip);
        dataStart = ip;
    }
    if (!inComment) {
        skipWhiteSpace(&ip);
        dataStart = ip;
    }
    while (*ip != 0) {
        switch (*ip) {
        case '<':
            if (ip != dataStart) {
                int size = ip - dataStart;
                memcpy(dataBuf, dataStart, size);
                dataBuf[size] = 0;
                handleData(dataBuf);
            }
            inData = false;
            if (ip[1] == '!' && ip[2] == '-' && ip[3] == '-') {
                ip += 4;
                inComment = true;
                readComment(&ip);
                dataStart = ip;
                continue;
            }
            if (ip[1] == '?') {
                ip += 2;
                readEnv(&ip);
                dataStart = ip;
                continue;
            }
            ++ip;
            readXmlNode(&ip);
            dataStart = ip;
            continue;
        default:
            break;
        }
        ++ip;
    }
    if (ip != dataStart) {
        int size = ip - dataStart;
        memcpy(dataBuf, dataStart, size);
        dataBuf[size] = 0;
        handleData(dataBuf);
        inData = true;
    }
}

