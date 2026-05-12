#define DEBUG
#undef DEBUG

#include <string>
#include <stack>
#include <variant>
#include <vector>
#include <map>
#include <set>
#include "quad.hh"
#include "opt.hh"

using namespace quad;
using namespace tree;

namespace {

int tempNum(QuadTemp *t) {
    return t == nullptr || t->temp == nullptr ? -1 : t->temp->num;
}

int termTempNum(QuadTerm *t) {
    if (t == nullptr || t->kind != QuadTermKind::TEMP) return -1;
    return tempNum(t->get_temp());
}

int labelNum(Label *l) {
    return l == nullptr ? -1 : l->num;
}

bool sameRtValue(RtValue a, RtValue b) {
    if (a.getType() != b.getType()) return false;
    return a.getType() != ValueType::ONE_VALUE || a.getIntValue() == b.getIntValue();
}

RtValue joinValue(RtValue oldv, RtValue newv) {
    if (oldv.getType() == ValueType::MANY_VALUES ||
        newv.getType() == ValueType::MANY_VALUES) {
        return RtValue(ValueType::MANY_VALUES);
    }
    if (oldv.getType() == ValueType::NO_VALUE) return newv;
    if (newv.getType() == ValueType::NO_VALUE) return oldv;
    if (oldv.getIntValue() == newv.getIntValue()) return oldv;
    return RtValue(ValueType::MANY_VALUES);
}

bool updateTemp(map<int, RtValue> &temp_value, int num, RtValue newv) {
    if (num < 0) return false;
    RtValue oldv = temp_value.count(num) ? temp_value[num] : RtValue();
    RtValue merged = joinValue(oldv, newv);
    if (sameRtValue(oldv, merged)) return false;
    temp_value[num] = merged;
    return true;
}

RtValue termValue(Opt *opt, QuadTerm *term) {
    if (term == nullptr) return RtValue(ValueType::MANY_VALUES);
    if (term->kind == QuadTermKind::CONST) return RtValue(term->get_const());
    if (term->kind == QuadTermKind::NAME) return RtValue(ValueType::MANY_VALUES);
    QuadTemp *qt = term->get_temp();
    if (qt == nullptr) return RtValue(ValueType::MANY_VALUES);
    if (qt->type != QuadType::INT) return RtValue(ValueType::MANY_VALUES);
    return opt->getRtValue(qt->temp->num);
}

bool calcBinop(const string &op, int l, int r, int &out) {
    if (op == "+") out = l + r;
    else if (op == "-") out = l - r;
    else if (op == "*") out = l * r;
    else if (op == "/") {
        if (r == 0) return false;
        out = l / r;
    }
    else if (op == "%") {
        if (r == 0) return false;
        out = l % r;
    }
    else if (op == "&&") out = (l != 0 && r != 0) ? 1 : 0;
    else if (op == "||") out = (l != 0 || r != 0) ? 1 : 0;
    else return false;
    return true;
}

RtValue evalBinop(Opt *opt, QuadMoveBinop *stm) {
    RtValue l = termValue(opt, stm->left);
    RtValue r = termValue(opt, stm->right);
    if (l.getType() == ValueType::MANY_VALUES ||
        r.getType() == ValueType::MANY_VALUES) {
        return RtValue(ValueType::MANY_VALUES);
    }
    if (l.getType() == ValueType::NO_VALUE ||
        r.getType() == ValueType::NO_VALUE) {
        return RtValue();
    }

    int result = 0;
    if (!calcBinop(stm->binop, l.getIntValue(), r.getIntValue(), result)) {
        return RtValue(ValueType::MANY_VALUES);
    }
    return RtValue(result);
}

bool calcRelop(const string &op, int l, int r, bool &out) {
    if (op == "==") out = l == r;
    else if (op == "!=") out = l != r;
    else if (op == "<") out = l < r;
    else if (op == "<=") out = l <= r;
    else if (op == ">") out = l > r;
    else if (op == ">=") out = l >= r;
    else return false;
    return true;
}

RtValue evalRelop(Opt *opt, QuadCJump *stm) {
    RtValue l = termValue(opt, stm->left);
    RtValue r = termValue(opt, stm->right);
    if (l.getType() == ValueType::MANY_VALUES ||
        r.getType() == ValueType::MANY_VALUES) {
        return RtValue(ValueType::MANY_VALUES);
    }
    if (l.getType() == ValueType::NO_VALUE ||
        r.getType() == ValueType::NO_VALUE) {
        return RtValue();
    }

    bool result = false;
    if (!calcRelop(stm->relop, l.getIntValue(), r.getIntValue(), result)) {
        return RtValue(ValueType::MANY_VALUES);
    }
    return RtValue(result ? 1 : 0);
}

QuadStm *lastStm(QuadBlock *block) {
    if (block == nullptr || block->quadlist == nullptr || block->quadlist->empty()) {
        return nullptr;
    }
    return block->quadlist->back();
}

bool edgeExecutable(Opt *opt, int from, int to) {
    if (!opt->block_executable[from]) return false;
    QuadBlock *block = opt->label2block[from];
    QuadStm *last = lastStm(block);
    if (last == nullptr) return false;

    if (last->kind == QuadKind::JUMP) {
        return labelNum(((QuadJump *)last)->label) == to;
    }
    if (last->kind == QuadKind::CJUMP) {
        QuadCJump *cj = (QuadCJump *)last;
        RtValue cond = evalRelop(opt, cj);
        if (cond.getType() == ValueType::NO_VALUE) return false;
        if (cond.getType() == ValueType::MANY_VALUES) {
            return labelNum(cj->t) == to || labelNum(cj->f) == to;
        }
        return labelNum(cond.getIntValue() != 0 ? cj->t : cj->f) == to;
    }
    if (last->kind == QuadKind::RETURN) return false;

    if (block->exit_labels == nullptr) return false;
    for (Label *l : *block->exit_labels) {
        if (labelNum(l) == to) return true;
    }
    return false;
}

vector<int> executableSuccessors(Opt *opt, QuadBlock *block) {
    vector<int> succs;
    if (block == nullptr) return succs;
    int from = labelNum(block->entry_label);
    QuadStm *last = lastStm(block);
    if (last != nullptr && last->kind == QuadKind::JUMP) {
        int to = labelNum(((QuadJump *)last)->label);
        if (edgeExecutable(opt, from, to)) succs.push_back(to);
        return succs;
    }
    if (last != nullptr && last->kind == QuadKind::CJUMP) {
        QuadCJump *cj = (QuadCJump *)last;
        int t = labelNum(cj->t);
        int f = labelNum(cj->f);
        if (edgeExecutable(opt, from, t)) succs.push_back(t);
        if (f != t && edgeExecutable(opt, from, f)) succs.push_back(f);
        return succs;
    }

    if (block->exit_labels != nullptr) {
        for (Label *l : *block->exit_labels) {
            int to = labelNum(l);
            if (edgeExecutable(opt, from, to)) succs.push_back(to);
        }
    }
    return succs;
}

set<Temp *> *singleUseFromTerms(vector<QuadTerm *> terms) {
    set<Temp *> *use = new set<Temp *>();
    for (QuadTerm *term : terms) {
        int num = termTempNum(term);
        if (num >= 0) use->insert(new Temp(num));
    }
    return use;
}

set<Temp *> *singleDef(int num) {
    set<Temp *> *def = new set<Temp *>();
    if (num >= 0) def->insert(new Temp(num));
    return def;
}

QuadTerm *replaceConstTerm(Opt *opt, QuadTerm *term) {
    if (term == nullptr) return nullptr;
    if (term->kind == QuadTermKind::TEMP) {
        QuadTemp *qt = term->get_temp();
        if (qt != nullptr && qt->type == QuadType::INT) {
            RtValue v = opt->getRtValue(qt->temp->num);
            if (v.getType() == ValueType::ONE_VALUE) {
                return new QuadTerm(v.getIntValue());
            }
        }
    }
    return term->clone();
}

vector<QuadTerm *> *replaceConstTerms(Opt *opt, vector<QuadTerm *> *terms) {
    vector<QuadTerm *> *newTerms = new vector<QuadTerm *>();
    if (terms == nullptr) return newTerms;
    for (QuadTerm *term : *terms) {
        newTerms->push_back(replaceConstTerm(opt, term));
    }
    return newTerms;
}

QuadCall *cloneCallWithConsts(Opt *opt, QuadCall *call) {
    if (call == nullptr) return nullptr;
    return new QuadCall(call->name,
                        replaceConstTerm(opt, call->obj_term),
                        replaceConstTerms(opt, call->args),
                        call->def == nullptr ? nullptr : new set<Temp *>(*call->def),
                        call->use == nullptr ? nullptr : singleUseFromTerms({replaceConstTerm(opt, call->obj_term)}));
}

QuadMove *newConstMove(int temp, QuadType type, int value) {
    return new QuadMove(new QuadTemp(new Temp(temp), type),
                        new QuadTerm(value),
                        singleDef(temp),
                        new set<Temp *>());
}

bool tempIsOne(Opt *opt, int num) {
    return num >= 0 && opt->getRtValue(num).getType() == ValueType::ONE_VALUE;
}

struct PhiPlan {
    enum class Kind { DROP, MOVE, PHI } kind = Kind::DROP;
    QuadTerm *move_src = nullptr;
    vector<pair<Temp *, Label *>> *args = nullptr;
};

QuadType phiType(QuadPhi *phi) {
    return phi != nullptr && phi->temp_exp != nullptr ? phi->temp_exp->type : QuadType::INT;
}

}  // namespace

