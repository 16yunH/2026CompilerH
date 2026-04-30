#define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <set>
#include "quad.hh"
#include "temp.hh"

using namespace std;
using namespace quad;

string quad::quadKindToString(QuadKind kind) {
    switch (kind) {
        case QuadKind::PROGRAM: return "PROGRAM";
        case QuadKind::FUNCDECL: return "FUNCDECL";
        case QuadKind::BLOCK: return "BLOCK";
        case QuadKind::MOVE: return "MOVE";
        case QuadKind::LOAD: return "LOAD";
        case QuadKind::STORE: return "STORE";
        case QuadKind::MOVE_BINOP: return "MOVE_BINOP";
        case QuadKind::CALL: return "CALL";
        case QuadKind::MOVE_CALL: return "MOVE_CALL";
        case QuadKind::EXTCALL: return "EXTCALL";
        case QuadKind::MOVE_EXTCALL: return "MOVE_EXTCALL";
        case QuadKind::LABEL: return "LABEL";
        case QuadKind::JUMP: return "JUMP";
        case QuadKind::CJUMP: return "CJUMP";
        case QuadKind::PHI: return "PHI";
        case QuadKind::RETURN: return "RETURN";
        case QuadKind::PTR_CALC: return "PTR_CALC";
    }
    return "UNKNOWN";
}

string quad::quadTypeToString(QuadType type) {
    switch (type) {
        case QuadType::INT: return "int";
        case QuadType::PTR: return "ptr";
    }
    return "unknown";
}

QuadType quad::stringToQuadType(const string& typeStr) {
    if (typeStr == "ptr") return QuadType::PTR;
    return QuadType::INT;
}

QuadTemp* QuadTemp::clone() const {
    return new QuadTemp(new Temp(temp->num), type);
}

QuadTerm* QuadTerm::clone() const {
    switch (kind) {
        case QuadTermKind::TEMP:
            return new QuadTerm(get<QuadTemp*>(term)->clone());
        case QuadTermKind::CONST:
            return new QuadTerm(get<int>(term));
        case QuadTermKind::NAME:
            return new QuadTerm(get<std::string>(term));
    }
    return nullptr;
}

std::string QuadTerm::print() {
#ifdef DEBUG
    cout << "In QuadTerm::print" << endl;
#endif
    QuadTemp *t_quadtemp;
    switch (kind) {
        case QuadTermKind::TEMP:
            t_quadtemp = get<QuadTemp*>(term);
            return "t" + to_string(t_quadtemp->temp->num) + ":" + quadTypeToString(t_quadtemp->type); 
        case QuadTermKind::CONST:
            return "Const:"+to_string(this->get_const());
        case QuadTermKind::NAME:
            return "Name:"+get<std::string>(term);
    }
    return "";
}
QuadTemp* QuadTerm::get_temp() {
    if (kind == QuadTermKind::TEMP) {
        return get<QuadTemp*>(term);
    }
    return nullptr;
}

int QuadTerm::get_const() {
    if (kind == QuadTermKind::CONST) {
        return get<int>(term);
    }
    return 0;
}

string QuadTerm::get_name() {
    if (kind == QuadTermKind::NAME) {
        return get<std::string>(term);
    }
    return "";
}

static string print_indent(int indent) {
#ifdef DEBUG
    cout << "In print_indent" << endl;
#endif
    std::string output_str; output_str.reserve(20);
    for (int i = 0; i < indent; i++) {
        output_str += " ";
    }
    return output_str;
}

string print_def_use(set<Temp*> *def, set<Temp*> *use) {
#ifdef DEBUG
    cout << "In print_def_use" << endl;
#endif
    std::string output_str; output_str.reserve(200);
    output_str += "def: ";
    if (def != nullptr) {
        for (auto t : *def) {
            output_str += "t" + to_string(t->num);
            output_str += " ";
        }
    }
    output_str += "use: ";
    if (use != nullptr) {
        for (auto t : *use) {
            output_str += "t" + to_string(t->num);
            output_str += " ";
        }
    }
    return output_str;
}

