#include "asmprog.hh"
#include <sstream>
#include <regex>

namespace instr {

namespace {
    // Helper function to format an instruction by replacing placeholders
    std::string format_instr(const AssemInstr& instr, const std::string& func_name) {
        std::ostringstream oss;
        std::string result = instr.assem;
        
        // Replace ^ with $ in the instruction string
        size_t pos = result.find('^');
        while (pos != std::string::npos) {
            result.replace(pos, 1, "$");
            pos = result.find('^', pos + 1);
        }
        
        // If dst/src/jumps are empty, just return the assem string as-is
        // (This happens when parsing from XML without Temp object information)
        if (instr.dst.empty() && instr.src.empty() && instr.jumps.labels.empty()) {
            // Check if this is a label (ends with :)
            bool is_label = !result.empty() && result.back() == ':';
            if (is_label) {
                // For label declarations, prefix with function name
                if (is_label && result.find(':') != std::string::npos) {
                    std::string label_text = result.substr(0, result.find(':'));
                    oss << func_name << "$" << label_text << ":";
                } else {
                    oss << result;
                }
            } else {
                oss << "\t" << result;
            }
            return oss.str();
        }
        
        // Replace `d0, `d1, etc. with actual destination temporaries
        for (size_t i = 0; i < instr.dst.size(); ++i) {
            std::string placeholder = "`d" + std::to_string(i);
            if (instr.dst[i] != nullptr) {
                // Use the temp number directly or get a register name
                // For now, just use the temp number as a string
                std::string temp_name = "r" + std::to_string(instr.dst[i]->num);
                size_t pos = result.find(placeholder);
                while (pos != std::string::npos) {
                    result.replace(pos, placeholder.length(), temp_name);
                    pos = result.find(placeholder, pos + temp_name.length());
                }
            }
        }
        
        // Replace `s0, `s1, etc. with actual source temporaries
        for (size_t i = 0; i < instr.src.size(); ++i) {
            std::string placeholder = "`s" + std::to_string(i);
            if (instr.src[i] != nullptr) {
                std::string temp_name = "r" + std::to_string(instr.src[i]->num);
                size_t pos = result.find(placeholder);
                while (pos != std::string::npos) {
                    result.replace(pos, placeholder.length(), temp_name);
                    pos = result.find(placeholder, pos + temp_name.length());
                }
            }
        }
        
        // Replace `j0, `j1, etc. with actual jump labels
        // This handles labels in jumps instructions (bne, beq, etc.)
        for (size_t i = 0; i < instr.jumps.labels.size(); ++i) {
            std::string placeholder = "`j" + std::to_string(i);
            if (instr.jumps.labels[i] != nullptr) {
                // Prefix label with function name
                std::string label_name = func_name + "$L" + std::to_string(instr.jumps.labels[i]->num);
                size_t pos = result.find(placeholder);
                while (pos != std::string::npos) {
                    result.replace(pos, placeholder.length(), label_name);
                    pos = result.find(placeholder, pos + label_name.length());
                }
            }
        }
        
        // Also handle labels in adr and str instructions that may have literal label names
        // Look for patterns like "adr `d0, labelname" and "str `s0, [`s1], labelname"
        // and prefix the label names with function name
        
        // Handle adr instructions - look for literal labels (not placeholders)
        std::regex adr_literal(R"(adr\s+\w+,\s+([A-Za-z_]\w*))");
        std::string::const_iterator searchStart(result.cbegin());
        std::smatch match;
        std::string temp_result;
        size_t last_pos = 0;
        
        while (std::regex_search(searchStart, result.cend(), match, adr_literal)) {
            std::string label_name = match[1].str();
            // Only prefix if it doesn't already contain the function name and is an L label
            if (label_name[0] == 'L' && label_name.find('$') == std::string::npos) {
                std::string prefixed_label = func_name + "$" + label_name;
                size_t match_pos = std::distance(result.cbegin(), match[0].first);
                size_t label_pos = std::distance(result.cbegin(), match[1].first);
                result = result.substr(0, label_pos) + prefixed_label + result.substr(label_pos + label_name.length());
                searchStart = result.cbegin() + label_pos + prefixed_label.length();
            } else {
                searchStart = match[0].second;
            }
        }

        // Replace "adr REG, GLOBAL$LABEL" with "ldr REG, =GLOBAL$LABEL".
        // adr is PC-relative and its offset must be an ARM 12-bit immediate
        // (8-bit value + 4-bit rotation), which excludes many distances even in ARM mode.
        // ldr REG, =label uses a literal pool and always works for any 32-bit address.
        result = std::regex_replace(result,
            std::regex(R"(adr\s+(\w+),\s+(\S*\$\S*))"),
            "ldr $1, =$2");

        // Check if this is a label (ends with :)
        bool is_label = !result.empty() && result.back() == ':';
        
        // Don't indent labels
        if (is_label) {
            oss << result;
        } else {
            oss << "\t" << result;
        }
        return oss.str();
    }
}

std::string AsmFunction::to_string() const {
    std::ostringstream oss;
    std::string func_name = name;
    // Replace ^ with $ in function name
    size_t pos = func_name.find('^');
    while (pos != std::string::npos) {
        func_name.replace(pos, 1, "$");
        pos = func_name.find('^', pos + 1);
    }
    
    // Extract clean function name: __$main__$main -> main
    // Pattern: __$NAME__$NAME, so extract the NAME part
    if (func_name.substr(0, 3) == "__$") {
        // Remove leading __$
        func_name = func_name.substr(3);
        // Find and remove trailing __$NAME pattern
        size_t underscores = func_name.find("__$");
        if (underscores != std::string::npos) {
            func_name = func_name.substr(0, underscores);
        }
    }
    
    // Add ARM assembly directives before function
    oss << ".balign 4\n";
    oss << ".global " << func_name << "\n";
    oss << ".section .text\n";
    oss << ".arm\n";  // Force ARM (32-bit) mode; spill offsets can exceed Thumb's 8-bit limit
    oss << func_name << ":\n";
    for (const auto& instr : instructions) {
        oss << format_instr(instr, func_name) << "\n";
    }
    return oss.str();
}

std::string AsmProg::to_string() const {
    std::ostringstream oss;
    for (size_t i = 0; i < functions.size(); ++i) {
        oss << functions[i].to_string();
        if (i < functions.size() - 1) {
            oss << "\n";
        }
    }
    
    // Add global declarations for external functions
    oss << "\n.global malloc\n";
    oss << ".global getint\n";
    oss << ".global getch\n";
    oss << ".global getarray\n";
    oss << ".global putint\n";
    oss << ".global putch\n";
    oss << ".global putarray\n";
    oss << ".global starttime\n";
    oss << ".global stoptime\n";
    
    return oss.str();
}

} // namespace instr