void Opt::calculateBT() {
    label2block.clear();
    block_executable.clear();
    temp_value.clear();

    if (func == nullptr || func->quadblocklist == nullptr || func->quadblocklist->empty()) {
        return;
    }

    for (QuadBlock *block : *func->quadblocklist) {
        int label = labelNum(block->entry_label);
        label2block[label] = block;
        block_executable[label] = false;
    }

    set<int> defined_temps;
    set<int> used_temps;
    for (QuadBlock *block : *func->quadblocklist) {
        if (block->quadlist == nullptr) continue;
        for (QuadStm *stm : *block->quadlist) {
            if (stm->def != nullptr) {
                for (Temp *temp : *stm->def) {
                    if (temp != nullptr) defined_temps.insert(temp->num);
                }
            }
            if (stm->use != nullptr) {
                for (Temp *temp : *stm->use) {
                    if (temp != nullptr) used_temps.insert(temp->num);
                }
            }
        }
    }

    for (int temp : used_temps) {
        if (!defined_temps.count(temp)) {
            temp_value[temp] = RtValue(ValueType::MANY_VALUES);
        }
    }

    if (func->params != nullptr) {
        for (Temp *param : *func->params) {
            if (param != nullptr) temp_value[param->num] = RtValue(ValueType::MANY_VALUES);
        }
    }

    block_executable[labelNum(func->quadblocklist->front()->entry_label)] = true;

    bool changed = true;
    while (changed) {
        changed = false;
        for (QuadBlock *block : *func->quadblocklist) {
            int block_label = labelNum(block->entry_label);
            if (!block_executable[block_label] || block->quadlist == nullptr) continue;

            for (QuadStm *stm : *block->quadlist) {
                switch (stm->kind) {
                    case QuadKind::MOVE: {
                        QuadMove *m = (QuadMove *)stm;
                        changed |= updateTemp(temp_value, tempNum(m->dst), termValue(this, m->src));
                        break;
                    }
                    case QuadKind::MOVE_BINOP: {
                        QuadMoveBinop *m = (QuadMoveBinop *)stm;
                        changed |= updateTemp(temp_value, tempNum(m->dst), evalBinop(this, m));
                        break;
                    }
                    case QuadKind::LOAD: {
                        changed |= updateTemp(temp_value, tempNum(((QuadLoad *)stm)->dst),
                                              RtValue(ValueType::MANY_VALUES));
                        break;
                    }
                    case QuadKind::MOVE_CALL: {
                        changed |= updateTemp(temp_value, tempNum(((QuadMoveCall *)stm)->dst),
                                              RtValue(ValueType::MANY_VALUES));
                        break;
                    }
                    case QuadKind::MOVE_EXTCALL: {
                        changed |= updateTemp(temp_value, tempNum(((QuadMoveExtCall *)stm)->dst),
                                              RtValue(ValueType::MANY_VALUES));
                        break;
                    }
                    case QuadKind::PTR_CALC: {
                        changed |= updateTemp(temp_value, termTempNum(((QuadPtrCalc *)stm)->dst),
                                              RtValue(ValueType::MANY_VALUES));
                        break;
                    }
                    case QuadKind::PHI: {
                        QuadPhi *phi = (QuadPhi *)stm;
                        RtValue value;
                        if (phi->args != nullptr) {
                            for (auto &arg : *phi->args) {
                                if (!edgeExecutable(this, labelNum(arg.second), block_label)) continue;
                                value = joinValue(value, getRtValue(arg.first->num));
                            }
                        }
                        changed |= updateTemp(temp_value, tempNum(phi->temp_exp), value);
                        break;
                    }
                    default:
                        break;
                }
            }

            for (int succ : executableSuccessors(this, block)) {
                if (!block_executable[succ]) {
                    block_executable[succ] = true;
                    changed = true;
                }
            }
        }
    }
}

