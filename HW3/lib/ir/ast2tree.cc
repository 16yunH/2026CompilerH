#define DEBUG
#undef DEBUG

#include <algorithm>
#include <iostream> // IWYU pragma: keep
#include <map>
#include <string>
#include <variant>
#include <vector>
#include "ASTheader.hh" // IWYU pragma: keep
#include "FDMJAST.hh"   // IWYU pragma: keep
#include "ast2tree.hh"  // IWYU pragma: keep
#include "config.hh"    // IWYU pragma: keep
#include "temp.hh"      // IWYU pragma: keep
#include "treep.hh"     // IWYU pragma: keep

using namespace std;

static tree::Type type_to_tree_type(fdmj::TypeKind tk)
{
    if (tk == fdmj::TypeKind::CLASS || tk == fdmj::TypeKind::ARRAY)
    {
        return tree::Type::PTR;
    }
    return tree::Type::INT;
}

static tree::Type type_to_tree_type(fdmj::Type *t)
{
    if (t == nullptr)
    {
        return tree::Type::INT;
    }
    return type_to_tree_type(t->typeKind);
}

static tree::Type semant_to_tree_type(AST_Semant *s)
{
    if (s == nullptr)
    {
        return tree::Type::INT;
    }
    return type_to_tree_type(s->get_type());
}

static tree::Seq *make_seq(vector<tree::Stm *> *sl)
{
    if (sl == nullptr)
    {
        return new tree::Seq();
    }
    return new tree::Seq(sl);
}

tree::Program *ast2tree(fdmj::Program *prog, AST_Semant_Map *semant_map)
{
    ASTToTreeVisitor vis;
    vis.semant_map = semant_map;
    vis.class_table = generate_class_table(semant_map);
    prog->accept(vis);
    return static_cast<tree::Program *>(vis.getTree());
}

Class_table *generate_class_table(AST_Semant_Map *semant_map)
{
    Class_table *ct = new Class_table();
    if (semant_map == nullptr || semant_map->getNameMaps() == nullptr)
    {
        return ct;
    }

    Name_Maps *nm = semant_map->getNameMaps();
    int var_pos = 0;
    int method_pos = 0;
    set<string> *cl = nm->get_class_list();
    for (const auto &c : *cl)
    {
        set<string> *vl = nm->get_class_var_list(c);
        for (const auto &v : *vl)
        {
            ct->var_pos_map[c + "^" + v] = var_pos++;
        }
        delete vl;
        set<string> *ml = nm->get_method_list(c);
        for (const auto &m : *ml)
        {
            if (ct->method_pos_map.find(m) == ct->method_pos_map.end())
            {
                ct->method_pos_map[m] = method_pos++;
            }
        }
        delete ml;
    }
    return ct;
}

Method_var_table *generate_method_var_table(string class_name, string method_name, Name_Maps *nm, Temp_map *tm)
{
    Method_var_table *mvt = new Method_var_table();
    if (nm == nullptr || tm == nullptr)
    {
        return mvt;
    }

    set<string> *vars = nm->get_method_var_list(class_name, method_name);
    if (vars != nullptr)
    {
        for (const auto &vname : *vars)
        {
            VarDecl *vd = nm->get_method_var(class_name, method_name, vname);
            if (vd == nullptr || vd->type == nullptr)
            {
                continue;
            }
            tree::Temp *t = tm->newtemp();
            (*mvt->var_temp_map)[vname] = t;
            (*mvt->var_type_map)[vname] = type_to_tree_type(vd->type);
        }
        delete vars;
    }

    vector<string> *formals = nm->get_method_formal_list_string(class_name, method_name);
    if (formals != nullptr)
    {
        for (const auto &fname : *formals)
        {
            Formal *f = nm->get_method_formal(class_name, method_name, fname);
            if (f == nullptr || f->type == nullptr)
            {
                continue;
            }
            tree::Temp *t = tm->newtemp();
            (*mvt->var_temp_map)[fname] = t;
            (*mvt->var_type_map)[fname] = type_to_tree_type(f->type);
        }
        delete formals;
    }

    return mvt;
}