static string print_temp(QuadTemp *t) {
#ifdef DEBUG
    cout << "In print_temp" << endl;
#endif
    std::string output_str; output_str.reserve(20);
    if (t != nullptr) {
        output_str += "t";
        output_str += to_string(t->temp->num);
        output_str += ":";
        output_str += quadTypeToString(t->type);
    }
    return output_str;
}

static std::string print_label(Label *l) {
#ifdef DEBUG
    cout << "In print_label" << endl;
#endif
    std::string str; str.reserve(20);
    if (l != nullptr) {
        str = "L" + to_string(l->num);
    }
    return str;
}

static std::string print_call(QuadCall *call) {
#ifdef DEBUG
    cout << "In print_call" << endl;
#endif
    std::string use_str; use_str.reserve(100);
    std::string id = call->name;
    QuadTerm *obj_term = call->obj_term;
    vector<QuadTerm*> *args = call->args;
    use_str += call->name;
    use_str += "[";
    use_str +=  obj_term->print();
    use_str += "] (";
    bool first = true;
    for (auto arg : *args) {
        use_str += (first?"":", ");
        use_str += arg->print();
        first = false;
    }
    use_str += "); " ;
    return use_str;
}

string print_extcall(QuadExtCall *call) {
#ifdef DEBUG
    cout << "In print_extcall" << endl;
#endif
    string use_str; use_str.reserve(100);
    vector<QuadTerm*> *args = call->args;
    use_str += call->extfun;
    use_str += "(";
    bool first = true;
    for (auto arg : *args) {
        use_str += (first?"":", ");
        use_str += arg->print();
        first = false;
    }
    use_str += "); " ;
    return use_str;
}

void QuadProgram::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadProgram::print: " <<  (this->kind == QuadKind::PROGRAM?"Program!":"Oh?") << endl;
#endif
    if (quadFuncDeclList == nullptr) {
        return ;
    }

    for (auto func : *quadFuncDeclList) {
        if (func == nullptr) {
            cerr << "Error: QuadFuncDecl is null!" << endl;
            continue;
        }
        func->print(use_str, indent, to_print_def_use);
    }
    return ;
}

void QuadFuncDecl::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadFuncDecl::print" << endl;
#endif
    if (this->quadblocklist == nullptr) {
        return;
    }
    use_str += "Function "; use_str += this->funcname;
    if (params != nullptr) {
        use_str += "(";
        bool first = true;
        for (auto param : *this->params) {
            if (!first) {
                use_str += ", ";
            }
            use_str += "t"+to_string(param->num);
            first = false;
        }
        use_str += ")";
    }
    use_str += " last_label=";
    use_str += to_string(this->last_label_num);
    use_str += " last_temp=";
    use_str += to_string(this->last_temp_num);
    use_str += ":\n";
    for (auto block : *this->quadblocklist) {
        block->print(use_str, indent+2, to_print_def_use);
    }
    return ;
}

void QuadBlock::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadBlock::print" << endl;
#endif
    use_str += print_indent(indent);
    use_str += "Block: Entry Label: ";
    use_str += print_label(this->entry_label);
    use_str +=+ "\n";
    if (this->exit_labels != nullptr) {
        use_str += print_indent(indent+2);
        use_str += "Exit labels: ";
        for (auto label : *this->exit_labels) {
            use_str += print_label(label);
            use_str += " ";
        }
        use_str += "\n";
    }
    for (auto quad : *this->quadlist) {
        quad->print(use_str, indent+2, to_print_def_use);
    }
    return ;
}

void QuadMove::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadMove::print" << endl;
#endif
    QuadTemp *dst_temp = this->dst;
    QuadTerm *src_term = this->src;
    use_str += print_indent(indent);
    use_str += "MOVE ";
    use_str += print_temp(dst_temp);
    use_str += " <- ";
    use_str += src_term->print();
    use_str += "; ";
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return ;
}

void QuadLoad::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadLoad::print" << endl;
#endif
    QuadTemp *dst_temp = this->dst;
    QuadTerm *src_term = this->src;
    use_str += print_indent(indent);
    use_str += "LOAD ";
    use_str +=  print_temp(dst_temp);
    use_str += " <- Mem(";
    use_str += src_term->print() + "); ";
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return;
}

