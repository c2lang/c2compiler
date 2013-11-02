/*
    xml2dot:
    tool to convert generated dependency xml files into dot format
*/

#include <stdio.h>
#include <map>
#include <string>

#include "XmlReader.h"
#include "StringBuilder.h"

using namespace C2;
using namespace std;

class XmlFileVisitor : public XmlNodeVisitor {
public:
    XmlFileVisitor(StringBuilder& output_)
        : output(output_)
        , count(1)
        , state(NODES)
    {}
    virtual void handle(const XmlNode* node) {
        // xml:  <file name="layer1.0">
        if (state == NODES) {
            // generate: 1[label="name"];
            const string& name = node->getAttribute("name");
            output << count << "[label=\"" << name << "\"];\n";
            nodes[name] = count;
        } else {
            // find all uses
            const XmlNode::Children& children = node->getChildren();
            XmlNode::ChildrenConstIter iter = children.begin();
            while (iter != children.end()) {
                const XmlNode* child = (*iter);
                if (child->getName() == "uses") {
                    // xml: <uses file="layer1.0" strength="1" />
                    const string& file = child->getAttribute("file");
                    NodesIter iter = nodes.find(file);
                    if (iter == nodes.end()) throw Error("unknown file in dependency");

                    // generate: 1->31;
                    output << count << "->" << iter->second << ";\n";
                }
                ++iter;
            }
        }
        count++;
    }
    void links() {
        state = LINKS;
        count = 1;
    }
private:
    StringBuilder& output;
    unsigned count;
    enum State { NODES, LINKS };
    State state;
    typedef std::map<std::string, unsigned> Nodes;
    typedef Nodes::iterator NodesIter;
    Nodes nodes;
};

int main(int argc, char* argv[]) {
	if (argc != 2) {
        printf("Usage: %s [xmlfile]\n", argv[0]);
        return -1;
    }

    const char* filename = argv[1];

	try {
        string msg = filename;
        msg += ": ";
	    XmlReader reader(filename);
        XmlNode* root = reader.getRootXmlNode();
        if (!root) throw Error(msg + "is not correct XML.");
        if (root->getName() != "dependencies")
            throw Error(msg + "expected 'dependencies' tag at root");

        StringBuilder output;
        // generate header
        output << "digraph G {\n";
        output << "node [shape=box]\n";

        // generate nodes
        XmlFileVisitor visitor(output);
        root->visitChildren("file", visitor);

        // generate links
        visitor.links();
        root->visitChildren("file", visitor);

        // generate end
        output << "}\n";

        printf("%s", (const char*)output);
	} catch (std::exception& e) {
		printf("ERROR: %s\n", e.what());
	}

    return 0;
}

