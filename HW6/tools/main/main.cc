#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "quad.hh"
#include "xml2quad.hh"
#include "flow2xml.hh"
#include "blocking.hh"
#include "flowinfo.hh"

using namespace std;
using namespace tree;
using namespace quad;
using namespace tinyxml2;

int main(int argc, const char *argv[]) {
    std::string file;

    const bool debug = argc > 1 && std::strcmp(argv[1], "--debug") == 0;

    if ((!debug && argc != 2) || (debug && argc != 3)) {
        std::cerr << "Usage: " << argv[0] << " [--debug] filename" << std::endl;
        return EXIT_FAILURE;
    }
    file = argv[argc - 1];

    // boilerplate output filenames (used throughout the compiler pipeline)
    std::string file_quad_xml = file + ".4-xml.quad";
    std::string file_quad_block = file + ".4-block.quad";
    string file_quadwithflow_xml = file + ".4-quadwithflow-xml.quad";

    // Read the quad program from the XML file
    std::cout << "Reading Quad from xml: " << file_quad_xml << std::endl;
    quad::QuadProgram *x3 = xml2quad(file_quad_xml.c_str());
    if (x3 == nullptr) {
        std::cerr << "Error reading Quad from XML file: " << file_quad_xml << std::endl;
        return EXIT_FAILURE;
    }
    // Block the quad program
    QuadProgram* x4 = blocking(x3);
    if (x4 == nullptr) {
        std::cerr << "Error blocking the Quad program" << std::endl;
        return EXIT_FAILURE;
    }

    // Write the blocked Quad to a text file for debugging purposes
    std::ofstream out2(file_quad_block);
    if (!out2) {
        std::cerr << "Error opening file: " << file_quad_block << std::endl;
        return EXIT_FAILURE;
    }
    std::string temp_str; temp_str.reserve(100000); // Reserve space to avoid multiple reallocations
    x4->print(temp_str, 0, true);
    out2 << temp_str;
    out2.flush(); out2.close();

    // Now calculate data flow information for the blocked quad program
    std::cout << "Computing data flow information" << std::endl;
    std::set<DataFlowInfo*>* allDfi = dataFLowProg(x4);
    std::set<FuncFlowInfo*>* allFuncFlow = new std::set<FuncFlowInfo*>();
    // For each function, compute its control flow information 
    /**/
    for (auto* dfItem : *allDfi) {
        if (!dfItem) continue;
        dfItem->computeLiveness();
        std::string liveness_str = dfItem->printLiveness();
        std::cout << "Liveness information for function: " << dfItem->func->funcname << std::endl;
        std::cout << liveness_str << std::endl;
        ControlFlowInfo* cfi = new ControlFlowInfo(dfItem->func);
        cfi->computeEverything();
        cfi->printEverything();
        allFuncFlow->insert(new FuncFlowInfo(cfi, dfItem));
    }

    if (!flowinfo2xml(allFuncFlow, file_quadwithflow_xml.c_str())) {
        std::cerr << "Error writing flow info XML file: " << file_quadwithflow_xml << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Done control flow information computing" << std::endl;
    return EXIT_SUCCESS;
}