//store is term->mem(term)
void QuadStore::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadStore::print" << endl;
#endif
    QuadTerm *src_term = this->src;
    QuadTerm *dst_term = this->dst;
    use_str += print_indent(indent);
    use_str += "STORE ";
    use_str += src_term->print();
    use_str += " -> ";
    use_str += "Mem(";
    use_str += dst_term->print();
    use_str += "); ";
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return;
}
        
void QuadMoveBinop::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadMoveBinop::print" << endl;
#endif
    QuadTemp *dst_temp = this->dst;
    string binop = this->binop;
    QuadTerm *left_term = this->left;
    QuadTerm *right_term = this->right;
    use_str += print_indent(indent);
    use_str += "MOVE_BINOP ";
    use_str += print_temp(dst_temp);
    use_str += " <- ";
    use_str += "(";
    use_str += binop;
    use_str += ", ";
    use_str += left_term->print();
    use_str += ", ";
    use_str += right_term->print();
    use_str += "); ";
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return ;
}

void QuadCall::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadCall::print" << endl;
#endif
        string call_string = print_call(this);
        use_str += print_indent(indent);
        use_str += "CALL ";
        use_str += call_string;
        use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
        use_str += "\n";
        return;
}

void QuadMoveCall::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadMoveCall::print" << endl;
#endif
    QuadTemp *dst_temp = this->dst;
    QuadCall *call = this->call;
    use_str += print_indent(indent);
    use_str += "MOVE_CALL ";
    use_str += print_temp(dst_temp);
    use_str += " <- ";
    use_str += print_call(call);
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return;
}

void QuadExtCall::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadExtCall::print" << endl;
#endif
    string extcall_str = print_extcall(this);
    use_str += print_indent(indent);
    use_str += "EXTCALL ";
    use_str += extcall_str;
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return;
}
        
void QuadMoveExtCall::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadMoveExtCall::print" << endl;
#endif
    QuadTemp *dst_temp = this->dst;
    QuadExtCall *extcall = this->extcall;
    use_str += print_indent(indent);
    use_str += "MOVE_EXTCALL ";
    use_str += print_temp(dst_temp);
    use_str += " <- ";
    use_str += print_extcall(extcall);
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return;
}

void QuadLabel::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadLabel::print" << endl;
#endif
    use_str += print_indent(indent);
    use_str += "LABEL ";
    use_str += print_label(this->label);
    use_str += "; ";
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return;
}

void QuadJump::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadJump::print" << endl;
#endif
    use_str += print_indent(indent);
    use_str += "JUMP ";
    use_str += print_label(this->label);
    use_str += "; ";
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return;
}

void QuadCJump::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadCJump::print" << endl;
#endif
    Label *true_label = this->t;
    Label *false_label = this->f;
    QuadTerm *left_term = this->left;
    QuadTerm *right_term = this->right;
    use_str += print_indent(indent);
    use_str += "CJUMP ";
    use_str += this->relop;
    use_str += " ";
    use_str += left_term->print();
    use_str += " ";
    use_str += right_term->print();
    use_str += "? ";
    use_str += print_label(true_label);
    use_str += " : ";
    use_str += print_label(false_label);
    use_str += "; ";
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return;
}

void QuadPhi::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadPhi::print" << endl;
#endif
    QuadTemp *phi_temp = this->temp_exp;
    string phi_str = print_indent(indent) + "PHI " + print_temp(phi_temp) + " <- Phi(";
    bool first = true;
    for (int i=0; i< this->args->size(); i++) {
        Temp *arg_temp = this->args->at(i).first;
        Label *arg_label = this->args->at(i).second;
        if (!first) {
            phi_str += ", ";
        }
        phi_str += "[t";
        phi_str += to_string(arg_temp->num);
        phi_str += ", ";
        phi_str += print_label(arg_label);
        phi_str += "]";
        first = false;
    }
    phi_str += "); ";
    phi_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    phi_str += "\n";
    use_str += phi_str;
    return ;
}

