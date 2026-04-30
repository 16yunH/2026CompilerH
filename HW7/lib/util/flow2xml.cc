#include <map>
#include <set>
#include <vector>

#include "flowinfo.hh"
#include "quad2xml.hh"
#include "flow2xml.hh"
#include "tinyxml2.hh"

using namespace std;
using namespace tinyxml2;
using namespace quad;

namespace {

void appendSetMap(XMLDocument* doc, XMLElement* parent, const char* name, const map<int, set<int>>& m) {
    XMLElement* section = doc->NewElement(name);
    for (const auto& kv : m) {
        XMLElement* blockElem = doc->NewElement("Block");
        blockElem->SetAttribute("id", kv.first);
        for (int v : kv.second) {
            XMLElement* item = doc->NewElement("Item");
            item->SetAttribute("id", v);
            blockElem->InsertEndChild(item);
        }
        section->InsertEndChild(blockElem);
    }
    parent->InsertEndChild(section);
}

void appendImmediateDom(XMLDocument* doc, XMLElement* parent, const map<int, int>& m) {
    XMLElement* section = doc->NewElement("ImmediateDominators");
    for (const auto& kv : m) {
        XMLElement* blockElem = doc->NewElement("Block");
        blockElem->SetAttribute("id", kv.first);
        blockElem->SetAttribute("idom", kv.second);
        section->InsertEndChild(blockElem);
    }
    parent->InsertEndChild(section);
}

void appendLiveness(XMLDocument* doc, XMLElement* funcElem, DataFlowInfo* dfi) {
    XMLElement* livenessElem = doc->NewElement("Liveness");

    if (!dfi || !dfi->func || !dfi->func->quadblocklist) {
        funcElem->InsertEndChild(livenessElem);
        return;
    }

    for (auto block : *dfi->func->quadblocklist) {
        if (!block || !block->entry_label || !block->quadlist) {
            continue;
        }

        XMLElement* blockElem = doc->NewElement("Block");
        blockElem->SetAttribute("id", block->entry_label->num);

        for (auto stmt : *block->quadlist) {
            if (!stmt) {
                continue;
            }

            XMLElement* stmtElem = doc->NewElement("Statement");
            stmtElem->SetAttribute("kind", quadKindToString(stmt->kind).c_str());

            XMLElement* liveInElem = doc->NewElement("LiveIn");
            auto liveInIt = dfi->livein->find(stmt);
            if (liveInIt != dfi->livein->end()) {
                for (int v : liveInIt->second) {
                    XMLElement* varElem = doc->NewElement("Var");
                    varElem->SetAttribute("id", v);
                    liveInElem->InsertEndChild(varElem);
                }
            }

            XMLElement* liveOutElem = doc->NewElement("LiveOut");
            auto liveOutIt = dfi->liveout->find(stmt);
            if (liveOutIt != dfi->liveout->end()) {
                for (int v : liveOutIt->second) {
                    XMLElement* varElem = doc->NewElement("Var");
                    varElem->SetAttribute("id", v);
                    liveOutElem->InsertEndChild(varElem);
                }
            }

            stmtElem->InsertEndChild(liveInElem);
            stmtElem->InsertEndChild(liveOutElem);
            blockElem->InsertEndChild(stmtElem);
        }

        livenessElem->InsertEndChild(blockElem);
    }

    funcElem->InsertEndChild(livenessElem);
}

} // namespace

bool flowinfo2xml(set<FuncFlowInfo*>* allFuncFlow, const char* filename) {
    if (!allFuncFlow || !filename) {
        return false;
    }

    XMLDocument doc;
    XMLElement* root = doc.NewElement("QuadWithFlowProgram");
    doc.InsertEndChild(root);

    // Embed full blocked quad program so xml2flow can recover exact statements.
    vector<QuadFuncDecl*>* funcs = new vector<QuadFuncDecl*>();
    funcs->reserve(allFuncFlow->size());
    int maxLastLabelNum = 0;
    int maxLastTempNum = 0;
    for (auto* ffi : *allFuncFlow) {
        if (!ffi || !ffi->dfi || !ffi->dfi->func) {
            continue;
        }
        QuadFuncDecl* f = ffi->dfi->func;
        funcs->push_back(f);
        if (f->last_label_num > maxLastLabelNum) {
            maxLastLabelNum = f->last_label_num;
        }
        if (f->last_temp_num > maxLastTempNum) {
            maxLastTempNum = f->last_temp_num;
        }
    }
    root->SetAttribute("program_last_label_num", maxLastLabelNum);
    root->SetAttribute("program_last_temp_num", maxLastTempNum);

    if (!funcs->empty()) {
        QuadProgram embeddedProg(funcs, maxLastLabelNum, maxLastTempNum);
        XMLDocument quadDoc;
        Quad2XML q2x(&quadDoc);
        embeddedProg.accept(q2x);
        if (quadDoc.RootElement()) {
            XMLNode* cloned = quadDoc.RootElement()->DeepClone(&doc);
            if (cloned) {
                root->InsertEndChild(cloned);
            }
        }
    }

    for (auto* ffi : *allFuncFlow) {
        if (!ffi || !ffi->cfi || !ffi->dfi || !ffi->dfi->func) {
            continue;
        }

        XMLElement* funcElem = doc.NewElement("Function");
        funcElem->SetAttribute("name", ffi->dfi->func->funcname.c_str());

        XMLElement* cfgElem = doc.NewElement("ControlFlow");
        appendSetMap(&doc, cfgElem, "Predecessors", ffi->cfi->predecessors);
        appendSetMap(&doc, cfgElem, "Successors", ffi->cfi->successors);
        appendSetMap(&doc, cfgElem, "Dominators", ffi->cfi->dominators);
        appendImmediateDom(&doc, cfgElem, ffi->cfi->immediateDominator);
        appendSetMap(&doc, cfgElem, "DominanceFrontier", ffi->cfi->dominanceFrontiers);
        appendSetMap(&doc, cfgElem, "DominatorTree", ffi->cfi->domTree);
        funcElem->InsertEndChild(cfgElem);

        appendLiveness(&doc, funcElem, ffi->dfi);

        root->InsertEndChild(funcElem);
    }

    return doc.SaveFile(filename) == XML_SUCCESS;
}
