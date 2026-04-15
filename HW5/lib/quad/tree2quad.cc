#define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "treep.hh"
#include "quad.hh"
#include "tree2quad.hh"

using namespace std;
using namespace tree;
using namespace quad;

namespace
{

    QuadType to_quad_type(tree::Type t)
    {
        return (t == tree::Type::PTR) ? QuadType::PTR : QuadType::INT;
    }

    set<Temp *> *new_temp_set()
    {
        return new set<Temp *>();
    }

    void add_term_to_use(set<Temp *> *use, QuadTerm *term)
    {
        if (use == nullptr || term == nullptr)
        {
            return;
        }
        if (term->kind == QuadTermKind::TEMP)
        {
            QuadTemp *qt = term->get_temp();
            if (qt != nullptr && qt->temp != nullptr)
            {
                use->insert(qt->temp);
            }
        }
    }

    QuadTemp *term_as_quadtemp(QuadTerm *term)
    {
        if (term == nullptr || term->kind != QuadTermKind::TEMP)
        {
            return nullptr;
        }
        return term->get_temp();
    }

    void append_stms(vector<QuadStm *> *dst, vector<QuadStm *> *src)
    {
        if (dst == nullptr || src == nullptr)
        {
            return;
        }
        dst->insert(dst->end(), src->begin(), src->end());
    }

    void visit_call_operands(Tree2Quad &v, tree::Exp *obj, vector<tree::Exp *> *args,
                             vector<QuadStm *> *out_stms, QuadTerm *&obj_term,
                             vector<QuadTerm *> *&arg_terms)
    {
        obj_term = nullptr;
        arg_terms = new vector<QuadTerm *>();

        if (obj != nullptr)
        {
            v.visit_result = new vector<QuadStm *>();
            obj->accept(v);
            append_stms(out_stms, v.visit_result);
            obj_term = v.output_term;
        }

        if (args == nullptr)
        {
            return;
        }
        for (auto arg : *args)
        {
            v.visit_result = new vector<QuadStm *>();
            if (arg != nullptr)
            {
                arg->accept(v);
                append_stms(out_stms, v.visit_result);
                arg_terms->push_back(v.output_term);
            }
        }
    }

    void visit_extcall_args(Tree2Quad &v, vector<tree::Exp *> *args,
                            vector<QuadStm *> *out_stms, vector<QuadTerm *> *&arg_terms)
    {
        arg_terms = new vector<QuadTerm *>();
        if (args == nullptr)
        {
            return;
        }
        for (auto arg : *args)
        {
            v.visit_result = new vector<QuadStm *>();
            if (arg != nullptr)
            {
                arg->accept(v);
                append_stms(out_stms, v.visit_result);
                arg_terms->push_back(v.output_term);
            }
        }
    }

} // namespace

QuadProgram *tree2quad(Program *prog)
{
#ifdef DEBUG
    cout << "in Tree2Quad::Converting IR to Quad" << endl;
#endif
    if (prog == nullptr)
    {
        return nullptr;
    }

    Tree2Quad v;

    v.quadprog = nullptr;
    v.visit_result = new vector<QuadStm *>();
    v.output_term = nullptr;
    v.temp_map = nullptr;

    prog->accept(v);
    return v.quadprog;
}

void Tree2Quad::visit(tree::Program *prog)
{
    if (prog == nullptr)
    {
        quadprog = nullptr;
        return;
    }

    vector<QuadFuncDecl *> *quadfuncdecllist = new vector<QuadFuncDecl *>();
    quadprog = new QuadProgram(quadfuncdecllist, 0, 0);

    if (prog->funcdecllist == nullptr)
    {
        return;
    }

    int max_label = 0;
    int max_temp = 0;
    for (auto func : *prog->funcdecllist)
    {
        if (func == nullptr)
        {
            continue;
        }
        visit_result = new vector<QuadStm *>();
        output_term = nullptr;
        func->accept(*this);
        if (!quadprog->quadFuncDeclList->empty())
        {
            QuadFuncDecl *last = quadprog->quadFuncDeclList->back();
            if (last != nullptr)
            {
                if (last->last_label_num > max_label)
                    max_label = last->last_label_num;
                if (last->last_temp_num > max_temp)
                    max_temp = last->last_temp_num;
            }
        }
    }
    quadprog->last_label_num = max_label;
    quadprog->last_temp_num = max_temp;
}

