#ifndef _QUAD
#define _QUAD

#include <vector>
#include <string>
#include <variant>
#include "tinyxml2.hh"
#include "temp.hh"

using namespace std;
using namespace tree; // for Temp and Label

namespace quad {

class Quad;
class QuadProgram;
class QuadFuncDecl;
class QuadBlock;
class QuadStm;
class QuadPtrCalc;
class QuadMove;
class QuadLoad;
class QuadStore;
class QuadMoveBinop;
class QuadCall;
class QuadExtCall;
class QuadMoveCall;
class QuadMoveExtCall;
class QuadLabel;
class QuadJump;
class QuadCJump;
class QuadPhi;
class QuadReturn;

class QuadVisitor {
  public:
    virtual void visit(QuadProgram *quad) = 0;
    virtual void visit(QuadFuncDecl *quad) = 0;
    virtual void visit(QuadBlock *quad) = 0;
    virtual void visit(QuadMove *quad) = 0;
    virtual void visit(QuadLoad *quad) = 0;
    virtual void visit(QuadStore *quad) = 0;
    virtual void visit(QuadMoveBinop *quad) = 0;
    virtual void visit(QuadCall *quad) = 0;
    virtual void visit(QuadMoveCall *quad) = 0;
    virtual void visit(QuadExtCall *quad) = 0;
    virtual void visit(QuadMoveExtCall *quad) = 0;
    virtual void visit(QuadLabel *quad) = 0;
    virtual void visit(QuadJump *quad) = 0;
    virtual void visit(QuadCJump *quad) = 0;
    virtual void visit(QuadPhi *quad) = 0;
    virtual void visit(QuadReturn *quad) = 0;
    virtual void visit(QuadPtrCalc *quad) = 0;
};

/*
Quad is a class that represents a quadruple in the intermediate representation.
It contains the following members:
- kind: the kind of the quadruple (e.g., MOVE, LOAD, STORE, etc.) 
- def: a set of temporary variables that are defined by this quadruple
- use: a set of temporary variables that are used by this quadruple

//Note: term is either a TempExp, Const or Name (string, from NAME)
Move:  temp <- term
Load:  temp <- mem(term)
Store: mem(term) <- term
MoveBinop: temp <- term op term
Call:  ExpStm(call) //ignore the result
ExtCall: ExpStm(extcall) //ignore the result
MoveCall: temp <- call
MoveExtCall: temp <- extcall
Label: label
Jump: jump label
CJump: cjump relop term, term, label, label
Phi:  temp <- list of {temp, label} //same as the Phi in the tree
Return: return term
PTR_CALC: temp <- term + term (for pointer arithmetic, the second term is the offset)
*/

enum class QuadKind { 
    PROGRAM, FUNCDECL, BLOCK,
    MOVE, LOAD, STORE, MOVE_BINOP, 
    CALL, MOVE_CALL, EXTCALL, MOVE_EXTCALL, 
    LABEL, JUMP, CJUMP,
    PHI, RETURN, PTR_CALC 
};

string quadKindToString(QuadKind kind);

enum class QuadTermKind { 
    TEMP, CONST, NAME
};

enum class QuadType {INT, PTR};

string quadTypeToString(QuadType type);

QuadType stringToQuadType(const string& typeStr);

class QuadTemp {
  public:
    Temp *temp;
    QuadType type;
    QuadTemp(Temp *temp, QuadType type) : temp(temp), type(type) {}
    // Clone function for QuadTemp
    QuadTemp* clone() const;
};

//Terminals of the Quad: temp, const, name(whichi is from a string label)
class QuadTerm {
  public:
    QuadTermKind kind;
    variant<monostate, QuadTemp*, int, string> term;
    QuadTerm(QuadTemp *quadtemp) : term(quadtemp), kind(QuadTermKind::TEMP) {}
    QuadTerm(int constant) : term(constant), kind(QuadTermKind::CONST) {}
    QuadTerm(string name) : term(name), kind(QuadTermKind::NAME) {}

    string print();
    QuadTemp *get_temp();
    int get_const();
    string get_name();
    
    // Clone function for QuadTerm
    QuadTerm* clone() const;
};

class Quad {
  public:
    QuadKind kind;
    virtual void print(string &output_str, int indent, bool print_def_use) = 0;
    virtual void accept(QuadVisitor &v) = 0;
    virtual Quad* clone() const = 0; // Abstract clone method
    Quad(QuadKind k) : kind(k) {}
};

class QuadProgram : public Quad {
  public:
    vector<QuadFuncDecl*> *quadFuncDeclList;
    int last_label_num;
    int last_temp_num;
    QuadProgram(vector<QuadFuncDecl*> *quadFuncDeclList, int lln, int ltn) 
        : Quad(QuadKind::PROGRAM), quadFuncDeclList(quadFuncDeclList), last_label_num(lln), last_temp_num(ltn) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadProgram* clone() const override;
};

class QuadFuncDecl : public Quad {
  public:
    vector<QuadBlock*> *quadblocklist;
    string funcname;
    vector<Temp*> *params;
    int last_label_num;
    int last_temp_num;
    QuadFuncDecl(string funcname, vector<Temp*> *params, vector<QuadBlock*> *quadblocklist, int lln, int ltn)
        : Quad(QuadKind::FUNCDECL), params(params), quadblocklist(quadblocklist), funcname(funcname), last_label_num(lln), last_temp_num(ltn) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadFuncDecl* clone() const override;
};

class QuadBlock : public Quad {
  public:
    Label *entry_label;
    vector<Label*> *exit_labels;
    vector<QuadStm*> *quadlist;
    QuadBlock(vector<QuadStm*> *quadlist, Label *entry_label, vector<Label*> *exit_labels)
        : Quad(QuadKind::BLOCK), entry_label(entry_label), exit_labels(exit_labels), quadlist(quadlist) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadBlock* clone() const override;
};

class QuadStm : public Quad {
  public:
    QuadKind kind;
    set<Temp*> *def;
    set<Temp*> *use;
    QuadStm(QuadKind k, set<Temp*> *def, set<Temp*> *use) 
        : Quad(k), kind(k), def(def), use(use) {}
    virtual void accept(QuadVisitor &v) = 0;
    virtual void print(string &output_str, int indent, bool print_def_use) = 0;
    