void ASTToTreeVisitor::visit(fdmj::Program *node)
{
    if (node == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }
    vector<tree::FuncDecl *> *funcs = new vector<tree::FuncDecl *>();
    if (node->main != nullptr)
    {
        node->main->accept(*this);
        if (visit_tree_result != nullptr)
        {
            funcs->push_back(static_cast<tree::FuncDecl *>(visit_tree_result));
        }
    }
    visit_tree_result = new tree::Program(funcs);
}

void ASTToTreeVisitor::visit(fdmj::MainMethod *node)
{
    current_class = "__$main__";
    current_method = "main";
    method_temp_map = new Temp_map();
    method_var_table = generate_method_var_table(current_class, current_method, semant_map->getNameMaps(), method_temp_map);

    vector<tree::Stm *> *body = new vector<tree::Stm *>();
    if (node->vdl != nullptr)
    {
        for (auto *vd : *node->vdl)
        {
            if (vd == nullptr)
            {
                continue;
            }
            vd->accept(*this);
            if (visit_tree_result != nullptr)
            {
                body->push_back(static_cast<tree::Stm *>(visit_tree_result));
            }
        }
    }
    if (node->sl != nullptr)
    {
        for (auto *stm : *node->sl)
        {
            if (stm == nullptr)
            {
                continue;
            }
            stm->accept(*this);
            if (visit_tree_result != nullptr)
            {
                body->push_back(static_cast<tree::Stm *>(visit_tree_result));
            }
        }
    }

    tree::FuncDecl *fd = new tree::FuncDecl(
        current_class + "^" + current_method,
        nullptr,
        new tree::Seq(body),
        tree::Type::INT,
        method_temp_map->next_temp - 1,
        method_temp_map->next_label - 1);
    visit_tree_result = fd;
}

void ASTToTreeVisitor::visit(fdmj::ClassDecl *node)
{
    (void)node;
    visit_tree_result = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::Type *node)
{
    (void)node;
    visit_tree_result = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::VarDecl *node)
{
    if (node == nullptr || node->id == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }
    tree::Temp *t = method_var_table->get_var_temp(node->id->id);
    if (t == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }

    if (holds_alternative<fdmj::IntExp *>(node->init))
    {
        auto *ie = get<fdmj::IntExp *>(node->init);
        if (ie != nullptr)
        {
            visit_tree_result = new tree::Move(new tree::TempExp(tree::Type::INT, t), new tree::Const(ie->val));
            return;
        }
    }
    visit_tree_result = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::MethodDecl *node)
{
    (void)node;
    visit_tree_result = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::Formal *node)
{
    (void)node;
    visit_tree_result = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::Nested *node)
{
    vector<tree::Stm *> *stms = new vector<tree::Stm *>();
    if (node != nullptr && node->sl != nullptr)
    {
        for (auto *s : *node->sl)
        {
            if (s == nullptr)
            {
                continue;
            }
            s->accept(*this);
            if (visit_tree_result != nullptr)
            {
                stms->push_back(static_cast<tree::Stm *>(visit_tree_result));
            }
        }
    }
    visit_tree_result = new tree::Seq(stms);
}

void ASTToTreeVisitor::visit(fdmj::If *node)
{
    if (node == nullptr || node->exp == nullptr || node->stm1 == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }

    node->exp->accept(*this);
    Tr_cx *cond = visit_exp_result->unCx(method_temp_map);

    node->stm1->accept(*this);
    tree::Stm *then_stm = static_cast<tree::Stm *>(visit_tree_result);

    tree::Stm *else_stm = nullptr;
    if (node->stm2 != nullptr)
    {
        node->stm2->accept(*this);
        else_stm = static_cast<tree::Stm *>(visit_tree_result);
    }

    tree::Label *l_true = method_temp_map->newlabel();
    tree::Label *l_false = method_temp_map->newlabel();
    tree::Label *l_done = method_temp_map->newlabel();
    cond->true_list->patch(l_true);
    cond->false_list->patch(l_false);

    vector<tree::Stm *> *sl = new vector<tree::Stm *>();
    sl->push_back(cond->stm);
    sl->push_back(new tree::LabelStm(l_true));
    if (then_stm != nullptr)
    {
        sl->push_back(then_stm);
    }
    sl->push_back(new tree::Jump(l_done));
    sl->push_back(new tree::LabelStm(l_false));
    if (else_stm != nullptr)
    {
        sl->push_back(else_stm);
    }
    sl->push_back(new tree::LabelStm(l_done));
    visit_tree_result = new tree::Seq(sl);
}