void Tree2Quad::visit(tree::FuncDecl *func)
{
    if (func == nullptr || quadprog == nullptr)
    {
        return;
    }

    if (temp_map != nullptr)
    {
        delete temp_map;
    }
    temp_map = new Temp_map();
    // Reserve one id gap to better match the reference numbering style.
    const int temp_start = func->last_temp_num + 2;
    temp_map->next_temp = temp_start;
    temp_map->next_label = func->last_label_num + 1;

    vector<QuadStm *> *quadlist = new vector<QuadStm *>();
    Label *entry = temp_map->newlabel();
    quadlist->push_back(new QuadLabel(entry, new_temp_set(), new_temp_set()));

    if (func->stm != nullptr)
    {
        visit_result = new vector<QuadStm *>();
        output_term = nullptr;
        func->stm->accept(*this);
        append_stms(quadlist, visit_result);
    }

    vector<QuadBlock *> *blocks = new vector<QuadBlock *>();
    blocks->push_back(new QuadBlock(quadlist, entry, nullptr));

    int func_last_temp = func->last_temp_num;
    if (temp_map->next_temp > temp_start)
    {
        func_last_temp = temp_map->next_temp - 1;
    }

    QuadFuncDecl *qf = new QuadFuncDecl(
        func->name,
        func->args,
        blocks,
        temp_map->next_label - 1,
        func_last_temp);
    quadprog->quadFuncDeclList->push_back(qf);
}

void Tree2Quad::visit(tree::Jump *jump)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (jump == nullptr)
    {
        return;
    }
    visit_result->push_back(new QuadJump(jump->label, new_temp_set(), new_temp_set()));
}

void Tree2Quad::visit(tree::Cjump *cjump)
{
    vector<QuadStm *> *result = new vector<QuadStm *>();
    visit_result = result;
    output_term = nullptr;
    if (cjump == nullptr)
    {
        return;
    }

    vector<QuadStm *> *left_stms = new vector<QuadStm *>();
    vector<QuadStm *> *right_stms = new vector<QuadStm *>();

    QuadTerm *left_term = nullptr;
    QuadTerm *right_term = nullptr;

    if (cjump->left != nullptr)
    {
        cjump->left->accept(*this);
        left_stms = visit_result;
        left_term = output_term;
    }
    if (cjump->right != nullptr)
    {
        cjump->right->accept(*this);
        right_stms = visit_result;
        right_term = output_term;
    }

    append_stms(result, left_stms);
    append_stms(result, right_stms);

    set<Temp *> *def = new_temp_set();
    set<Temp *> *use = new_temp_set();
    add_term_to_use(use, left_term);
    add_term_to_use(use, right_term);
    result->push_back(new QuadCJump(cjump->relop, left_term, right_term, cjump->t, cjump->f, def, use));
    visit_result = result;
}

