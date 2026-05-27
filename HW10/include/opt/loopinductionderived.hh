#ifndef __LOOPINDUCTIONDERIVED_HH__
#define __LOOPINDUCTIONDERIVED_HH__

#include "loopinductionbasic.hh"
#include <iostream>

struct AffineIVExpr {
	// The basic induction variable temp number (e.g., 10000 for loop counter)
	int basicTempNum;
	
	// The coefficient multiplying the basic IV (a in k = a*i + b)
	int basicCoeff;
	
	// The constant offset (b in k = a*i + b)
	int constant;
};

class DerivedInductionVar {
public:
	// Label of the loop header this DIV belongs to
	int headerLabel;
	
	// The temporary number of this derived IV (e.g., 10002)
	int tempNum;
	
	// The temporary number of the immediate source of this DIV
	// (the value used in the expression that computes this DIV)
	int sourceTempNum;
	
	// Order of the source statement in its block; used to determine if DIV
	// is computed after the basic IV's step increment (sourceAfterBasicUpdate)
	size_t sourceOrder;
	
	// The affine expression: basicCoeff * basicIV + constant
	// where basicIV is the basic induction variable this DIV is derived from
	AffineIVExpr expr;
	
	// The statement that defines this DIV (t_k = basicCoeff * t_i + constant)
	quad::QuadStm* defStm;

	DerivedInductionVar(
		int headerLabel,
		int tempNum,
		int sourceTempNum,
		size_t sourceOrder,
		const AffineIVExpr& expr,
		quad::QuadStm* defStm
	) : headerLabel(headerLabel),
		tempNum(tempNum),
		sourceTempNum(sourceTempNum),
		sourceOrder(sourceOrder),
		expr(expr),
		defStm(defStm) {}
	
	void print() const {
		std::cout << "  t" << tempNum << " (derived IV)" << std::endl;
		std::cout << "    Depends on: t" << sourceTempNum << std::endl;
		std::cout << "    Source order: ";
		if (sourceOrder != static_cast<size_t>(-1)) {
			std::cout << sourceOrder << std::endl;
		} else {
			std::cout << "unknown" << std::endl;
		}
		std::cout << "    Expression: ";

		bool printedExpr = false;

		if (expr.basicCoeff != 0) {
			if (expr.basicCoeff < 0) {
				std::cout << "-";
			}
			int absCoeff = expr.basicCoeff >= 0
				? expr.basicCoeff
				: -expr.basicCoeff;
			if (absCoeff != 1) {
				std::cout << absCoeff << "*";
			}
			std::cout << "t" << expr.basicTempNum;
			printedExpr = true;
		}

		if (expr.constant != 0 || !printedExpr) {
			if (printedExpr) {
				if (expr.constant > 0) {
					std::cout << " + ";
				} else {
					std::cout << " - ";
				}
			} else if (expr.constant < 0) {
				std::cout << "-";
			}
			std::cout << (expr.constant >= 0 ? expr.constant : -expr.constant);
			printedExpr = true;
		}
		if (!printedExpr) {
			std::cout << "0";
		}
		std::cout << std::endl;
		std::cout << "    Definition: ";
		if (defStm != nullptr) {
			std::string defStr;
			defStm->print(defStr, 0, false);
			std::cout << defStr;
		}
		std::cout << std::endl;
	}
};

map<int, vector<DerivedInductionVar>> discoverDerivedInductionVars(quad::QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap, const DefUseChain& du, const ControlFlowInfo& cfi);

#endif