void ASTToTreeVisitor::visit(fdmj::While *node)
{
    if (node == nullptr || node->exp == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }

    node->exp->accept(*this);
    Tr_cx *cond = visit_exp_result->unCx(method_temp_map);

    tree::Label *l_head = method_temp_map->newlabel();
    tree::Label *l_body = method_temp_map->newlabel();
    tree::Label *l_done = method_temp_map->newlabel();
    cond->true_list->patch(l_body);
    cond->false_list->patch(l_done);

    tree::Label *old_continue = continue_label;
    tree::Label *old_break = break_label;
    continue_label = l_head;
    break_label = l_done;

    tree::Stm *body_stm = nullptr;
    if (node->stm != nullptr)
    {
        node->stm->accept(*this);
        body_stm = static_cast<tree::Stm *>(visit_tree_result);
    }

    vector<tree::Stm *> *sl = new vector<tree::Stm *>();
    sl->push_back(new tree::LabelStm(l_head));
    sl->push_back(cond->stm);
    sl->push_back(new tree::LabelStm(l_body));
    if (body_stm != nullptr)
    {
        sl->push_back(body_stm);
    }
    sl->push_back(new tree::Jump(l_head));
    sl->push_back(new tree::LabelStm(l_done));

    continue_label = old_continue;
    break_label = old_break;
    visit_tree_result = new tree::Seq(sl);
}

void ASTToTreeVisitor::visit(fdmj::Assign *node)
{
    if (node == nullptr || node->left == nullptr || node->exp == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }
    node->left->accept(*this);
    tree::Exp *dst = visit_exp_result->unEx(method_temp_map)->exp;
    node->exp->accept(*this);
    tree::Exp *src = visit_exp_result->unEx(method_temp_map)->exp;
    visit_tree_result = new tree::Move(dst, src);
}

void ASTToTreeVisitor::visit(fdmj::CallStm *node)
{
    if (node == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }
    vector<tree::Exp *> *args = new vector<tree::Exp *>();
    if (node->par != nullptr)
    {
        for (auto *p : *node->par)
        {
            p->accept(*this);
            args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
        }
    }
    string name = node->name == nullptr ? "" : node->name->id;
    visit_tree_result = new tree::ExpStm(new tree::ExtCall(tree::Type::INT, name, args));
}

void ASTToTreeVisitor::visit(fdmj::Continue *node)
{
    (void)node;
    visit_tree_result = new tree::Jump(continue_label);
}

void ASTToTreeVisitor::visit(fdmj::Break *node)
{
    (void)node;
    visit_tree_result = new tree::Jump(break_label);
}

void ASTToTreeVisitor::visit(fdmj::Return *node)
{
    if (node == nullptr || node->exp == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }
    node->exp->accept(*this);
    visit_tree_result = new tree::Return(visit_exp_result->unEx(method_temp_map)->exp);
}

void ASTToTreeVisitor::visit(fdmj::PutInt *node)
{
    if (node == nullptr || node->exp == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }
    node->exp->accept(*this);
    vector<tree::Exp *> *args = new vector<tree::Exp *>();
    args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
    visit_tree_result = new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "putint", args));
}

void ASTToTreeVisitor::visit(fdmj::PutCh *node)
{
    if (node == nullptr || node->exp == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }
    node->exp->accept(*this);
    vector<tree::Exp *> *args = new vector<tree::Exp *>();
    args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
    visit_tree_result = new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "putch", args));
}

void ASTToTreeVisitor::visit(fdmj::PutArray *node)
{
    if (node == nullptr || node->n == nullptr || node->arr == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }
    node->n->accept(*this);
    tree::Exp *n = visit_exp_result->unEx(method_temp_map)->exp;
    node->arr->accept(*this);
    tree::Exp *arr = visit_exp_result->unEx(method_temp_map)->exp;
    vector<tree::Exp *> *args = new vector<tree::Exp *>();
    args->push_back(n);
    args->push_back(arr);
    visit_tree_result = new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "putarray", args));
}

