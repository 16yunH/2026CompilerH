#ifndef QUADFLOW_DEFUSECHAIN_HH
#define QUADFLOW_DEFUSECHAIN_HH

#include <map>
#include <set>
#include <vector>
#include "quad.hh"

using namespace std;
using namespace quad;

/**
 * DefUseChain: Represents the def-use chain for SSA Quad IR
 * 
 * In SSA form, each temporary variable has exactly one definition.
 * This class builds chains linking:
 *   - Each definition (def) to all its uses
 *   - Each use to its single definition
 * 
 * This structure provides the foundation for various compiler optimizations
 * and analyses that operate on SSA form.
 */

// Forward declarations
class DefUseNode;
class VarDefInfo;
class UseInfo;

/**
 * VarDefInfo: Information about a definition of a temporary variable
 * Stores the statement that defines the variable and all its uses
 */
class VarDefInfo {
  public:
    int tempNum;                           // Temporary variable number
    quad::QuadStm* defStm;                 // The statement that defines this temporary
    quad::QuadBlock* defBlock;             // The block containing the definition
    set<pair<quad::QuadBlock*, quad::QuadStm*>> useSet;  // All uses of this temporary

    VarDefInfo(int temp, quad::QuadStm* stm, quad::QuadBlock* block)
        : tempNum(temp), defStm(stm), defBlock(block) {}

    // Add a use location
    void addUse(quad::QuadBlock* block, quad::QuadStm* stm) {
        useSet.insert({block, stm});
    }
};

/**
 * UseInfo: Information about a use of a temporary variable
 * Stores the statement that uses the variable and its definition
 */
class UseInfo {
  public:
    int tempNum;                           // Temporary variable number
    quad::QuadStm* useStm;                 // The statement that uses this temporary
    quad::QuadBlock* useBlock;             // The block containing the use
    quad::QuadStm* defStm;                 // The statement that defines this temporary
    quad::QuadBlock* defBlock;             // The block containing the definition

    UseInfo(int temp, quad::QuadStm* stm, quad::QuadBlock* block,
            quad::QuadStm* def_stm, quad::QuadBlock* def_block)
        : tempNum(temp), useStm(stm), useBlock(block),
          defStm(def_stm), defBlock(def_block) {}

    // Print information for debugging
    void print(string &output_str) const;
};

/**
 * DefUseChain: Main class for managing def-use chains
 */
class DefUseChain {
  private:
    quad::QuadFuncDecl* func;              // The function we're analyzing
    
    // Maps: tempNum -> VarDefInfo
    map<int, VarDefInfo*> defMap;          // Definitions indexed by temporary number
    
    // Maps: QuadStm* -> set of VarDefInfo for temps defined by this statement
    map<quad::QuadStm*, set<int>> stmToDefs;

    // Maps: QuadStm* -> set of VarDefInfo for temps used by this statement
    map<quad::QuadStm*, set<int>> stmToUses;

    // Helper methods for building the def-use chain
    void extractDefsFromStm(quad::QuadStm* stm, quad::QuadBlock* block);
    void extractUsesFromStm(quad::QuadStm* stm, quad::QuadBlock* block);
    void linkDefUseChain();

  public:
    /**
     * Constructor: Build def-use chain for a function
     * @param func The QuadFuncDecl to analyze
     */
    DefUseChain(quad::QuadFuncDecl* func);

    /**
     * Destructor: Clean up allocated resources
     */
    ~DefUseChain();

    /**
     * Get definition information for a temporary
     * @param tempNum The temporary variable number
     * @return Pointer to VarDefInfo, or nullptr if not found
     */
    VarDefInfo* getDef(int tempNum) const;

    /**
     * Get all definitions for all temporaries
     * @return Reference to the def map
     */
    const map<int, VarDefInfo*>& getAllDefs() const { return defMap; }

    /**
     * Get definitions introduced by a specific statement
     * @param stm The statement to query
     * @return Set of temporary numbers defined by this statement
     */
    set<int> getDefsBy(quad::QuadStm* stm) const;

    /**
     * Get uses within a specific statement
     * @param stm The statement to query
     * @return Set of temporary numbers used by this statement
     */
    set<int> getUsesBy(quad::QuadStm* stm) const;
};

#endif  // QUADFLOW_DEFUSECHAIN_HH
