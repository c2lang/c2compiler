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

#ifndef XMLREADER_H
#define XMLREADER_H

#include <string>
#include <deque>
#include <map>
#include <stack>
#include <exception>

namespace C2 {

class Error : public std::exception {
public:
    Error(const std::string& m) : msg(m) {}
    virtual ~Error() throw() {}
    virtual const char* what() const throw();
protected:
    std::string msg;
};

class XmlNode;

class XmlNodeVisitor {
public:
    virtual ~XmlNodeVisitor() {}
    virtual void handle(const XmlNode* node) = 0;
};


class XmlNode {
public:
    XmlNode(const std::string& name_, unsigned int line_);
    XmlNode(const std::string& name_, const std::string& text_, unsigned int line_);
    ~XmlNode();
    const std::string& getName() const { return name; }
    const std::string& getText() const { return text; }
    unsigned int getLine() const { return line; }

    typedef std::deque<XmlNode*> Children;
    typedef Children::const_iterator ChildrenConstIter;

    bool hasChild(const std::string& key) const;
    const XmlNode* getChild(const std::string& key) const;
    const Children& getChildren() const { return children; }
    int getChildLine(const std::string& key) const;
    const std::string& getChildValue(const std::string& key) const;
	unsigned int getChildUIntValue(const std::string& key) const;

    bool hasAttribute(const std::string& key) const;
    const std::string& getAttribute(const std::string& key) const;
    const std::string& getOptionalAttribute(const std::string& key) const;
	unsigned int getUIntAttribute(const std::string& key) const;

    void addChild(XmlNode* child);
    void addAttribute(const std::string& key, const std::string& value);

    // visit all children named 'name'
    void visitChildren(const std::string& name, XmlNodeVisitor& visitor) const;
private:
    friend class XmlReader;

    std::string name;
    std::string text;
    enum XmlNodeType {TBD=0, LEAF=1, NODE=2};
    XmlNodeType type;
    unsigned int line;

    Children children;

    typedef std::map<std::string, std::string> Attributes;
    typedef Attributes::const_iterator AttributesConstIter;
    Attributes attributes;

    XmlNode(const XmlNode&);
    XmlNode& operator= (const XmlNode&);
};


class XmlReader {
public:
    XmlReader(const char* filename);
    XmlReader(const char* filename, int); //FOR TESTING
    virtual ~XmlReader();
    XmlNode* getRootXmlNode() const { return root; }
    void foreach(const std::string& path, XmlNodeVisitor& visitor) const;
protected:
    void error(const std::string& msg);

    void openXmlNode(const char* name);
    void closeXmlNode(const char* name);

    void handleData(char* data);
    void readEnv(char** ipp);
    void readComment(char** ipp);
    void skipWhiteSpace(char** ipp);

    void readAttributeName(char** ipp, char* op);
    void readAttributeValue(char** ipp, char* op, char quote);
    void readAttribute(char** ipp);
    void readXmlNodeName(char** ipp, char* op);
    void readXmlNode(char** ipp);
    void readLine(char* ip);

    XmlReader(const XmlReader&);
    XmlReader& operator= (const XmlReader&);

    XmlNode* root;

    typedef std::stack<XmlNode*> Stack;
    Stack stack;
    unsigned int line;
    bool inData;
    bool inComment;
    char nameBuf[200];
    char dataBuf[400];
    char attrName[200];
    char attrValue[200];
};

}

#endif