void ASTToTreeVisitor::visit(fdmj::Starttime *node)
{
    (void)node;
    visit_tree_result = new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "starttime", new vector<tree::Exp *>()));
}

void ASTToTreeVisitor::visit(fdmj::Stoptime *node)
{
    (void)node;
    visit_tree_result = new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "stoptime", new vector<tree::Exp *>()));
}

void ASTToTreeVisitor::visit(fdmj::BinaryOp *node)
{
    if (node == nullptr || node->left == nullptr || node->right == nullptr || node->op == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }

    string op = node->op->op;

    if (op == "&&" || op == "||")
    {
        node->left->accept(*this);
        Tr_cx *lcx = visit_exp_result->unCx(method_temp_map);
        node->right->accept(*this);
        Tr_cx *rcx = visit_exp_result->unCx(method_temp_map);

        tree::Label *mid = method_temp_map->newlabel();
        vector<tree::Stm *> *sl = new vector<tree::Stm *>();
        if (op == "&&")
        {
            lcx->true_list->patch(mid);
            sl->push_back(lcx->stm);
            sl->push_back(new tree::LabelStm(mid));
            sl->push_back(rcx->stm);
            Patch_list *t = rcx->true_list;
            Patch_list *f = lcx->false_list;
            f->add(rcx->false_list);
            visit_exp_result = new Tr_cx(t, f, make_seq(sl));
            return;
        }

        lcx->false_list->patch(mid);
        sl->push_back(lcx->stm);
        sl->push_back(new tree::LabelStm(mid));
        sl->push_back(rcx->stm);
        Patch_list *t = lcx->true_list;
        t->add(rcx->true_list);
        Patch_list *f = rcx->false_list;
        visit_exp_result = new Tr_cx(t, f, make_seq(sl));
        return;
    }

    node->left->accept(*this);
    tree::Exp *le = visit_exp_result->unEx(method_temp_map)->exp;
    node->right->accept(*this);
    tree::Exp *re = visit_exp_result->unEx(method_temp_map)->exp;

    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
    {
        Patch_list *t = new Patch_list();
        Patch_list *f = new Patch_list();
        tree::Label *tl = method_temp_map->newlabel();
        tree::Label *fl = method_temp_map->newlabel();
        t->add_patch(tl);
        f->add_patch(fl);
        visit_exp_result = new Tr_cx(t, f, new tree::Cjump(op, le, re, tl, fl));
        return;
    }

    tree::Type bt = semant_to_tree_type(semant_map->getSemant(node));
    visit_exp_result = new Tr_ex(new tree::Binop(bt, op, le, re));
}

void ASTToTreeVisitor::visit(fdmj::UnaryOp *node)
{
    if (node == nullptr || node->op == nullptr || node->exp == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }
    node->exp->accept(*this);
    tree::Exp *e = visit_exp_result->unEx(method_temp_map)->exp;
    if (node->op->op == "!")
    {
        visit_exp_result = new Tr_ex(new tree::Binop(tree::Type::INT, "xor", new tree::Const(1), e));
        return;
    }
    if (node->op->op == "-")
    {
        visit_exp_result = new Tr_ex(new tree::Binop(tree::Type::INT, "-", new tree::Const(0), e));
        return;
    }
    visit_exp_result = new Tr_ex(e);
}

void ASTToTreeVisitor::visit(fdmj::ArrayExp *node)
{
    if (node == nullptr || node->arr == nullptr || node->index == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }
    node->arr->accept(*this);
    tree::Exp *arr = visit_exp_result->unEx(method_temp_map)->exp;
    node->index->accept(*this);
    tree::Exp *idx = visit_exp_result->unEx(method_temp_map)->exp;
    tree::Exp *offset = new tree::Binop(tree::Type::INT, "*", idx, new tree::Const(4));
    tree::Exp *addr = new tree::Binop(tree::Type::PTR, "+", arr, offset);
    visit_exp_result = new Tr_ex(new tree::Mem(tree::Type::INT, addr));
}

