#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include "advDFG.hh"
#include "flowinfo.hh"
#include "instrSelection.hh"
#include "preSchedule.hh"
#include "schedule.hh"
#include "quad.hh"
#include "xml2flow.hh"

using namespace std;
using namespace quad;
using namespace instr;

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <basefile>" << endl;
        return EXIT_FAILURE;
    }

    string inputBaseName = argv[1];
    string fileQuadSsaWithflowXml = inputBaseName + ".4-ssa-withflow-xml.quad";

    cout << "Reading Quad (SSA with flow) from xml: " << fileQuadSsaWithflowXml << endl;

    set<FuncFlowInfo*> *flowProgSet = xml2flow(fileQuadSsaWithflowXml.c_str());
    if (flowProgSet == nullptr || flowProgSet->empty()) {
        cerr << "Error reading Quad with flow information from xml" << endl;
        return EXIT_FAILURE;
    }

    auto *funcList = new vector<QuadFuncDecl*>();
    int programLastLabelNum = -1;
    int programLastTempNum = -1;

    for (FuncFlowInfo *ffi : *flowProgSet) {
        if (ffi == nullptr || ffi->cfi == nullptr || ffi->cfi->func == nullptr) {
            continue;
        }

        funcList->push_back(ffi->cfi->func);
        if (ffi->programLastLabelNum >= 0) {
            programLastLabelNum = ffi->programLastLabelNum;
        }
        if (ffi->programLastTempNum >= 0) {
            programLastTempNum = ffi->programLastTempNum;
        }
    }

    if (funcList->empty()) {
        cerr << "No functions found in flow XML." << endl;
        return EXIT_FAILURE;
    }

    QuadProgram *program = new QuadProgram(funcList, programLastLabelNum, programLastTempNum);

    advDFGprog *graphProgram = buildAdvDFGprog(program);
    if (graphProgram == nullptr) {
        cerr << "Failed to build advDFG program." << endl;
        return EXIT_FAILURE;
    }

    graphProgram->printAllGraphsToStdout();

    preScheduleProg *preScheduleProgram = buildPreScheduleProg(program);
    if (preScheduleProgram == nullptr) {
        cerr << "Failed to build preSchedule program." << endl;
        return EXIT_FAILURE;
    }

    runInstructionSelectionPass(*graphProgram, *preScheduleProgram);

    preScheduleProgram->printToStdout();

    ScheduleProg *scheduleProgram = scheduleProg(preScheduleProgram);
    if (scheduleProgram == nullptr) {
        cerr << "Failed to build schedule program." << endl;
        return EXIT_FAILURE;
    }

    scheduleProgram->writeToFile(inputBaseName);
    return EXIT_SUCCESS;
}