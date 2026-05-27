#ifndef __LOOPSTRENGTHREDUCTION_HH__
#define __LOOPSTRENGTHREDUCTION_HH__

#include "loopinductionderived.hh"
#include <iostream>

// StrengthReductionPlan represents the transformations needed for one loop
class StrengthReductionPlan {
public:
	struct ReplacementIV {
		struct TempMap {
			int headerLabel = -1;      // Loop header label where this replacement is applied
			int oldTempNum = -1;       // Original derived IV temp being replaced
			int basicTempNum = -1;     // Basic IV temp that the derived IV is based on
			int newPhiTemp = -1;       // Temp number for the new PHI node introduced by strength reduction
			int newBackedgeTemp = -1;  // Temp number for the backedge value feeding the new PHI node
		};

		struct InitExpr {
			int initTempNum = -1;            // Temp holding the original initialization value used to build the affine init
			int basicCoeff = 1;              // Coefficient a in k = a*i + b
			int constant = 0;                // Constant offset b in k = a*i + b
			bool sourceAfterBasicUpdate = false; // True when the source init is computed after the basic IV update
			int basicStepTempNum = -1;       // Temp number of the basic IV step value when it is represented as a temp
			int basicStepValue = 0;          // Constant step value of the basic IV when it is not temp-based
		};

		struct InitTemps {
			int newInitTemp = -1;             // Final temp that holds the affine initial value for the new IV
			int newInitAdjustedSourceTemp = -1; // Optional temp that stores the source adjusted by the basic IV step first
			int newInitIntermediateTemp = -1; // Optional temp used for intermediate multiply-then-add initialization
		};

		struct StepExpr {
			int stepIncrementValue = 0;      // Constant step increment for the new backedge update
			int stepIncrementTempNum = -1;   // Temp-based step increment when the basic IV step is temp-based
			bool stepIncrementNegative = false; // Sign flag for temp-based step increments
			int stepTempScaleFactor = 1;     // Scaling factor applied to the step source temp
			int newStepTemp = -1;            // Temp holding the prepared step value if one must be materialized
			int stepSourceTempNum = -1;      // Primary source temp for step preparation
			int stepSecondaryTempNum = -1;   // Optional secondary source when step needs temp*temp
			int stepMulTemp = -1;            // Optional intermediate temp for (temp*temp)*const
		};

		struct Placement {
			int initLabel = -1;              // Label of the block where the new init statements should be inserted
			int backedgeLabel = -1;          // Label of the loop backedge block where the update should be inserted
		};

		size_t sourceOrder = 0;              // Source statement order used to keep replacements stable and to detect init timing

		TempMap map;                         // Temp rewrites for the replacement IV
		InitExpr initExpr;                   // Initialization expression needed to materialize the new IV value
		InitTemps initTemps;                 // Temps allocated for init expression construction
		StepExpr stepExpr;                   // Step expression needed for the loop backedge update
		Placement placement;                 // Block labels where init and backedge updates are inserted
	};

	// Mapping from original derived IV temp to replacement temp
	map<int, int> tempReplacement;
	
	// List of statements to remove from the original code
	set<quad::QuadStm*> stmtsToRemove;
	
	// Map of new PHI statements to add to loop header
	// PHI temp -> (init value temp, backedge value temp)
	map<int, pair<int, int>> phiStmtsToAdd;
	
	// Map of new update statements to add to loop body
	// temp -> (scale factor, offset)
	map<int, pair<int, int>> updateStmtsToAdd;

	// Concrete replacement records used for statement insertion in apply stage.
	vector<ReplacementIV> replacements;
	
	void print() const {
		if (replacements.empty()) {
			std::cout << "=== NO STRENGTH REDUCTIONS PLANNED ===" << std::endl;
			return;
		}

		
		std::cout << "=== STRENGTH REDUCTION PLAN ===" << std::endl;
		std::cout << "Total replacements: " << replacements.size() << std::endl;
		for (size_t i = 0; i < replacements.size(); ++i) {
			const ReplacementIV& repl = replacements[i];
			std::cout << "Replacement " << (i + 1) << ":" << std::endl;
			std::cout << "  Original IV: t" << repl.map.oldTempNum << std::endl;
			std::cout << "  New PHI temp: t" << repl.map.newPhiTemp << std::endl;
			std::cout << "  New backedge temp: t" << repl.map.newBackedgeTemp << std::endl;
			std::cout << "  Loop header: L" << repl.map.headerLabel << std::endl;
			std::cout << "  Source order: ";
			if (repl.sourceOrder != static_cast<size_t>(-1)) {
				std::cout << repl.sourceOrder << std::endl;
			} else {
				std::cout << "unknown" << std::endl;
			}
			std::cout << "  Init adjusted for update order: "
			          << (repl.initExpr.sourceAfterBasicUpdate ? "yes" : "no") << std::endl;
			std::cout << "  Init expr: ";

			bool printedInit = false;
			if (repl.initExpr.basicCoeff != 0) {
				if (repl.initExpr.basicCoeff < 0) {
					std::cout << "-";
				}
				int absCoeff = repl.initExpr.basicCoeff >= 0
					? repl.initExpr.basicCoeff
					: -repl.initExpr.basicCoeff;
				if (absCoeff != 1) {
					std::cout << absCoeff << "*";
				}
				std::cout << "t" << repl.map.basicTempNum;
				printedInit = true;
			}

			if (repl.initExpr.constant != 0 || !printedInit) {
				if (printedInit) {
					if (repl.initExpr.constant > 0) {
						std::cout << " + ";
					} else {
						std::cout << " - ";
					}
				} else if (repl.initExpr.constant < 0) {
					std::cout << "-";
				}
				std::cout << (repl.initExpr.constant >= 0 ? repl.initExpr.constant : -repl.initExpr.constant);
				printedInit = true;
			}
			
			if (!printedInit) {
				std::cout << "0";
			}
			std::cout << std::endl;

			std::cout << "  Step expr: ";
			if (repl.stepExpr.stepIncrementTempNum != -1) {
				std::cout << (repl.stepExpr.stepIncrementNegative ? "-" : "+")
				          << "t" << repl.stepExpr.stepIncrementTempNum;
			} else {
				std::cout << repl.stepExpr.stepIncrementValue;
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
};

// Generate strength reduction plan for a loop
// Returns a plan that can be applied to the function
StrengthReductionPlan generateStrengthReductionPlan(
	quad::QuadFuncDecl* func,
	const map<int, vector<DerivedInductionVar>>& derivedIVsByHeader,
	const map<int, vector<BasicInductionVar>>& basicIVsByHeader,
	LoopHeaderMap* loopHeaderMap
);

// Apply strength reduction transformations to the function
quad::QuadFuncDecl* applyStrengthReduction(
	quad::QuadFuncDecl* func,
	const StrengthReductionPlan& plan
);

#endif