void Tree2Quad::visit(tree::Move *move)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (move == nullptr || move->dst == nullptr || move->src == nullptr)
    {
        return;
    }

    if (move->dst->getTreeKind() == Kind::TEMPEXP)
    {
        TempExp *dst_temp_exp = static_cast<TempExp *>(move->dst);
        QuadTemp *dst = new QuadTemp(dst_temp_exp->temp, to_quad_type(dst_temp_exp->type));

        if (move->src->getTreeKind() == Kind::MEM)
        {
            Mem *src_mem = static_cast<Mem *>(move->src);
            vector<QuadStm *> *addr_stms = new vector<QuadStm *>();
            QuadTerm *addr_term = nullptr;
            if (src_mem->mem != nullptr)
            {
                src_mem->mem->accept(*this);
                addr_stms = visit_result;
                addr_term = output_term;
            }

            vector<QuadStm *> *result = new vector<QuadStm *>();
            append_stms(result, addr_stms);
            set<Temp *> *def = new_temp_set();
            set<Temp *> *use = new_temp_set();
            def->insert(dst->temp);
            add_term_to_use(use, addr_term);
            result->push_back(new QuadLoad(dst, addr_term, def, use));
            visit_result = result;
            return;
        }

        if (move->src->getTreeKind() == Kind::BINOP)
        {
            Binop *src_bin = static_cast<Binop *>(move->src);
            vector<QuadStm *> *left_stms = new vector<QuadStm *>();
            vector<QuadStm *> *right_stms = new vector<QuadStm *>();
            QuadTerm *left_term = nullptr;
            QuadTerm *right_term = nullptr;

            if (src_bin->left != nullptr)
            {
                src_bin->left->accept(*this);
                left_stms = visit_result;
                left_term = output_term;
            }
            if (src_bin->right != nullptr)
            {
                src_bin->right->accept(*this);
                right_stms = visit_result;
                right_term = output_term;
            }

            vector<QuadStm *> *result = new vector<QuadStm *>();
            append_stms(result, left_stms);
            append_stms(result, right_stms);

            set<Temp *> *def = new_temp_set();
            set<Temp *> *use = new_temp_set();
            def->insert(dst->temp);
            add_term_to_use(use, left_term);
            add_term_to_use(use, right_term);

            if (src_bin->type == tree::Type::PTR && src_bin->op == "+")
            {
                result->push_back(new QuadPtrCalc(new QuadTerm(dst), left_term, right_term, def, use));
            }
            else
            {
                result->push_back(new QuadMoveBinop(dst, left_term, src_bin->op, right_term, def, use));
            }
            visit_result = result;
            return;
        }

        if (move->src->getTreeKind() == Kind::CALL)
        {
            Call *call = static_cast<Call *>(move->src);
            vector<QuadStm *> *prefix = new vector<QuadStm *>();
            QuadTerm *obj_term = nullptr;
            vector<QuadTerm *> *args = nullptr;
            visit_call_operands(*this, call->obj, call->args, prefix, obj_term, args);
            append_stms(visit_result, prefix);

            set<Temp *> *call_def = new_temp_set();
            set<Temp *> *call_use = new_temp_set();
            add_term_to_use(call_use, obj_term);
            for (auto arg : *args)
                add_term_to_use(call_use, arg);

            QuadCall *qcall = new QuadCall(call->id, obj_term, args, call_def, call_use);
            set<Temp *> *def = new_temp_set();
            set<Temp *> *use = new_temp_set();
            def->insert(dst->temp);
            for (auto t : *call_use)
                use->insert(t);
            visit_result->push_back(new QuadMoveCall(dst, qcall, def, use));
            return;
        }

        if (move->src->getTreeKind() == Kind::EXTCALL)
        {
            ExtCall *extcall = static_cast<ExtCall *>(move->src);
            vector<QuadStm *> *prefix = new vector<QuadStm *>();
            vector<QuadTerm *> *args = nullptr;
            visit_extcall_args(*this, extcall->args, prefix, args);
            append_stms(visit_result, prefix);

            set<Temp *> *ext_def = new_temp_set();
            set<Temp *> *ext_use = new_temp_set();
            for (auto arg : *args)
                add_term_to_use(ext_use, arg);

            QuadExtCall *qext = new QuadExtCall(extcall->extfun, args, ext_def, ext_use);
            set<Temp *> *def = new_temp_set();
            set<Temp *> *use = new_temp_set();
            def->insert(dst->temp);
            for (auto t : *ext_use)
                use->insert(t);
            visit_result->push_back(new QuadMoveExtCall(dst, qext, def, use));
            return;
        }

        move->src->accept(*this);
        vector<QuadStm *> *src_stms = visit_result;
        QuadTerm *src_term = output_term;

        vector<QuadStm *> *result = new vector<QuadStm *>();
        append_stms(result, src_stms);

        set<Temp *> *def = new_temp_set();
        set<Temp *> *use = new_temp_set();
        def->insert(dst->temp);
        add_term_to_use(use, src_term);
        result->push_back(new QuadMove(dst, src_term, def, use));
        visit_result = result;
        return;
    }

    if (move->dst->getTreeKind() == Kind::MEM)
    {
        Mem *dst_mem = static_cast<Mem *>(move->dst);

        vector<QuadStm *> *addr_stms = new vector<QuadStm *>();
        QuadTerm *addr_term = nullptr;
        if (dst_mem->mem != nullptr)
        {
            dst_mem->mem->accept(*this);
            addr_stms = visit_result;
            addr_term = output_term;
        }

        vector<QuadStm *> *src_stms = new vector<QuadStm *>();
        QuadTerm *src_term = nullptr;
        if (move->src != nullptr)
        {
            move->src->accept(*this);
            src_stms = visit_result;
            src_term = output_term;
        }

        visit_result = new vector<QuadStm *>();
        append_stms(visit_result, addr_stms);
        append_stms(visit_result, src_stms);

        set<Temp *> *def = new_temp_set();
        set<Temp *> *use = new_temp_set();
        add_term_to_use(use, src_term);
        add_term_to_use(use, addr_term);
        visit_result->push_back(new QuadStore(src_term, addr_term, def, use));
        return;
    }

    move->src->accept(*this);
}

