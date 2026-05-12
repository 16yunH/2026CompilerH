#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "quad.hh"
#include "xml2quad.hh"
#include "opt.hh"

using namespace std;
using namespace quad;
using namespace tinyxml2;

int main(int argc, const char *argv[]) {
    string file;

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " filename" << endl;
        return EXIT_FAILURE;
    }

    file = argv[1];

    // boilerplate in/output filenames (used throughout the compiler pipeline)
    string file_quad_ssa_xml = file + ".4-ssa-xml.quad";
    string file_quad_ssa_opt = file + ".4-ssa-opt.quad";

    cout << "Reading Quad (SSA) from xml: " << file_quad_ssa_xml << endl;
    quad::QuadProgram *x3 = xml2quad(file_quad_ssa_xml.c_str());
    if (x3 == nullptr) {
        cerr << "Error reading Quad from xml: " << file_quad_ssa_xml << endl;
        return EXIT_FAILURE;
    }
    QuadProgram *x4 = optProg(x3); //using registers to prepare the quad for RPI
    cout << "Writing optimized Quad to file: " << file_quad_ssa_opt << endl;
    ofstream ofs(file_quad_ssa_opt);
    if (!ofs) {
        cerr << "Error opening output file: " << file_quad_ssa_opt << endl;
        return EXIT_FAILURE;
    }
    string output_str;
    x4->print(output_str, 0, false);
    ofs << output_str;
    ofs.close();
    cout << "-----Done---" << endl;
    return EXIT_SUCCESS;
}