    // Helper method for cloning def/use sets
    set<QuadTemp*>* cloneTemps(const set<QuadTemp*>* temps) const;
};

class QuadPtrCalc : public QuadStm {
  public:
    QuadTerm *dst; // should be a temp
    QuadTerm *ptr; // should be a temp (base pointer) 
    QuadTerm *offset; // should be int offset

    QuadPtrCalc(QuadTerm *dst, QuadTerm *ptr, QuadTerm *offset,  set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::PTR_CALC, def, use), dst(dst), ptr(ptr), offset(offset) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    QuadPtrCalc* clone() const override;
};

class QuadMove : public QuadStm{
  public:
    QuadTemp *dst;
    QuadTerm *src;
    QuadMove(QuadTemp *dst, QuadTerm *src, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::MOVE, def, use), dst(dst), src(src) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadMove* clone() const override;
};

class QuadLoad : public QuadStm{
  public:
    QuadTemp *dst;
    QuadTerm *src;
    QuadLoad(QuadTemp *dst, QuadTerm *src, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::LOAD, def, use), dst(dst), src(src) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadLoad* clone() const override;
};

class QuadStore : public QuadStm{
  public:
    QuadTerm *src;
    QuadTerm *dst; //in the form: mem(dst)
    QuadStore(QuadTerm *src, QuadTerm *dst, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::STORE, def, use), src(src), dst(dst) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadStore* clone() const override;
};

class QuadMoveBinop : public QuadStm{
  public:
    QuadTemp *dst;
    QuadTerm *left;
    QuadTerm *right;
    string binop;
    QuadMoveBinop(QuadTemp *dst, QuadTerm *left, string binop, QuadTerm *right, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::MOVE_BINOP, def, use), dst(dst), left(left), binop(binop), right(right) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadMoveBinop* clone() const override;
};

//Call is always a load a call result to a temp
class QuadCall : public QuadStm{
  public:
    string name;
    QuadTerm *obj_term;
    vector<QuadTerm*> *args;
    QuadCall(string name, QuadTerm *obj_term, vector<QuadTerm*> *args, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::CALL, def, use), name(name), obj_term(obj_term), args(args) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadCall* clone() const override;
};

class QuadMoveCall : public QuadStm{
  public:
    QuadTemp *dst;
    QuadCall *call;
    QuadMoveCall(QuadTemp *dst, QuadCall *call, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::MOVE_CALL, def, use), dst(dst), call(call) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadMoveCall* clone() const override;
};

class QuadExtCall : public QuadStm{
  public:
    string extfun;
    vector<QuadTerm*> *args;
    QuadExtCall(string extfun, vector<QuadTerm*> *args, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::EXTCALL, def, use), extfun(extfun), args(args) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadExtCall* clone() const override;
};

class QuadMoveExtCall : public QuadStm{
  public:
    QuadTemp *dst;
    QuadExtCall *extcall;
    QuadMoveExtCall(QuadTemp *dst, QuadExtCall *extcall, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::MOVE_EXTCALL, def, use), dst(dst), extcall(extcall) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadMoveExtCall* clone() const override;
};

class QuadLabel : public QuadStm{
  public:
    Label *label;
    QuadLabel(Label *label, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::LABEL, def, use), label(label) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadLabel* clone() const override;
};

class QuadJump : public QuadStm{
  public:
    Label *label;
    QuadJump(Label *label, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::JUMP, def, use), label(label) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadJump* clone() const override;
};

class QuadCJump : public QuadStm{
  public:
    string relop;
    QuadTerm *left;
    QuadTerm *right; 
    Label *t, *f;
    QuadCJump(string relop, QuadTerm *left, QuadTerm *right, Label *t, Label *f, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::CJUMP, def, use), relop(relop), left(left), right(right), t(t), f(f) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadCJump* clone() const override;
};

class QuadPhi : public QuadStm {
  public:
    QuadTemp *temp_exp;
    vector<pair<Temp*, Label*>> *args;
    QuadPhi(QuadTemp *temp_exp, vector<pair<Temp*, Label*>> *args, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::PHI, def, use), temp_exp(temp_exp), args(args) {}
    void accept(QuadVisitor &v) override {v.visit(this);};
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadPhi* clone() const override;
};

class QuadReturn : public QuadStm{
  public:
    QuadTerm *exp;
    QuadReturn(QuadTerm *exp, set<Temp*> *def, set<Temp*> *use)
        : QuadStm(QuadKind::RETURN, def, use), exp(exp) {}
    void accept(QuadVisitor &v) override {v.visit(this);}
    void print(string &output_str, int indent, bool print_def_use) override;
    
    // Clone function
    QuadReturn* clone() const override;
};

} //namespace quad

#endif
