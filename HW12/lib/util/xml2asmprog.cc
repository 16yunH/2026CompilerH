#include "xml2asmprog.hh"
#include "tinyxml2.hh"
#include "temp.hh"
#include <iostream>
#include <map>

namespace instr {

using namespace tinyxml2;

AsmProg* xml2asmprog(const char* filename) {
    XMLDocument doc;
    XMLError eResult = doc.LoadFile(filename);
    
    if (eResult != XML_SUCCESS) {
        std::cerr << "Error loading XML file: " << filename << std::endl;
        return nullptr;
    }

    AsmProg* program = new AsmProg();
    
    // Map to keep track of created Temps and Labels so we can reuse them
    std::map<int, tree::Temp*> tempMap;
    std::map<int, tree::Label*> labelMap;
    
    XMLElement* scheduleProgElem = doc.FirstChildElement("ScheduleProgram");
    if (!scheduleProgElem) {
        std::cerr << "Error: ScheduleProgram element not found" << std::endl;
        delete program;
        return nullptr;
    }

    // Iterate through all Function elements
    for (XMLElement* funcElem = scheduleProgElem->FirstChildElement("Function");
         funcElem != nullptr;
         funcElem = funcElem->NextSiblingElement("Function")) {
        
        const char* funcName = funcElem->Attribute("name");
        if (!funcName) {
            std::cerr << "Error: Function name attribute not found" << std::endl;
            continue;
        }

        AsmFunction asmFunc(funcName);

        // Iterate through all Instruction elements within this function
        for (XMLElement* instrElem = funcElem->FirstChildElement("Instruction");
             instrElem != nullptr;
             instrElem = instrElem->NextSiblingElement("Instruction")) {
            
            const char* assemStr = instrElem->Attribute("assem");
            if (!assemStr) {
                std::cerr << "Warning: Instruction assem attribute not found" << std::endl;
                continue;
            }

            AssemInstr instr;
            instr.assem = assemStr;
            instr.kind = AssemInstr::I_OPER; // Default kind
            
            const char* kind = instrElem->Attribute("kind");
            if (kind) {
                std::string kindStr(kind);
                if (kindStr == "I_LABEL") {
                    instr.kind = AssemInstr::I_LABEL;
                } else if (kindStr == "I_MOVE") {
                    instr.kind = AssemInstr::I_MOVE;
                } else if (kindStr == "I_CALL") {
                    instr.kind = AssemInstr::I_CALL;
                } else if (kindStr == "I_EXTCALL") {
                    instr.kind = AssemInstr::I_EXTCALL;
                } else if (kindStr == "I_OPER") {
                    instr.kind = AssemInstr::I_OPER;
                }
            }

            // Parse Label (for I_LABEL instructions)
            if (instr.kind == AssemInstr::I_LABEL) {
                XMLElement* labelElem = instrElem->FirstChildElement("Label");
                if (labelElem) {
                    const char* numStr = labelElem->Attribute("num");
                    if (numStr) {
                        int labelNum = std::atoi(numStr);
                        // Create or reuse Label object
                        if (labelMap.find(labelNum) == labelMap.end()) {
                            labelMap[labelNum] = new tree::Label(labelNum);
                        }
                        instr.label = labelMap[labelNum];
                    }
                }
            }

            // Parse Dst (destination temporaries)
            XMLElement* dstElem = instrElem->FirstChildElement("Dst");
            if (dstElem) {
                for (XMLElement* tempElem = dstElem->FirstChildElement("Temp");
                     tempElem != nullptr;
                     tempElem = tempElem->NextSiblingElement("Temp")) {
                    
                    const char* numStr = tempElem->Attribute("num");
                    if (numStr) {
                        int tempNum = std::atoi(numStr);
                        // Create or reuse Temp object
                        if (tempMap.find(tempNum) == tempMap.end()) {
                            tempMap[tempNum] = new tree::Temp(tempNum);
                        }
                        instr.dst.push_back(tempMap[tempNum]);
                    }
                }
            }

            // Parse Src (source temporaries)
            XMLElement* srcElem = instrElem->FirstChildElement("Src");
            if (srcElem) {
                for (XMLElement* tempElem = srcElem->FirstChildElement("Temp");
                     tempElem != nullptr;
                     tempElem = tempElem->NextSiblingElement("Temp")) {
                    
                    const char* numStr = tempElem->Attribute("num");
                    if (numStr) {
                        int tempNum = std::atoi(numStr);
                        // Create or reuse Temp object
                        if (tempMap.find(tempNum) == tempMap.end()) {
                            tempMap[tempNum] = new tree::Temp(tempNum);
                        }
                        instr.src.push_back(tempMap[tempNum]);
                    }
                }
            }

            // Parse Jumps (jump labels)
            XMLElement* jumpsElem = instrElem->FirstChildElement("Jumps");
            if (jumpsElem) {
                for (XMLElement* labelElem = jumpsElem->FirstChildElement("Label");
                     labelElem != nullptr;
                     labelElem = labelElem->NextSiblingElement("Label")) {
                    
                    const char* numStr = labelElem->Attribute("num");
                    if (numStr) {
                        int labelNum = std::atoi(numStr);
                        // Create or reuse Label object
                        if (labelMap.find(labelNum) == labelMap.end()) {
                            labelMap[labelNum] = new tree::Label(labelNum);
                        }
                        instr.jumps.labels.push_back(labelMap[labelNum]);
                    }
                }
            }

            asmFunc.instructions.push_back(instr);
        }

        program->functions.push_back(asmFunc);
    }

    return program;
}

} // namespace instr