void QuadReturn::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadReturn::print" << endl;  
#endif
    QuadTerm *ret_term = this->exp;
    use_str += print_indent(indent);
    use_str += "RETURN ";
    use_str += ret_term->print();
    use_str += "; ";
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return ;
}

void QuadPtrCalc::print(string &use_str, int indent, bool to_print_def_use) {
#ifdef DEBUG
    cout << "In QuadPtrCalc::print" << endl;  
#endif
    use_str += print_indent(indent);
    use_str += "PTR_CALC ";
    if (this->dst != nullptr) {
        use_str += this->dst->print();
    }
    use_str += " <- ";
    if (this->ptr != nullptr) {
        use_str += this->ptr->print(); // base pointer
        if (this->offset != nullptr) {
            use_str += " + ";
            use_str += this->offset->print(); // offset
        }
    }
    use_str += "; ";
    use_str += (to_print_def_use? print_def_use(this->def, this->use) : "");
    use_str += "\n";
    return ;
}

QuadPtrCalc* QuadPtrCalc::clone() const {
    QuadTerm* cloned_dst = dst != nullptr ? dst->clone() : nullptr;
    QuadTerm* cloned_ptr = ptr != nullptr ? ptr->clone() : nullptr;
    QuadTerm* cloned_offset = offset != nullptr ? offset->clone() : nullptr;
    
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadPtrCalc(cloned_dst, cloned_ptr, cloned_offset, cloned_def, cloned_use);
}

QuadProgram* QuadProgram::clone() const {
    vector<QuadFuncDecl*>* cloned_funcs = nullptr;
    if (quadFuncDeclList != nullptr) {
        cloned_funcs = new vector<QuadFuncDecl*>();
        for (QuadFuncDecl* func : *quadFuncDeclList) {
            cloned_funcs->push_back(func != nullptr ? (QuadFuncDecl*)func->clone() : nullptr);
        }
    }
    return new QuadProgram(cloned_funcs, last_label_num, last_temp_num);
}

QuadFuncDecl* QuadFuncDecl::clone() const {
    vector<Temp*>* cloned_params = nullptr;
    if (params != nullptr) {
        cloned_params = new vector<Temp*>();
        for (Temp* p : *params) {
            cloned_params->push_back(p != nullptr ? new Temp(p->num) : nullptr);
        }
    }
    vector<QuadBlock*>* cloned_blocks = nullptr;
    if (quadblocklist != nullptr) {
        cloned_blocks = new vector<QuadBlock*>();
        for (QuadBlock* block : *quadblocklist) {
            cloned_blocks->push_back(block != nullptr ? (QuadBlock*)block->clone() : nullptr);
        }
    }
    return new QuadFuncDecl(funcname, cloned_params, cloned_blocks, last_label_num, last_temp_num);
}

QuadBlock* QuadBlock::clone() const {
    vector<QuadStm*>* cloned_quads = nullptr;
    if (quadlist != nullptr) {
        cloned_quads = new vector<QuadStm*>();
        for (QuadStm* stm : *quadlist) {
            cloned_quads->push_back(stm != nullptr ? (QuadStm*)stm->clone() : nullptr);
        }
    }
    Label* cloned_entry = entry_label;
    vector<Label*>* cloned_exits = nullptr;
    if (exit_labels != nullptr) {
        cloned_exits = new vector<Label*>();
        for (Label* l : *exit_labels) {
            cloned_exits->push_back(l);
        }
    }
    return new QuadBlock(cloned_quads, cloned_entry, cloned_exits);
}

QuadMove* QuadMove::clone() const {
    QuadTemp* cloned_dst = dst != nullptr ? dst->clone() : nullptr;
    QuadTerm* cloned_src = src != nullptr ? src->clone() : nullptr;
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadMove(cloned_dst, cloned_src, cloned_def, cloned_use);
}

QuadLoad* QuadLoad::clone() const {
    QuadTemp* cloned_dst = dst != nullptr ? dst->clone() : nullptr;
    QuadTerm* cloned_src = src != nullptr ? src->clone() : nullptr;
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadLoad(cloned_dst, cloned_src, cloned_def, cloned_use);
}

