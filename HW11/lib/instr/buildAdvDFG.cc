#include "advDFG.hh"

#include <algorithm>
#include <set>
#include <unordered_map>
#include <vector>

namespace instr {

static int tempNum(tree::Temp *temp) {
    return temp == nullptr ? -1 : temp->num;
}

static int firstDefTemp(const quad::QuadStm *stm) {
    if (stm == nullptr || stm->def == nullptr || stm->def->empty()) {
        return -1;
    }

    int out = -1;
    for (auto *temp : *stm->def) {
        int num = tempNum(temp);
        if (num >= 0 && (out < 0 || num < out)) {
            out = num;
        }
    }
    return out;
}

static std::set<int> usedTemps(const quad::QuadStm *stm) {
    std::set<int> out;
    if (stm == nullptr || stm->use == nullptr) {
        return out;
    }

    for (auto *temp : *stm->use) {
        int num = tempNum(temp);
        if (num >= 0) {
            out.insert(num);
        }
    }
    return out;
}

static bool isExitStatement(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    return stm->kind == quad::QuadKind::JUMP ||
           stm->kind == quad::QuadKind::CJUMP ||
           stm->kind == quad::QuadKind::RETURN;
}

static bool touchesMemoryOrControl(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }

    switch (stm->kind) {
        case quad::QuadKind::LOAD:
        case quad::QuadKind::STORE:
        case quad::QuadKind::CALL:
        case quad::QuadKind::MOVE_CALL:
        case quad::QuadKind::EXTCALL:
        case quad::QuadKind::MOVE_EXTCALL:
        case quad::QuadKind::JUMP:
        case quad::QuadKind::CJUMP:
        case quad::QuadKind::RETURN:
            return true;
        default:
            return false;
    }
}

static advDFGNode *makeNode(const quad::QuadStm *stm, int &nextChain) {
    NodeType type = NodeType::Statement;
    if (stm != nullptr && stm->kind == quad::QuadKind::LABEL) {
        type = NodeType::EntryLabel;
    } else if (isExitStatement(stm)) {
        type = NodeType::ExitStatement;
    }

    auto *node = new advDFGNode(type, stm);
    node->tempDefined = firstDefTemp(stm);
    node->tempsUsed = usedTemps(stm);

    if (touchesMemoryOrControl(stm)) {
        node->chainUsed = nextChain - 1;
        node->chainDefined = nextChain++;
    }

    return node;
}

advDFGprog *buildAdvDFGprog(const quad::QuadProgram *program) {
    auto *out = new advDFGprog(program);
    if (program == nullptr || program->quadFuncDeclList == nullptr) {
        return out;
    }

    for (auto *func : *program->quadFuncDeclList) {
        auto *funcGraph = new advDFGfunc(func);
        out->addFunc(funcGraph);

        if (func == nullptr || func->quadblocklist == nullptr) {
            continue;
        }

        for (auto *block : *func->quadblocklist) {
            auto *blockGraph = new advDFGblock(block);
            funcGraph->addBlock(blockGraph);

            if (block == nullptr || block->quadlist == nullptr) {
                continue;
            }

            std::unordered_map<int, advDFGNode*> lastTempDef;
            advDFGNode *lastChainDef = nullptr;
            int nextChain = 0;

            for (auto *stm : *block->quadlist) {
                if (stm == nullptr) {
                    continue;
                }

                auto *node = makeNode(stm, nextChain);
                blockGraph->graph.addNode(node);

                for (int temp : node->tempsUsed) {
                    auto it = lastTempDef.find(temp);
                    if (it != lastTempDef.end() && it->second != nullptr && it->second != node) {
                        blockGraph->graph.addEdge(it->second, node);
                    }
                }

                if (node->chainUsed >= 0 && lastChainDef != nullptr && lastChainDef != node) {
                    blockGraph->graph.addEdge(lastChainDef, node);
                }

                if (node->tempDefined >= 0) {
                    lastTempDef[node->tempDefined] = node;
                }
                if (node->chainDefined >= 0) {
                    lastChainDef = node;
                }
            }

            const auto &nodes = blockGraph->graph.getNodes();
            advDFGNode *entryNode = nullptr;
            for (auto *node : nodes) {
                if (node != nullptr && node->type == NodeType::EntryLabel) {
                    entryNode = node;
                    break;
                }
            }

            if (entryNode != nullptr) {
                for (auto *node : nodes) {
                    if (node != nullptr && node != entryNode && node->predecessors.empty()) {
                        blockGraph->graph.addEdge(entryNode, node);
                    }
                }
            }
        }
    }

    return out;

}

} // namespace instr
