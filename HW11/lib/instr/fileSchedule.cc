#include "schedule.hh"

#include <algorithm>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace instr {

static std::string assemblyLineForInstr(const AssemInstr &instr) {
    switch (instr.kind) {
        case AssemInstr::I_LABEL:
            return (instr.label == nullptr) ? std::string() : (instr.label->str() + ":");
        case AssemInstr::I_MOVE:
        case AssemInstr::I_OPER:
        case AssemInstr::I_CALL:
        case AssemInstr::I_EXTCALL:
            return "    " + instr.assem;
    }
    return instr.assem;
}

static std::string tempNameAt(const std::vector<tree::Temp*> &temps, size_t index) {
    if (index >= temps.size() || temps[index] == nullptr) {
        return "";
    }
    return temps[index]->str();
}

static std::string labelNameAt(const std::vector<tree::Label*> &labels, size_t index) {
    if (index >= labels.size() || labels[index] == nullptr) {
        return "";
    }
    return labels[index]->str();
}

static std::string renderAssemblyText(const AssemInstr &instr) {
    std::string rendered;
    const std::string &assem = instr.assem;
    for (size_t i = 0; i < assem.size(); ++i) {
        if (assem[i] == '`' && i + 2 < assem.size()) {
            char kind = assem[i + 1];
            char indexChar = assem[i + 2];
            if (indexChar >= '0' && indexChar <= '9') {
                size_t index = static_cast<size_t>(indexChar - '0');
                std::string replacement;
                if (kind == 'd') {
                    replacement = tempNameAt(instr.dst, index);
                } else if (kind == 's') {
                    replacement = tempNameAt(instr.src, index);
                } else if (kind == 'j') {
                    replacement = labelNameAt(instr.jumps.labels, index);
                }

                if (!replacement.empty()) {
                    rendered += replacement;
                    i += 2;
                    continue;
                }
            }
        }

        rendered += assem[i];
    }
    return rendered;
}

static std::vector<ScheduleFunc*> orderedFunctionsForOutput(
    const std::vector<ScheduleFunc*> &funcSchedules,
    const std::string &outputBaseName
) {
    std::vector<ScheduleFunc*> ordered = funcSchedules;

    std::ifstream inFile(outputBaseName + ".4-ssa-withflow-xml.quad");
    if (!inFile.is_open()) {
        return ordered;
    }

    std::unordered_map<std::string, size_t> sourceOrder;
    std::string line;
    size_t nextOrder = 0;
    while (std::getline(inFile, line)) {
        if (line.find("<funcdecl") == std::string::npos) {
            continue;
        }
        size_t namePos = line.find("name=\"");
        if (namePos == std::string::npos) {
            continue;
        }
        namePos += 6;
        size_t endPos = line.find('"', namePos);
        if (endPos == std::string::npos) {
            continue;
        }
        std::string name = line.substr(namePos, endPos - namePos);
        if (sourceOrder.find(name) == sourceOrder.end()) {
            sourceOrder[name] = nextOrder++;
        }
    }

    if (sourceOrder.empty()) {
        return ordered;
    }

    std::stable_sort(
        ordered.begin(),
        ordered.end(),
        [&](ScheduleFunc *left, ScheduleFunc *right) {
            std::string leftName = (left == nullptr || left->quadFunc == nullptr) ? "" : left->quadFunc->funcname;
            std::string rightName = (right == nullptr || right->quadFunc == nullptr) ? "" : right->quadFunc->funcname;
            auto leftIt = sourceOrder.find(leftName);
            auto rightIt = sourceOrder.find(rightName);
            bool leftKnown = leftIt != sourceOrder.end();
            bool rightKnown = rightIt != sourceOrder.end();
            if (leftKnown != rightKnown) {
                return leftKnown;
            }
            if (!leftKnown) {
                return false;
            }
            return leftIt->second < rightIt->second;
        });

    return ordered;
}

void ScheduleProg::writeToFile(const std::string &outputBaseName) const {
    std::ofstream outFile(outputBaseName + ".s");
    if (!outFile.is_open()) {
        return;
    }

    auto orderedFuncSchedules = orderedFunctionsForOutput(funcSchedules, outputBaseName);
    for (auto *funcSchedule : orderedFuncSchedules) {
        if (funcSchedule == nullptr || funcSchedule->quadFunc == nullptr) {
            continue;
        }

        outFile << ".global " << funcSchedule->quadFunc->funcname << '\n';
        outFile << funcSchedule->quadFunc->funcname << ":\n";
        for (const auto &instr : funcSchedule->linearizedInstructions.instrs) {
            std::string line = assemblyLineForInstr(instr);
            if (!line.empty()) {
                outFile << renderAssemblyText(instr) << '\n';
            }
        }
        outFile << '\n';
    }
}

} // namespace instr
