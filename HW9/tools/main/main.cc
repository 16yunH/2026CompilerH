#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "quad.hh"
#include "xml2flow.hh"
#include "flowinfo.hh"
#include "loopopt.hh"

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
    string file_quad_ssa_withflow_xml = file + ".4-ssa-withflow-xml.quad";
    string file_quad_ssa_opt = file + ".4-ssa-loopopt.quad";

    cout << "Reading Quad (SSA) with flow info from xml: " << file_quad_ssa_withflow_xml << endl;
    set<FuncFlowInfo*>* sffi = xml2flow(file_quad_ssa_withflow_xml.c_str());
    if (sffi == nullptr) {
        cerr << "Error reading Quad from xml: " << file_quad_ssa_withflow_xml << endl;
        return EXIT_FAILURE;
    }

    vector<QuadFuncDecl*>* optimizedFuncs = new vector<QuadFuncDecl*>();

    for (FuncFlowInfo* ffi : *sffi) {
        QuadFuncDecl* func = ffi->cfi->func; // Assuming ControlFlowInfo has a pointer to the corresponding QuadFuncDecl
        LoopHeaderMap *loopHeaderMap = findLoopHeaders(func, ffi);
        cout << "Loop headers for function " << func->funcname << ": ";
        for (LoopHeader* loopHeader : loopHeaderMap->funcLoopHeaders[func])
        {
            cout << "Header Label: " << loopHeader->headerLabel << ", Body Blocks: {";
            for (int blockLabel : loopHeader->bodyBlocks) {
                cout << blockLabel << " ";
            }
            cout << "} ";
        }
        cout << endl;
        QuadFuncDecl* optimizedFunc = loopHoistFunc(func, loopHeaderMap);
        cout << "Optimized function " << optimizedFunc->funcname << endl;
        optimizedFuncs->push_back(optimizedFunc);
    }
    QuadProgram* x = new QuadProgram(optimizedFuncs, 0, 0);

    cout << "Writing optimized Quad to file: " << file_quad_ssa_opt << endl;
    ofstream ofs(file_quad_ssa_opt);
    if (!ofs) {
        cerr << "Error opening output file: " << file_quad_ssa_opt << endl;
        return EXIT_FAILURE;
    }
    string output_str;
    x->print(output_str, 0, false);
    ofs << output_str;
    ofs.close();
    cout << "-----Done---" << endl;
    return EXIT_SUCCESS;
}