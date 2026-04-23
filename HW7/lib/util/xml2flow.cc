#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include "quad.hh"
#include "flowinfo.hh"
#include "xml2flow.hh"
#include "xml2quad.hh"
#include "tinyxml2.hh"

using namespace std;
using namespace tinyxml2;
using namespace quad;

namespace {

set<int> parseVarSet(XMLElement* parent, const char* setName) {
    set<int> out;
    XMLElement* setElem = parent ? parent->FirstChildElement(setName) : nullptr;
    if (!setElem) {
        return out;
    }

    for (XMLElement* varElem = setElem->FirstChildElement("Var");
         varElem;
         varElem = varElem->NextSiblingElement("Var")) {
        int id = 0;
        varElem->QueryIntAttribute("id", &id);
        out.insert(id);
    }
    return out;
}

bool parseSetMap(XMLElement* parent, const char* name, map<int, set<int>>& out) {
    XMLElement* section = parent ? parent->FirstChildElement(name) : nullptr;
    if (!section) {
        return false;
    }

    out.clear();
    for (XMLElement* blockElem = section->FirstChildElement("Block");
         blockElem;
         blockElem = blockElem->NextSiblingElement("Block")) {
        int blockId = 0;
        blockElem->QueryIntAttribute("id", &blockId);
        set<int>& vals = out[blockId];

        for (XMLElement* item = blockElem->FirstChildElement("Item");
             item;
             item = item->NextSiblingElement("Item")) {
            int id = 0;
            item->QueryIntAttribute("id", &id);
            vals.insert(id);
        }
    }

    return true;
}

bool parseImmediateDom(XMLElement* parent, map<int, int>& out) {
    XMLElement* section = parent ? parent->FirstChildElement("ImmediateDominators") : nullptr;
    if (!section) {
        return false;
    }

    out.clear();
    for (XMLElement* blockElem = section->FirstChildElement("Block");
         blockElem;
         blockElem = blockElem->NextSiblingElement("Block")) {
        int blockId = 0;
        int idom = -1;
        blockElem->QueryIntAttribute("id", &blockId);
        blockElem->QueryIntAttribute("idom", &idom);
        out[blockId] = idom;
    }

    return true;
}

void printSetMap(const string& title, const map<int, set<int>>& m, bool rightArrow) {
    cout << title << endl;
    for (const auto& kv : m) {
        cout << "Block: " << kv.first << (rightArrow ? " -> " : " <- ");
        for (int v : kv.second) {
            cout << v << " ";
        }
        cout << endl;
    }
}

void printImmediate(const map<int, int>& m) {
    cout << "Immediate Dominators:" << endl;
    for (const auto& kv : m) {
        cout << "Block: " << kv.first << " -> " << kv.second << endl;
    }
}

void printLivenessFromXml(XMLElement* funcElem, const string& funcName) {
    XMLElement* liveness = funcElem ? funcElem->FirstChildElement("Liveness") : nullptr;
    if (!liveness) {
        return;
    }

    cout << "Liveness information for function: " << funcName << endl;

    for (XMLElement* blockElem = liveness->FirstChildElement("Block");
         blockElem;
         blockElem = blockElem->NextSiblingElement("Block")) {
        int blockId = 0;
        blockElem->QueryIntAttribute("id", &blockId);

        cout << "Block " << blockId << ":" << endl;

        for (XMLElement* stmtElem = blockElem->FirstChildElement("Statement");
             stmtElem;
             stmtElem = stmtElem->NextSiblingElement("Statement")) {
            const char* kind = stmtElem->Attribute("kind");
            cout << "Statement " << (kind ? kind : "") << "  Live-in:  {";

            bool first = true;
            XMLElement* liveIn = stmtElem->FirstChildElement("LiveIn");
            if (liveIn) {
                for (XMLElement* v = liveIn->FirstChildElement("Var"); v; v = v->NextSiblingElement("Var")) {
                    int id = 0;
                    v->QueryIntAttribute("id", &id);
                    if (!first) {
                        cout << ", ";
                    }
                    cout << "t" << id;
                    first = false;
                }
            }

            cout << "} Live-out: {";
            first = true;

            XMLElement* liveOut = stmtElem->FirstChildElement("LiveOut");
            if (liveOut) {
                for (XMLElement* v = liveOut->FirstChildElement("Var"); v; v = v->NextSiblingElement("Var")) {
                    int id = 0;
                    v->QueryIntAttribute("id", &id);
                    if (!first) {
                        cout << ", ";
                    }
                    cout << "t" << id;
                    first = false;
                }
            }

            cout << "}" << endl;
        }
    }
}

} // namespace