void Tree2Quad::visit(tree::Seq *seq)
{
    vector<QuadStm *> *result = new vector<QuadStm *>();
    visit_result = result;
    output_term = nullptr;
    if (seq == nullptr || seq->sl == nullptr)
    {
        return;
    }

    for (auto stm : *seq->sl)
    {
        if (stm == nullptr)
        {
            continue;
        }
        stm->accept(*this);
        append_stms(result, this->visit_result);
    }
    visit_result = result;
}

void Tree2Quad::visit(tree::LabelStm *labelstm)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (labelstm == nullptr)
    {
        return;
    }
    visit_result->push_back(new QuadLabel(labelstm->label, new_temp_set(), new_temp_set()));
}

void Tree2Quad::visit(tree::Return *ret)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (ret == nullptr || ret->exp == nullptr)
    {
        return;
    }

    ret->exp->accept(*this);
    vector<QuadStm *> *exp_stms = visit_result;
    QuadTerm *ret_term = output_term;

    visit_result = new vector<QuadStm *>();
    append_stms(visit_result, exp_stms);

    set<Temp *> *def = new_temp_set();
    set<Temp *> *use = new_temp_set();
    add_term_to_use(use, ret_term);
    visit_result->push_back(new QuadReturn(ret_term, def, use));
}

void Tree2Quad::visit(tree::ExpStm *exp)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (exp == nullptr || exp->exp == nullptr)
    {
        return;
    }

    if (exp->exp->getTreeKind() == Kind::CALL)
    {
        Call *call = static_cast<Call *>(exp->exp);
        vector<QuadStm *> *prefix = new vector<QuadStm *>();
        QuadTerm *obj_term = nullptr;
        vector<QuadTerm *> *args = nullptr;
        visit_call_operands(*this, call->obj, call->args, prefix, obj_term, args);
        append_stms(visit_result, prefix);

        set<Temp *> *def = new_temp_set();
        set<Temp *> *use = new_temp_set();
        add_term_to_use(use, obj_term);
        for (auto arg : *args)
            add_term_to_use(use, arg);
        visit_result->push_back(new QuadCall(call->id, obj_term, args, def, use));
        return;
    }

    if (exp->exp->getTreeKind() == Kind::EXTCALL)
    {
        ExtCall *extcall = static_cast<ExtCall *>(exp->exp);
        vector<QuadStm *> *prefix = new vector<QuadStm *>();
        vector<QuadTerm *> *args = nullptr;
        visit_extcall_args(*this, extcall->args, prefix, args);
        append_stms(visit_result, prefix);

        set<Temp *> *def = new_temp_set();
        set<Temp *> *use = new_temp_set();
        for (auto arg : *args)
            add_term_to_use(use, arg);
        visit_result->push_back(new QuadExtCall(extcall->extfun, args, def, use));
        return;
    }

    exp->exp->accept(*this);
}

void Tree2Quad::visit(tree::Binop *binop)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (binop == nullptr || temp_map == nullptr)
    {
        return;
    }

    vector<QuadStm *> *left_stms = new vector<QuadStm *>();
    vector<QuadStm *> *right_stms = new vector<QuadStm *>();
    QuadTerm *left_term = nullptr;
    QuadTerm *right_term = nullptr;

    if (binop->left != nullptr)
    {
        binop->left->accept(*this);
        left_stms = visit_result;
        left_term = output_term;
    }
    if (binop->right != nullptr)
    {
        binop->right->accept(*this);
        right_stms = visit_result;
        right_term = output_term;
    }

    visit_result = new vector<QuadStm *>();
    append_stms(visit_result, left_stms);
    append_stms(visit_result, right_stms);

    QuadTemp *dst_qtemp = new QuadTemp(temp_map->newtemp(), to_quad_type(binop->type));
    output_term = new QuadTerm(dst_qtemp);

    set<Temp *> *def = new_temp_set();
    set<Temp *> *use = new_temp_set();
    def->insert(dst_qtemp->temp);
    add_term_to_use(use, left_term);
    add_term_to_use(use, right_term);

    if (binop->type == tree::Type::PTR && binop->op == "+")
    {
        visit_result->push_back(new QuadPtrCalc(output_term, left_term, right_term, def, use));
    }
    else
    {
        visit_result->push_back(new QuadMoveBinop(dst_qtemp, left_term, binop->op, right_term, def, use));
    }
}

