#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>
#include <filesystem>

#include "xml2asmprog.hh"
#include "asmprogpass.hh"
#include "ig.hh"
#include "coloring.hh"

using namespace std;
using namespace instr;
namespace fs = std::filesystem;

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <basefile>" << endl;
        return EXIT_FAILURE;
    }

    string baseName = argv[1];
    string xmlInputFile = baseName + ".5-xml.asm";
    string asmOutputFile = baseName + ".s";
    
    // Extract filename from baseName for colored output path
    size_t lastSlash = baseName.find_last_of("/\\");
    string fileName = (lastSlash != string::npos) ? baseName.substr(lastSlash + 1) : baseName;

    cout << "Reading assembly program from: " << xmlInputFile << endl;

    AsmProg* program = xml2asmprog(xmlInputFile.c_str());
    if (program == nullptr) {
        cerr << "Error reading assembly program from " << xmlInputFile << endl;
        return EXIT_FAILURE;
    }

    cout << "Writing assembly to: " << asmOutputFile << endl;

    ofstream outFile(asmOutputFile);
    if (!outFile.is_open()) {
        cerr << "Error opening output file: " << asmOutputFile << endl;
        delete program;
        return EXIT_FAILURE;
    }

    outFile << program->to_string();
    outFile.close();

    // Make a pass to prepare the program for dataflow analysis
    preDataFlowPass(program);

    // Build interference graphs for all functions
    cout << "\nBuilding interference graphs..." << endl;
    vector<InterferenceGraph*> graphs = buildIgProg(program);

    // Print interference graphs for each function
    for (size_t i = 0; i < graphs.size() && i < program->functions.size(); ++i) {
        cout << "\nInterference Graph for function: " << program->functions[i].name << endl;
        cout << graphs[i]->printGraph();
    }

    // Loop through different k values
    vector<int> kValues = {2, 5, 9};
    for (int k : kValues) {
        cout << "\n========== REGISTER ALLOCATION WITH k=" << k << " ==========" << endl;
        
        // Create output directory in current directory
        string outputDir = "k" + to_string(k);
        try {
            fs::create_directories(outputDir);
        } catch (const exception& e) {
            cerr << "Error creating directory " << outputDir << ": " << e.what() << endl;
            return EXIT_FAILURE;
        }

        // Perform register allocation (coloring) for each function
        vector<Coloring*> colorings;
        for (size_t i = 0; i < graphs.size(); ++i) {
            cout << "\nColoring function " << program->functions[i].name << " with k=" << k << " colors..." << endl;
            Coloring* c = coloring(graphs[i], k);
            colorings.push_back(c);
            cout << "Coloring result:" << endl;
            cout << c->printColoring() << endl;
        }

        // Apply colors to assembly and generate colored output
        cout << "\n========== APPLYING COLORS FOR k=" << k << " ==========" << endl;
        AsmProg* coloredProgram = asmprog2colored(program, colorings);
        
        string coloredOutputFile = outputDir + "/" + fileName + ".colored.s";
        cout << "Writing colored assembly to: " << coloredOutputFile << endl;
        ofstream coloredFile(coloredOutputFile);
        if (!coloredFile.is_open()) {
            cerr << "Error opening colored output file: " << coloredOutputFile << endl;
            delete program;
            delete coloredProgram;
            return EXIT_FAILURE;
        }
        coloredFile << coloredProgram->to_string();
        coloredFile.close();

        // Clean up colorings for this k
        for (auto c : colorings) {
            delete c;
        }
        delete coloredProgram;
    }

    delete program;

    cout << "\nDone." << endl;
    return EXIT_SUCCESS;
}