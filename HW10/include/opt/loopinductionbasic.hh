#ifndef __LOOPINDUCTIONBASIC_HH__
#define __LOOPINDUCTIONBASIC_HH__

#include "loopheader.hh"
#include "defusechain.hh"
#include <iostream>
#include <set>

class BasicInductionVar {
public:
	int headerLabel;
	int phiTempNum;
	int initTempNum;
	int backedgeTempNum;
	int step;
	int stepTempNum;  // If step is a loop-invariant temp, store its temp number; -1 if step is a literal constant
	int updateOrder;  // Statement order of the basic IV update within the function
	quad::QuadStm* phiStm;
	quad::QuadStm* updateStm;
	
	// Enhanced tracking of related temps
	std::set<int> relatedTemps;         // All temps involved in this IV
	std::set<int> uselessRelatedTemps;  // Related temps only used in backedges (cyclic/useless)
	std::set<int> usefulRelatedTemps;   // Related temps used in actual computations

	BasicInductionVar(
		int headerLabel,
		int phiTempNum,
		int initTempNum,
		int backedgeTempNum,
		int step,
		quad::QuadStm* phiStm,
		quad::QuadStm* updateStm
	) : headerLabel(headerLabel),
		phiTempNum(phiTempNum),
		initTempNum(initTempNum),
		backedgeTempNum(backedgeTempNum),
		step(step),
		stepTempNum(-1),  // Initialize to -1 (step is a literal constant, not a temp)
		updateOrder(-1),
		phiStm(phiStm),
		updateStm(updateStm) {
		// Initialize related temps with the core ones
		relatedTemps.insert(phiTempNum);
		relatedTemps.insert(initTempNum);
		relatedTemps.insert(backedgeTempNum);
		usefulRelatedTemps.insert(phiTempNum);  // PHI is always useful
	}
	
	// Check if a temp is part of this IV's family
	bool isRelatedTemp(int tempNum) const {
		return relatedTemps.count(tempNum) > 0;
	}
	
	
	// Add a related temp to this IV's family
	void addRelatedTemp(int tempNum) {
		relatedTemps.insert(tempNum);
	}
	
	// Mark a temp as useless (only in backedges, cyclic)
	void markUseless(int tempNum) {
		uselessRelatedTemps.insert(tempNum);
		usefulRelatedTemps.erase(tempNum);
	}
	
	// Mark a temp as useful (used in actual computations)
	void markUseful(int tempNum) {
		usefulRelatedTemps.insert(tempNum);
		uselessRelatedTemps.erase(tempNum);
	}
	
	void print() const {
		std::cout << "  t" << phiTempNum << " (PHI at L" << headerLabel << ")" << std::endl;
		std::cout << "    Initial value: t" << initTempNum << std::endl;
		std::cout << "    Backedge: t" << backedgeTempNum << std::endl;
		std::cout << "    Update order: ";
		if (updateOrder >= 0) {
			std::cout << updateOrder << std::endl;
		} else {
			std::cout << "unknown" << std::endl;
		}
		if (stepTempNum != -1) {
			int absStepTempNum = (stepTempNum < 0) ? -stepTempNum : stepTempNum;
			char op = (stepTempNum < 0) ? '-' : '+';
			std::cout << "    Step: " << op << "t" << absStepTempNum << " (loop-invariant)" << std::endl;
		} else {
			std::cout << "    Step: " << step << " (constant)" << std::endl;
		}
		std::cout << "    Related temps: ";
		for (int temp : relatedTemps) {
			std::cout << "t" << temp << " ";
		}
		std::cout << std::endl;
		if (!uselessRelatedTemps.empty()) {
			std::cout << "    Useless (cyclic only): ";
			for (int temp : uselessRelatedTemps) {
				std::cout << "t" << temp << " ";
			}
			std::cout << std::endl;
		}
	}
};

map<int, vector<BasicInductionVar>> discoverBasicInductionVars(quad::QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap, const DefUseChain& du, const ControlFlowInfo& cfi);

// Classify related temps as useless (cyclic only) or useful (in computations)
void classifyRelatedTemps(
    QuadFuncDecl* func,
    map<int, vector<BasicInductionVar>>& basicByHeader,
    const DefUseChain& du
);

#endif
