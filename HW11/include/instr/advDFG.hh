#ifndef ADV_DFG_HH
#define ADV_DFG_HH

#include <set>
#include <string>
#include <vector>
#include "quad.hh"

namespace instr {

// Enum for node type
enum class NodeType {
    EntryLabel,
    Statement,
    ExitStatement
};

// Structure for a node in the advanced data flow graph
struct advDFGNode {
    NodeType type; // Type of the node
    const quad::QuadStm *quadStatement; // Pointer to a QuadStm
    int tempDefined; // Temp defined by this node or chain order number
    int chainDefined; // Chain defined by this node
    std::set<int> tempsUsed; // Set of temps used by this node
    int chainUsed; // Chain used by this node, -1 when none
    std::set<advDFGNode*> predecessors; // Set of predecessor nodes
    std::set<advDFGNode*> successors;   // Set of successor nodes

    // Constructor
    advDFGNode(NodeType nodeType, const quad::QuadStm *stm = nullptr)
        : type(nodeType), quadStatement(stm), tempDefined(-1), chainDefined(-1), chainUsed(-1) {}
};

// Structure for the advanced data flow graph
class advDFG {
public:
    // Add a node to the graph
    void addNode(advDFGNode* node) {
        if (node != nullptr) {
            nodes.push_back(node);
        }
    }

    // Add an edge between two nodes
    void addEdge(advDFGNode* from, advDFGNode* to) {
        from->successors.insert(to);
        to->predecessors.insert(from);
    }

    // Get all nodes in the graph
    const std::vector<advDFGNode*>& getNodes() const {
        return nodes;
    }

    void printToStdout() const;

private:
    std::vector<advDFGNode*> nodes; // Nodes in insertion order
};

struct advDFGblock {
    const quad::QuadBlock *quadBlock; // Owning source block in quad IR
    advDFG graph; // advDFG for this block

    explicit advDFGblock(const quad::QuadBlock *block = nullptr)
        : quadBlock(block) {}
};

struct advDFGfunc {
    const quad::QuadFuncDecl *quadFunc; // Owning source function in quad IR
    std::vector<advDFGblock*> blockgraph; // Block-level advDFG records

    explicit advDFGfunc(const quad::QuadFuncDecl *func = nullptr)
        : quadFunc(func) {}

    void addBlock(advDFGblock *block) {
        if (block != nullptr) {
            blockgraph.push_back(block);
        }
    }
};

struct advDFGprog {
    const quad::QuadProgram *quadProgram; // Source quad program
    std::vector<advDFGfunc*> fungraph; // Function-level advDFG records

    explicit advDFGprog(const quad::QuadProgram *prog = nullptr)
        : quadProgram(prog) {}

    void addFunc(advDFGfunc *func) {
        if (func != nullptr) {
            fungraph.push_back(func);
        }
    }

    void printAllGraphsToStdout() const;
};

advDFGprog *buildAdvDFGprog(const quad::QuadProgram *program);

} // namespace instr

#endif // ADV_DFG_HH