void Opt::modifyFunc() {
    if (func == nullptr || func->quadblocklist == nullptr) return;

    int next_temp = func->last_temp_num;
    map<int, vector<QuadStm *>> pred_insertions;
    map<QuadPhi *, PhiPlan> phi_plans;

    for (QuadBlock *block : *func->quadblocklist) {
        int block_label = labelNum(block->entry_label);
        if (!block_executable[block_label] || block->quadlist == nullptr) continue;

        for (QuadStm *stm : *block->quadlist) {
            if (stm->kind != QuadKind::PHI) continue;
            QuadPhi *phi = (QuadPhi *)stm;
            int dst = tempNum(phi->temp_exp);
            vector<pair<Temp *, Label *>> live_args;
            if (phi->args != nullptr) {
                for (auto &arg : *phi->args) {
                    if (edgeExecutable(this, labelNum(arg.second), block_label)) {
                        live_args.push_back(arg);
                    }
                }
            }

            PhiPlan plan;
            RtValue dst_value = getRtValue(dst);
            if (live_args.empty() || dst_value.getType() == ValueType::ONE_VALUE) {
                plan.kind = PhiPlan::Kind::DROP;
            } else if (live_args.size() == 1) {
                int src = live_args.front().first->num;
                RtValue src_value = getRtValue(src);
                if (src_value.getType() == ValueType::ONE_VALUE) {
                    plan.kind = PhiPlan::Kind::DROP;
                } else {
                    plan.kind = PhiPlan::Kind::MOVE;
                    plan.move_src = new QuadTerm(new QuadTemp(new Temp(src), phiType(phi)));
                }
            } else {
                plan.kind = PhiPlan::Kind::PHI;
                plan.args = new vector<pair<Temp *, Label *>>();
                for (auto &arg : live_args) {
                    RtValue arg_value = getRtValue(arg.first->num);
                    if (arg_value.getType() == ValueType::ONE_VALUE) {
                        int fresh = ++next_temp;
                        pred_insertions[labelNum(arg.second)].push_back(
                            newConstMove(fresh, phiType(phi), arg_value.getIntValue()));
                        plan.args->push_back({new Temp(fresh), arg.second});
                    } else {
                        plan.args->push_back({new Temp(arg.first->num), arg.second});
                    }
                }
            }
            phi_plans[phi] = plan;
        }
    }

    vector<QuadBlock *> *new_blocks = new vector<QuadBlock *>();
    for (QuadBlock *block : *func->quadblocklist) {
        int block_label = labelNum(block->entry_label);
        if (!block_executable[block_label] || block->quadlist == nullptr) continue;

        vector<QuadStm *> *new_stms = new vector<QuadStm *>();
        for (QuadStm *stm : *block->quadlist) {
            if (stm->kind == QuadKind::LABEL) {
                new_stms->push_back((QuadStm *)stm->clone());
                continue;
            }

            if (stm->kind == QuadKind::PHI) {
                QuadPhi *phi = (QuadPhi *)stm;
                PhiPlan plan = phi_plans[phi];
                int dst = tempNum(phi->temp_exp);
                if (plan.kind == PhiPlan::Kind::MOVE) {
                    QuadTerm *src = replaceConstTerm(this, plan.move_src);
                    new_stms->push_back(new QuadMove(new QuadTemp(new Temp(dst), phiType(phi)),
                                                     src,
                                                     singleDef(dst),
                                                     singleUseFromTerms({src})));
                } else if (plan.kind == PhiPlan::Kind::PHI) {
                    set<Temp *> *use = new set<Temp *>();
                    for (auto &arg : *plan.args) use->insert(new Temp(arg.first->num));
                    new_stms->push_back(new QuadPhi(new QuadTemp(new Temp(dst), phiType(phi)),
                                                    plan.args,
                                                    singleDef(dst),
                                                    use));
                }
                continue;
            }

            bool is_terminator = stm->kind == QuadKind::JUMP ||
                                 stm->kind == QuadKind::CJUMP ||
                                 stm->kind == QuadKind::RETURN;
            if (is_terminator) {
                for (QuadStm *ins : pred_insertions[block_label]) {
                    new_stms->push_back(ins);
                }
            }

            switch (stm->kind) {
                case QuadKind::MOVE: {
                    QuadMove *m = (QuadMove *)stm;
                    if (!tempIsOne(this, tempNum(m->dst))) {
                        QuadTerm *src = replaceConstTerm(this, m->src);
                        new_stms->push_back(new QuadMove(m->dst->clone(), src,
                                                         singleDef(tempNum(m->dst)),
                                                         singleUseFromTerms({src})));
                    }
                    break;
                }
                case QuadKind::MOVE_BINOP: {
                    QuadMoveBinop *m = (QuadMoveBinop *)stm;
                    if (!tempIsOne(this, tempNum(m->dst))) {
                        QuadTerm *left = replaceConstTerm(this, m->left);
                        QuadTerm *right = replaceConstTerm(this, m->right);
                        new_stms->push_back(new QuadMoveBinop(m->dst->clone(), left, m->binop, right,
                                                              singleDef(tempNum(m->dst)),
                                                              singleUseFromTerms({left, right})));
                    }
                    break;
                }
                case QuadKind::LOAD: {
                    QuadLoad *m = (QuadLoad *)stm;
                    if (!tempIsOne(this, tempNum(m->dst))) {
                        QuadTerm *src = replaceConstTerm(this, m->src);
                        new_stms->push_back(new QuadLoad(m->dst->clone(), src,
                                                         singleDef(tempNum(m->dst)),
                                                         singleUseFromTerms({src})));
                    }
                    break;
                }
                case QuadKind::STORE: {
                    QuadStore *m = (QuadStore *)stm;
                    QuadTerm *src = replaceConstTerm(this, m->src);
                    QuadTerm *dst = replaceConstTerm(this, m->dst);
                    new_stms->push_back(new QuadStore(src, dst, new set<Temp *>(),
                                                      singleUseFromTerms({src, dst})));
                    break;
                }
                case QuadKind::CALL: {
                    QuadCall *m = (QuadCall *)stm;
                    new_stms->push_back(new QuadCall(m->name,
                                                     replaceConstTerm(this, m->obj_term),
                                                     replaceConstTerms(this, m->args),
                                                     new set<Temp *>(),
                                                     m->use == nullptr ? new set<Temp *>() : new set<Temp *>(*m->use)));
                    break;
                }
                case QuadKind::MOVE_CALL: {
                    QuadMoveCall *m = (QuadMoveCall *)stm;
                    if (!tempIsOne(this, tempNum(m->dst))) {
                        new_stms->push_back(new QuadMoveCall(m->dst->clone(),
                                                             cloneCallWithConsts(this, m->call),
                                                             singleDef(tempNum(m->dst)),
                                                             m->use == nullptr ? new set<Temp *>() : new set<Temp *>(*m->use)));
                    }
                    break;
                }
                case QuadKind::EXTCALL: {
                    QuadExtCall *m = (QuadExtCall *)stm;
                    vector<QuadTerm *> *args = replaceConstTerms(this, m->args);
                    new_stms->push_back(new QuadExtCall(m->extfun, args, new set<Temp *>(),
                                                        m->use == nullptr ? new set<Temp *>() : new set<Temp *>(*m->use)));
                    break;
                }
                case QuadKind::MOVE_EXTCALL: {
                    QuadMoveExtCall *m = (QuadMoveExtCall *)stm;
                    if (!tempIsOne(this, tempNum(m->dst))) {
                        vector<QuadTerm *> *args = replaceConstTerms(this, m->extcall->args);
                        QuadExtCall *call = new QuadExtCall(m->extcall->extfun, args,
                                                            new set<Temp *>(),
                                                            m->extcall->use == nullptr ? new set<Temp *>() : new set<Temp *>(*m->extcall->use));
                        new_stms->push_back(new QuadMoveExtCall(m->dst->clone(), call,
                                                                singleDef(tempNum(m->dst)),
                                                                m->use == nullptr ? new set<Temp *>() : new set<Temp *>(*m->use)));
                    }
                    break;
                }
                case QuadKind::PTR_CALC: {
                    QuadPtrCalc *m = (QuadPtrCalc *)stm;
                    QuadTerm *dst = m->dst == nullptr ? nullptr : m->dst->clone();
                    int dst_num = termTempNum(dst);
                    if (!tempIsOne(this, dst_num)) {
                        QuadTerm *ptr = replaceConstTerm(this, m->ptr);
                        QuadTerm *offset = replaceConstTerm(this, m->offset);
                        new_stms->push_back(new QuadPtrCalc(dst, ptr, offset,
                                                            singleDef(dst_num),
                                                            singleUseFromTerms({ptr, offset})));
                    }
                    break;
                }
                case QuadKind::JUMP: {
                    QuadJump *j = (QuadJump *)stm;
                    new_stms->push_back(new QuadJump(j->label, new set<Temp *>(), new set<Temp *>()));
                    break;
                }
                case QuadKind::CJUMP: {
                    QuadCJump *cj = (QuadCJump *)stm;
                    RtValue cond = evalRelop(this, cj);
                    if (cond.getType() == ValueType::ONE_VALUE) {
                        Label *target = cond.getIntValue() != 0 ? cj->t : cj->f;
                        new_stms->push_back(new QuadJump(target, new set<Temp *>(), new set<Temp *>()));
                    } else {
                        QuadTerm *left = replaceConstTerm(this, cj->left);
                        QuadTerm *right = replaceConstTerm(this, cj->right);
                        new_stms->push_back(new QuadCJump(cj->relop, left, right, cj->t, cj->f,
                                                          new set<Temp *>(),
                                                          singleUseFromTerms({left, right})));
                    }
                    break;
                }
                case QuadKind::RETURN: {
                    QuadReturn *r = (QuadReturn *)stm;
                    QuadTerm *exp = replaceConstTerm(this, r->exp);
                    new_stms->push_back(new QuadReturn(exp, new set<Temp *>(), singleUseFromTerms({exp})));
                    break;
                }
                default:
                    break;
            }
        }

        if (new_stms->empty() || (new_stms->back()->kind != QuadKind::JUMP &&
                                  new_stms->back()->kind != QuadKind::CJUMP &&
                                  new_stms->back()->kind != QuadKind::RETURN)) {
            for (QuadStm *ins : pred_insertions[block_label]) {
                new_stms->push_back(ins);
            }
        }

        vector<Label *> *new_exits = new vector<Label *>();
        QuadStm *last = new_stms->empty() ? nullptr : new_stms->back();
        if (last != nullptr && last->kind == QuadKind::JUMP) {
            new_exits->push_back(((QuadJump *)last)->label);
        } else if (last != nullptr && last->kind == QuadKind::CJUMP) {
            new_exits->push_back(((QuadCJump *)last)->t);
            new_exits->push_back(((QuadCJump *)last)->f);
        }

        new_blocks->push_back(new QuadBlock(new_stms, block->entry_label, new_exits));
    }

    func->quadblocklist = new_blocks;
    func->last_temp_num = next_temp + 2;
}

QuadFuncDecl* Opt::optFunc() {
    calculateBT();
    modifyFunc();
    return func;
}

QuadProgram* optProg(QuadProgram* prog) {
    QuadProgram* newProg = new QuadProgram(new vector<QuadFuncDecl*>(), prog->last_label_num, prog->last_temp_num);
    for (int i=0; i < prog->quadFuncDeclList->size(); i++) {
        Opt optthis(prog->quadFuncDeclList->at(i));
        newProg->quadFuncDeclList->push_back(optthis.optFunc());
    }
    return newProg;
}