void Tree2Quad::visit(tree::Mem *mem)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (mem == nullptr || mem->mem == nullptr || temp_map == nullptr)
    {
        return;
    }

    mem->mem->accept(*this);
    vector<QuadStm *> *addr_stms = visit_result;
    QuadTerm *addr_term = output_term;

    visit_result = new vector<QuadStm *>();
    append_stms(visit_result, addr_stms);

    QuadTemp *dst = new QuadTemp(temp_map->newtemp(), to_quad_type(mem->type));
    output_term = new QuadTerm(dst);

    set<Temp *> *def = new_temp_set();
    set<Temp *> *use = new_temp_set();
    def->insert(dst->temp);
    add_term_to_use(use, addr_term);
    visit_result->push_back(new QuadLoad(dst, addr_term, def, use));
}

void Tree2Quad::visit(tree::TempExp *tempexp)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (tempexp == nullptr)
    {
        return;
    }
    output_term = new QuadTerm(new QuadTemp(tempexp->temp, to_quad_type(tempexp->type)));
}

void Tree2Quad::visit(tree::Eseq *eseq)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (eseq == nullptr)
    {
        return;
    }

    vector<QuadStm *> *stm_stms = new vector<QuadStm *>();
    if (eseq->stm != nullptr)
    {
        eseq->stm->accept(*this);
        stm_stms = visit_result;
    }

    vector<QuadStm *> *exp_stms = new vector<QuadStm *>();
    QuadTerm *exp_term = nullptr;
    if (eseq->exp != nullptr)
    {
        eseq->exp->accept(*this);
        exp_stms = visit_result;
        exp_term = output_term;
    }

    visit_result = new vector<QuadStm *>();
    append_stms(visit_result, stm_stms);
    append_stms(visit_result, exp_stms);
    output_term = exp_term;
}

void Tree2Quad::visit(tree::Name *name)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (name == nullptr)
    {
        return;
    }

    if (name->sname != nullptr)
    {
        output_term = new QuadTerm(name->sname->name);
        return;
    }
    if (name->name != nullptr)
    {
        output_term = new QuadTerm("L" + to_string(name->name->num));
    }
}

void Tree2Quad::visit(tree::Const *const_exp)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (const_exp == nullptr)
    {
        return;
    }
    output_term = new QuadTerm(const_exp->constVal);
}

void Tree2Quad::visit(tree::Call *call)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (call == nullptr || temp_map == nullptr)
    {
        return;
    }

    vector<QuadStm *> *prefix = new vector<QuadStm *>();
    QuadTerm *obj_term = nullptr;
    vector<QuadTerm *> *args = nullptr;
    visit_call_operands(*this, call->obj, call->args, prefix, obj_term, args);
    append_stms(visit_result, prefix);

    set<Temp *> *call_def = new_temp_set();
    set<Temp *> *call_use = new_temp_set();
    add_term_to_use(call_use, obj_term);
    for (auto arg : *args)
        add_term_to_use(call_use, arg);
    QuadCall *qcall = new QuadCall(call->id, obj_term, args, call_def, call_use);

    QuadTemp *dst = new QuadTemp(temp_map->newtemp(), to_quad_type(call->type));
    output_term = new QuadTerm(dst);

    set<Temp *> *def = new_temp_set();
    set<Temp *> *use = new_temp_set();
    def->insert(dst->temp);
    for (auto t : *call_use)
        use->insert(t);
    visit_result->push_back(new QuadMoveCall(dst, qcall, def, use));
}

void Tree2Quad::visit(tree::ExtCall *extcall)
{
    visit_result = new vector<QuadStm *>();
    output_term = nullptr;
    if (extcall == nullptr || temp_map == nullptr)
    {
        return;
    }

    vector<QuadStm *> *prefix = new vector<QuadStm *>();
    vector<QuadTerm *> *args = nullptr;
    visit_extcall_args(*this, extcall->args, prefix, args);
    append_stms(visit_result, prefix);

    set<Temp *> *ext_def = new_temp_set();
    set<Temp *> *ext_use = new_temp_set();
    for (auto arg : *args)
        add_term_to_use(ext_use, arg);
    QuadExtCall *qext = new QuadExtCall(extcall->extfun, args, ext_def, ext_use);

    QuadTemp *dst = new QuadTemp(temp_map->newtemp(), to_quad_type(extcall->type));
    output_term = new QuadTerm(dst);

    set<Temp *> *def = new_temp_set();
    set<Temp *> *use = new_temp_set();
    def->insert(dst->temp);
    for (auto t : *ext_use)
        use->insert(t);
    visit_result->push_back(new QuadMoveExtCall(dst, qext, def, use));
}