QuadStore* QuadStore::clone() const {
    QuadTerm* cloned_src = src != nullptr ? src->clone() : nullptr;
    QuadTerm* cloned_dst = dst != nullptr ? dst->clone() : nullptr;
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadStore(cloned_src, cloned_dst, cloned_def, cloned_use);
}

QuadMoveBinop* QuadMoveBinop::clone() const {
    QuadTemp* cloned_dst = dst != nullptr ? dst->clone() : nullptr;
    QuadTerm* cloned_left = left != nullptr ? left->clone() : nullptr;
    QuadTerm* cloned_right = right != nullptr ? right->clone() : nullptr;
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadMoveBinop(cloned_dst, cloned_left, binop, cloned_right, cloned_def, cloned_use);
}

QuadCall* QuadCall::clone() const {
    QuadTerm* cloned_obj = obj_term != nullptr ? obj_term->clone() : nullptr;
    vector<QuadTerm*>* cloned_args = nullptr;
    if (args != nullptr) {
        cloned_args = new vector<QuadTerm*>();
        for (QuadTerm* arg : *args) {
            cloned_args->push_back(arg != nullptr ? arg->clone() : nullptr);
        }
    }
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadCall(name, cloned_obj, cloned_args, cloned_def, cloned_use);
}

QuadMoveCall* QuadMoveCall::clone() const {
    QuadTemp* cloned_dst = dst != nullptr ? dst->clone() : nullptr;
    QuadCall* cloned_call = call != nullptr ? (QuadCall*)call->clone() : nullptr;
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadMoveCall(cloned_dst, cloned_call, cloned_def, cloned_use);
}

QuadExtCall* QuadExtCall::clone() const {
    vector<QuadTerm*>* cloned_args = nullptr;
    if (args != nullptr) {
        cloned_args = new vector<QuadTerm*>();
        for (QuadTerm* arg : *args) {
            cloned_args->push_back(arg != nullptr ? arg->clone() : nullptr);
        }
    }
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadExtCall(extfun, cloned_args, cloned_def, cloned_use);
}

QuadMoveExtCall* QuadMoveExtCall::clone() const {
    QuadTemp* cloned_dst = dst != nullptr ? dst->clone() : nullptr;
    QuadExtCall* cloned_extcall = extcall != nullptr ? (QuadExtCall*)extcall->clone() : nullptr;
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadMoveExtCall(cloned_dst, cloned_extcall, cloned_def, cloned_use);
}

QuadLabel* QuadLabel::clone() const {
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadLabel(label, cloned_def, cloned_use);
}

QuadJump* QuadJump::clone() const {
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadJump(label, cloned_def, cloned_use);
}

QuadCJump* QuadCJump::clone() const {
    QuadTerm* cloned_left = left != nullptr ? left->clone() : nullptr;
    QuadTerm* cloned_right = right != nullptr ? right->clone() : nullptr;
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadCJump(relop, cloned_left, cloned_right, t, f, cloned_def, cloned_use);
}

QuadPhi* QuadPhi::clone() const {
    QuadTemp* cloned_temp = temp_exp != nullptr ? temp_exp->clone() : nullptr;
    vector<pair<Temp*, Label*>>* cloned_args = nullptr;
    if (args != nullptr) {
        cloned_args = new vector<pair<Temp*, Label*>>();
        for (auto& arg : *args) {
            cloned_args->push_back({arg.first, arg.second});
        }
    }
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadPhi(cloned_temp, cloned_args, cloned_def, cloned_use);
}

QuadReturn* QuadReturn::clone() const {
    QuadTerm* cloned_exp = exp != nullptr ? exp->clone() : nullptr;
    set<Temp*>* cloned_def = nullptr;
    if (def != nullptr) {
        cloned_def = new set<Temp*>();
        for (Temp* temp : *def) {
            cloned_def->insert(temp);
        }
    }
    set<Temp*>* cloned_use = nullptr;
    if (use != nullptr) {
        cloned_use = new set<Temp*>();
        for (Temp* temp : *use) {
            cloned_use->insert(temp);
        }
    }
    return new QuadReturn(cloned_exp, cloned_def, cloned_use);
}
