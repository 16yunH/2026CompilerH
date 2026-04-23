#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "quad.hh"
#include "quad2xml.hh"
#include "xml2quad.hh"
#include "quadssa.hh"
#include "flowinfo.hh"
#include "flow2xml.hh"
#include "xml2flow.hh"

using namespace std;
using namespace tree;
using namespace quad;
using namespace tinyxml2;

int main(int argc, const char *argv[]) {
    string file;

    const bool debug = argc > 1 && std::strcmp(argv[1], "--debug") == 0;

    if ((!debug && argc != 2) || (debug && argc != 3)) {
        cerr << "Usage: " << argv[0] << " [--debug] filename" << endl;
        return EXIT_FAILURE;
    }
    file = argv[argc - 1];

    // boilerplate output filenames (used throughout the compiler pipeline)
    string file_quadwithflow_xml = file + ".4-quadwithflow-xml.quad";
    string file_quad_ssa = file + ".4-ssa.quad";

    std::cout << "Reading quadwithflow from xml: " << file_quadwithflow_xml << std::endl;
    std::set<FuncFlowInfo*>* ffiFromXml = xml2flow(file_quadwithflow_xml.c_str());
    if (!ffiFromXml) {
        std::cerr << "Error reading quadwithflow XML file: " << file_quadwithflow_xml << std::endl;
        return EXIT_FAILURE;
    }
    cout << "Converting Quad to Quad-SSA" << endl;
    //quad2ssa assumes blocked quad!!!
    QuadProgram *x5 = quad2ssa(ffiFromXml);
    if (x5 == nullptr) {
        cerr << "Error converting Quad to Quad-SSA" << endl;
        return EXIT_FAILURE;
    }
    cout << "Done converting Quad to Quad-SSA" << endl;
    cout << "Writing Quad-SSA to: " << file_quad_ssa<< endl;
    ofstream out_ssa(file_quad_ssa);
    if (!out_ssa) {
        cerr << "Error opening file: " << file_quad_ssa << endl;
        return EXIT_FAILURE;
    }
    string temp_str;
    temp_str.clear(); temp_str.reserve(10000);
    x5->print(temp_str, 0, true);
    out_ssa << temp_str;
    out_ssa.flush(); out_ssa.close();
    cout << "Done writing Quad-SSA (text) to: " << file_quad_ssa << endl;
    cout << "-----Done---" << endl;
    return EXIT_SUCCESS;
}