set<FuncFlowInfo*>* xml2flow(const char* filename) {
    if (!filename) {
        return nullptr;
    }

    XMLDocument doc;
    if (doc.LoadFile(filename) != XML_SUCCESS) {
        return nullptr;
    }

    XMLElement* root = doc.FirstChildElement("QuadWithFlowProgram");
    if (!root) {
        root = doc.FirstChildElement("FlowInfoProgram");
    }
    if (!root) {
        return nullptr;
    }

    int programLastLabelNum = -1;
    int programLastTempNum = -1;
    root->QueryIntAttribute("program_last_label_num", &programLastLabelNum);
    root->QueryIntAttribute("program_last_temp_num", &programLastTempNum);

    XMLElement* programElem = root->FirstChildElement("Program");
    if (!programElem) {
        // Backward compatibility with older flow XML format without embedded quad.
        return nullptr;
    }

    // Now:
    // Parse the embedded quad program to recover function and block structure for flow info.
    // This is a bit inefficient since we convert the quad to XML and then parse it back, 
    // but it allows us to reuse the existing XML format and parsing logic without needing to 
    // write custom parsing for the quad structure from the flow XML.
    QuadProgram* prog = nullptr;
    XMLPrinter printer;
    programElem->Accept(&printer);
    const char* xmlText = printer.CStr();
    if (xmlText) {
        FILE* memFile = tmpfile();
        if (memFile) {
            size_t len = std::strlen(xmlText);
            size_t wrote = std::fwrite(xmlText, 1, len, memFile);
            std::fflush(memFile);
            std::rewind(memFile);
            if (wrote == len) {
                string fdPath = "/dev/fd/" + to_string(fileno(memFile));
                prog = xml2quad(fdPath.c_str());
            }
            std::fclose(memFile);
        }
    }
    if (!prog || !prog->quadFuncDeclList) {
        return nullptr;
    }

    map<string, QuadFuncDecl*> funcByName;
    for (auto* f : *prog->quadFuncDeclList) {
        if (f) {
            funcByName[f->funcname] = f;
        }
    }

    set<FuncFlowInfo*>* allFuncFlow = new set<FuncFlowInfo*>();

    for (XMLElement* funcElem = root->FirstChildElement("Function");
         funcElem;
         funcElem = funcElem->NextSiblingElement("Function")) {
        const char* funcNameAttr = funcElem->Attribute("name");
        string funcName = funcNameAttr ? funcNameAttr : "";
        auto funcIt = funcByName.find(funcName);
        if (funcIt == funcByName.end() || !funcIt->second) {
            continue;
        }

        QuadFuncDecl* func = funcIt->second;
        DataFlowInfo* dfi = new DataFlowInfo(func);

        XMLElement* liveness = funcElem->FirstChildElement("Liveness");
        if (liveness) {
            map<int, QuadBlock*> blockByLabel;
            for (auto* block : *func->quadblocklist) {
                if (block && block->entry_label) {
                    blockByLabel[block->entry_label->num] = block;
                }
            }

            for (XMLElement* blockElem = liveness->FirstChildElement("Block");
                 blockElem;
                 blockElem = blockElem->NextSiblingElement("Block")) {
                int blockId = 0;
                blockElem->QueryIntAttribute("id", &blockId);
                auto bIt = blockByLabel.find(blockId);
                if (bIt == blockByLabel.end() || !bIt->second || !bIt->second->quadlist) {
                    continue;
                }

                QuadBlock* block = bIt->second;
                auto stmtIt = block->quadlist->begin();
                for (XMLElement* stmtElem = blockElem->FirstChildElement("Statement");
                     stmtElem && stmtIt != block->quadlist->end();
                     stmtElem = stmtElem->NextSiblingElement("Statement"), ++stmtIt) {
                    QuadStm* stmt = *stmtIt;
                    if (!stmt) {
                        continue;
                    }

                    set<int> lin = parseVarSet(stmtElem, "LiveIn");
                    set<int> lout = parseVarSet(stmtElem, "LiveOut");
                    (*dfi->livein)[stmt] = lin;
                    (*dfi->liveout)[stmt] = lout;
                    for (int v : lin) dfi->allVars.insert(v);
                    for (int v : lout) dfi->allVars.insert(v);
                }
            }
        }

        dfi->printLiveness();

        // Build ControlFlowInfo from the skeleton func and populate maps from XML
        ControlFlowInfo* cfi = new ControlFlowInfo(func);

        XMLElement* cfgElem = funcElem->FirstChildElement("ControlFlow");
        if (cfgElem) {
            parseSetMap(cfgElem, "Predecessors", cfi->predecessors);
            parseSetMap(cfgElem, "Successors", cfi->successors);
            parseSetMap(cfgElem, "Dominators", cfi->dominators);
            parseImmediateDom(cfgElem, cfi->immediateDominator);
            parseSetMap(cfgElem, "DominanceFrontier", cfi->dominanceFrontiers);
            parseSetMap(cfgElem, "DominatorTree", cfi->domTree);

            cout << "Control Flow Information for function: " << funcName << endl;
            printSetMap("Predecessors:", cfi->predecessors, false);
            printSetMap("Successors:", cfi->successors, true);
            printSetMap("Dominators:", cfi->dominators, false);
            printImmediate(cfi->immediateDominator);
            printSetMap("Dominance Frontier:", cfi->dominanceFrontiers, true);
            printSetMap("Dominator Tree:", cfi->domTree, true);
        }

        allFuncFlow->insert(new FuncFlowInfo(cfi, dfi, programLastLabelNum, programLastTempNum));
    }

    return allFuncFlow;
}