void ASTToTreeVisitor::visit(fdmj::CallExp *node)
{
    if (node == nullptr || node->name == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }
    vector<tree::Exp *> *args = new vector<tree::Exp *>();
    if (node->par != nullptr)
    {
        for (auto *p : *node->par)
        {
            p->accept(*this);
            args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
        }
    }
    tree::Type rt = semant_to_tree_type(semant_map->getSemant(node));
    visit_exp_result = new Tr_ex(new tree::ExtCall(rt, node->name->id, args));
}

void ASTToTreeVisitor::visit(fdmj::ClassVar *node)
{
    (void)node;
    visit_exp_result = new Tr_ex(new tree::Const(0));
}

void ASTToTreeVisitor::visit(fdmj::This *node)
{
    (void)node;
    tree::Temp *t = method_var_table == nullptr ? nullptr : method_var_table->get_var_temp("this");
    if (t == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }
    visit_exp_result = new Tr_ex(new tree::TempExp(tree::Type::PTR, t));
}

void ASTToTreeVisitor::visit(fdmj::Length *node)
{
    if (node == nullptr || node->exp == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }
    node->exp->accept(*this);
    tree::Exp *arr = visit_exp_result->unEx(method_temp_map)->exp;
    tree::Exp *base = new tree::Binop(tree::Type::PTR, "-", arr, new tree::Const(4));
    visit_exp_result = new Tr_ex(new tree::Mem(tree::Type::INT, base));
}

void ASTToTreeVisitor::visit(fdmj::NewArray *node)
{
    if (node == nullptr || node->size == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }
    node->size->accept(*this);
    tree::Exp *n = visit_exp_result->unEx(method_temp_map)->exp;
    vector<tree::Exp *> *args = new vector<tree::Exp *>();
    args->push_back(n);
    visit_exp_result = new Tr_ex(new tree::ExtCall(tree::Type::PTR, "newarray", args));
}

void ASTToTreeVisitor::visit(fdmj::NewObject *node)
{
    (void)node;
    visit_exp_result = new Tr_ex(new tree::Const(0));
}

void ASTToTreeVisitor::visit(fdmj::GetInt *node)
{
    (void)node;
    visit_exp_result = new Tr_ex(new tree::ExtCall(tree::Type::INT, "getint", new vector<tree::Exp *>()));
}

void ASTToTreeVisitor::visit(fdmj::GetCh *node)
{
    (void)node;
    visit_exp_result = new Tr_ex(new tree::ExtCall(tree::Type::INT, "getch", new vector<tree::Exp *>()));
}

void ASTToTreeVisitor::visit(fdmj::GetArray *node)
{
    if (node == nullptr || node->exp == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }
    node->exp->accept(*this);
    vector<tree::Exp *> *args = new vector<tree::Exp *>();
    args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
    visit_exp_result = new Tr_ex(new tree::ExtCall(tree::Type::INT, "getarray", args));
}

void ASTToTreeVisitor::visit(fdmj::IdExp *node)
{
    if (node == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }
    tree::Temp *t = method_var_table == nullptr ? nullptr : method_var_table->get_var_temp(node->id);
    tree::Type ty = tree::Type::INT;
    if (method_var_table != nullptr && method_var_table->var_type_map->find(node->id) != method_var_table->var_type_map->end())
    {
        ty = method_var_table->get_var_type(node->id);
    }
    if (t == nullptr)
    {
        t = method_temp_map->newtemp();
        if (method_var_table != nullptr)
        {
            (*method_var_table->var_temp_map)[node->id] = t;
            (*method_var_table->var_type_map)[node->id] = ty;
        }
    }
    visit_exp_result = new Tr_ex(new tree::TempExp(ty, t));
}

void ASTToTreeVisitor::visit(fdmj::OpExp *node)
{
    (void)node;
    visit_exp_result = new Tr_ex(new tree::Const(0));
}

void ASTToTreeVisitor::visit(fdmj::IntExp *node)
{
    if (node == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }
    visit_exp_result = new Tr_ex(new tree::Const(node->val));
}
