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

static bool is_return_pseudo_name(const string &name)
{
    return name.rfind("_^return^_", 0) == 0;
}

static string find_declaring_class_for_var(Name_Maps *nm, const string &start_class, const string &var_name)
{
    string c = start_class;
    while (!c.empty())
    {
        if (nm->is_class_var(c, var_name))
        {
            return c;
        }
        c = nm->get_parent(c);
    }
    return "";
}

static string find_impl_class_for_method(Name_Maps *nm, const string &start_class, const string &method_name)
{
    string c = start_class;
    while (!c.empty())
    {
        if (nm->is_method(c, method_name))
        {
            return c;
        }
        c = nm->get_parent(c);
    }
    return "";
}

static int bytes_per_slot()
{
    return 4;
}

static int class_var_slot_count(Class_table *ct)
{
    return static_cast<int>(ct->var_pos_map.size());
}

static int method_offset_bytes(Class_table *ct, const string &method_name)
{
    return (class_var_slot_count(ct) + ct->get_method_pos(method_name)) * bytes_per_slot();
}

static tree::Exp *build_checked_array_index(Temp_map *tm, tree::Exp *arr_ptr, tree::Exp *idx_exp)
{
    auto is_simple_exp = [](tree::Exp *e) -> bool
    {
        return dynamic_cast<tree::TempExp *>(e) != nullptr ||
               dynamic_cast<tree::Const *>(e) != nullptr ||
               dynamic_cast<tree::Name *>(e) != nullptr;
    };

    tree::Exp *arr_use = arr_ptr;
    tree::Exp *idx_use = idx_exp;
    tree::Temp *arr_t = nullptr;
    tree::Temp *idx_t = nullptr;
    vector<tree::Stm *> *pre_sl = new vector<tree::Stm *>();

    if (!is_simple_exp(arr_ptr))
    {
        arr_t = tm->newtemp();
        pre_sl->push_back(new tree::Move(new tree::TempExp(tree::Type::PTR, arr_t), arr_ptr));
        arr_use = new tree::TempExp(tree::Type::PTR, arr_t);
    }
    if (!is_simple_exp(idx_exp))
    {
        idx_t = tm->newtemp();
        pre_sl->push_back(new tree::Move(new tree::TempExp(tree::Type::INT, idx_t), idx_exp));
        idx_use = new tree::TempExp(tree::Type::INT, idx_t);
    }

    tree::Temp *len_t = tm->newtemp();
    tree::Label *l_oob = tm->newlabel();
    tree::Label *l_inrange_left = tm->newlabel();
    tree::Label *l_ok = tm->newlabel();

    vector<tree::Stm *> *sl = new vector<tree::Stm *>();

    sl->push_back(new tree::Move(
        new tree::TempExp(tree::Type::INT, len_t),
        new tree::Mem(tree::Type::INT, arr_use)));
    sl->push_back(new tree::Cjump(">=",
                                  idx_use,
                                  new tree::Const(0),
                                  l_inrange_left,
                                  l_oob));
    sl->push_back(new tree::LabelStm(l_inrange_left));
    sl->push_back(new tree::Cjump(">=",
                                  idx_use,
                                  new tree::TempExp(tree::Type::INT, len_t),
                                  l_oob,
                                  l_ok));
    sl->push_back(new tree::LabelStm(l_oob));
    vector<tree::Exp *> *exit_args = new vector<tree::Exp *>();
    exit_args->push_back(new tree::Const(-1));
    sl->push_back(new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "exit", exit_args)));
    sl->push_back(new tree::LabelStm(l_ok));

    tree::Exp *checked_index = new tree::Eseq(tree::Type::INT, make_seq(sl), idx_use);
    tree::Exp *elem_index = new tree::Binop(tree::Type::INT, "+", checked_index, new tree::Const(1));
    tree::Exp *offset = new tree::Binop(tree::Type::INT, "*", elem_index, new tree::Const(bytes_per_slot()));
    tree::Exp *addr = new tree::Binop(tree::Type::PTR,
                                      "+",
                                      arr_use,
                                      offset);
    if (pre_sl->empty())
    {
        return addr;
    }
    return new tree::Eseq(tree::Type::PTR, make_seq(pre_sl), addr);
}

static void append_stmt_flatten(vector<tree::Stm *> *body, tree::Stm *stm)
{
    if (body == nullptr || stm == nullptr)
    {
        return;
    }

    tree::Seq *seq = dynamic_cast<tree::Seq *>(stm);
    if (seq != nullptr && seq->sl != nullptr)
    {
        for (auto *s : *seq->sl)
        {
            if (s != nullptr)
            {
                body->push_back(s);
            }
        }
        return;
    }

    body->push_back(stm);
}

static const string kInitShadowPrefix = "_^init_shadow^_";

static string shadow_name_for(const string &var_name)
{
    return kInitShadowPrefix + var_name;
}

static bool is_local_int_var(ASTToTreeVisitor *vis, const string &var_name)
{
    if (vis == nullptr || vis->semant_map == nullptr || vis->semant_map->getNameMaps() == nullptr)
    {
        return false;
    }
    VarDecl *vd = vis->semant_map->getNameMaps()->get_method_var(
        vis->current_class,
        vis->current_method,
        var_name);
    return vd != nullptr && vd->type != nullptr && vd->type->typeKind == fdmj::TypeKind::INT;
}

static tree::Temp *get_shadow_temp(ASTToTreeVisitor *vis, const string &var_name)
{
    if (vis == nullptr || vis->method_var_table == nullptr)
    {
        return nullptr;
    }
    string key = shadow_name_for(var_name);
    auto it = vis->method_var_table->var_temp_map->find(key);
    if (it == vis->method_var_table->var_temp_map->end())
    {
        return nullptr;
    }
    return it->second;
}

static tree::Temp *ensure_shadow_temp(ASTToTreeVisitor *vis, const string &var_name)
{
    if (!is_local_int_var(vis, var_name))
    {
        return nullptr;
    }

    tree::Temp *existing = get_shadow_temp(vis, var_name);
    if (existing != nullptr)
    {
        return existing;
    }

    if (vis == nullptr || vis->method_temp_map == nullptr || vis->method_var_table == nullptr)
    {
        return nullptr;
    }

    tree::Temp *shadow = vis->method_temp_map->newtemp();
    string key = shadow_name_for(var_name);
    (*vis->method_var_table->var_temp_map)[key] = shadow;
    (*vis->method_var_table->var_type_map)[key] = tree::Type::INT;
    return shadow;
}

static void append_shadow_inits_for_local_ints(
    ASTToTreeVisitor *vis,
    vector<fdmj::VarDecl *> *vdl,
    vector<tree::Stm *> *body)
{
    if (vis == nullptr || vdl == nullptr || body == nullptr)
    {
        return;
    }

    for (auto *vd : *vdl)
    {
        if (vd == nullptr || vd->id == nullptr || vd->type == nullptr)
        {
            continue;
        }
        if (vd->type->typeKind != fdmj::TypeKind::INT)
        {
            continue;
        }

        tree::Temp *shadow = ensure_shadow_temp(vis, vd->id->id);
        if (shadow != nullptr)
        {
            body->push_back(new tree::Move(
                new tree::TempExp(tree::Type::INT, shadow),
                new tree::Const(0)));
        }
    }
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
        if (c == "__$main__")
        {
            continue;
        }
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

    if (class_name != "__$main__")
    {
        tree::Temp *this_t = tm->newtemp();
        (*mvt->var_temp_map)["this"] = this_t;
        (*mvt->var_type_map)["this"] = tree::Type::PTR;
    }

    vector<string> *formals = nm->get_method_formal_list_string(class_name, method_name);
    if (formals != nullptr)
    {
        for (const auto &fname : *formals)
        {
            if (is_return_pseudo_name(fname))
            {
                continue;
            }
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

    tree::Type ret_ty = tree::Type::INT;
    Formal *ret_formal = nm->get_method_formal(class_name, method_name, "_^return^_" + method_name);
    if (ret_formal != nullptr && ret_formal->type != nullptr)
    {
        ret_ty = type_to_tree_type(ret_formal->type);
    }
    tree::Temp *ret_t = tm->newtemp();
    (*mvt->var_temp_map)["_^return^_" + method_name] = ret_t;
    (*mvt->var_type_map)["_^return^_" + method_name] = ret_ty;

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

    if (node->cdl != nullptr)
    {
        for (auto *cd : *node->cdl)
        {
            if (cd == nullptr || cd->id == nullptr || cd->mdl == nullptr)
            {
                continue;
            }
            current_class = cd->id->id;
            for (auto *md : *cd->mdl)
            {
                if (md == nullptr)
                {
                    continue;
                }
                md->accept(*this);
                if (visit_tree_result != nullptr)
                {
                    funcs->push_back(static_cast<tree::FuncDecl *>(visit_tree_result));
                }
            }
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

    append_shadow_inits_for_local_ints(this, node == nullptr ? nullptr : node->vdl, body);

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
                append_stmt_flatten(body, static_cast<tree::Stm *>(visit_tree_result));
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

    tree::Type vt = tree::Type::INT;
    if (method_var_table != nullptr && method_var_table->var_type_map->find(node->id->id) != method_var_table->var_type_map->end())
    {
        vt = method_var_table->get_var_type(node->id->id);
    }

    if (holds_alternative<fdmj::IntExp *>(node->init))
    {
        auto *ie = get<fdmj::IntExp *>(node->init);
        if (ie != nullptr)
        {
            tree::Stm *init_stm = new tree::Move(new tree::TempExp(vt, t), new tree::Const(ie->val));

            // declaration with int initializer counts as "initialized"
            tree::Temp *shadow = ensure_shadow_temp(this, node->id->id);
            if (shadow != nullptr)
            {
                vector<tree::Stm *> *sl = new vector<tree::Stm *>();
                sl->push_back(init_stm);
                sl->push_back(new tree::Move(new tree::TempExp(tree::Type::INT, shadow), new tree::Const(1)));
                visit_tree_result = make_seq(sl);
                return;
            }

            visit_tree_result = init_stm;
            return;
        }
    }

    if (holds_alternative<vector<fdmj::IntExp *> *>(node->init) && node->type != nullptr && node->type->typeKind == fdmj::TypeKind::ARRAY)
    {
        vector<fdmj::IntExp *> *vals = get<vector<fdmj::IntExp *> *>(node->init);
        if (vals == nullptr)
        {
            visit_tree_result = nullptr;
            return;
        }
        int n = vals == nullptr ? 0 : static_cast<int>(vals->size());

        vector<tree::Stm *> *sl = new vector<tree::Stm *>();
        vector<tree::Exp *> *malloc_args = new vector<tree::Exp *>();
        malloc_args->push_back(new tree::Const((n + 1) * bytes_per_slot()));
        sl->push_back(new tree::Move(
            new tree::TempExp(tree::Type::PTR, t),
            new tree::ExtCall(tree::Type::PTR, "malloc", malloc_args)));
        sl->push_back(new tree::Move(
            new tree::Mem(tree::Type::INT, new tree::TempExp(tree::Type::PTR, t)),
            new tree::Const(n)));

        if (vals != nullptr)
        {
            for (int i = 0; i < n; ++i)
            {
                int v = (vals->at(i) == nullptr) ? 0 : vals->at(i)->val;
                tree::Exp *addr = new tree::Binop(tree::Type::PTR,
                                                  "+",
                                                  new tree::TempExp(tree::Type::PTR, t),
                                                  new tree::Const((i + 1) * bytes_per_slot()));
                sl->push_back(new tree::Move(new tree::Mem(tree::Type::INT, addr), new tree::Const(v)));
            }
        }

        visit_tree_result = make_seq(sl);
        return;
    }

    if (holds_alternative<monostate>(node->init) && vt == tree::Type::PTR)
    {
        visit_tree_result = new tree::Move(new tree::TempExp(tree::Type::PTR, t), new tree::Const(0));
        return;
    }

    visit_tree_result = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::MethodDecl *node)
{
    if (node == nullptr || node->id == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }

    current_method = node->id->id;
    method_temp_map = new Temp_map();
    method_var_table = generate_method_var_table(current_class, current_method, semant_map->getNameMaps(), method_temp_map);

    vector<tree::Stm *> *body = new vector<tree::Stm *>();

    append_shadow_inits_for_local_ints(this, node->vdl, body);

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
                append_stmt_flatten(body, static_cast<tree::Stm *>(visit_tree_result));
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

    vector<tree::Temp *> *args = new vector<tree::Temp *>();
    tree::Temp *this_t = method_var_table->get_var_temp("this");
    if (this_t != nullptr)
    {
        args->push_back(this_t);
    }
    vector<string> *formals = semant_map->getNameMaps()->get_method_formal_list_string(current_class, current_method);
    if (formals != nullptr)
    {
        for (const auto &fname : *formals)
        {
            if (is_return_pseudo_name(fname))
            {
                continue;
            }
            tree::Temp *ft = method_var_table->get_var_temp(fname);
            if (ft != nullptr)
            {
                args->push_back(ft);
            }
        }
        delete formals;
    }

    tree::Stm *body_stm = body->empty() ? nullptr : static_cast<tree::Stm *>(new tree::Seq(body));
    visit_tree_result = new tree::FuncDecl(
        current_class + "^" + current_method,
        args,
        body_stm,
        type_to_tree_type(node->type),
        method_temp_map->next_temp - 1,
        method_temp_map->next_label - 1);
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

    fdmj::IdExp *lhs_id = dynamic_cast<fdmj::IdExp *>(node->left);
    if (lhs_id != nullptr)
    {
        tree::Type dst_ty = tree::Type::INT;
        if (method_var_table != nullptr &&
            method_var_table->var_type_map->find(lhs_id->id) != method_var_table->var_type_map->end())
        {
            dst_ty = method_var_table->get_var_type(lhs_id->id);
        }

        tree::Temp *dst_t = method_var_table == nullptr ? nullptr : method_var_table->get_var_temp(lhs_id->id);
        if (dst_t == nullptr)
        {
            dst_t = method_temp_map->newtemp();
            if (method_var_table != nullptr)
            {
                (*method_var_table->var_temp_map)[lhs_id->id] = dst_t;
                (*method_var_table->var_type_map)[lhs_id->id] = dst_ty;
            }
        }

        node->exp->accept(*this);
        tree::Exp *src = visit_exp_result->unEx(method_temp_map)->exp;

        vector<tree::Stm *> *sl = new vector<tree::Stm *>();
        sl->push_back(new tree::Move(new tree::TempExp(dst_ty, dst_t), src));

        tree::Temp *shadow = ensure_shadow_temp(this, lhs_id->id);
        if (shadow != nullptr)
        {
            sl->push_back(new tree::Move(new tree::TempExp(tree::Type::INT, shadow), new tree::Const(1)));
        }

        visit_tree_result = (sl->size() == 1)
                                ? static_cast<tree::Stm *>(sl->at(0))
                                : static_cast<tree::Stm *>(new tree::Seq(sl));
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
    if (node->obj == nullptr || node->name == nullptr)
    {
        visit_tree_result = nullptr;
        return;
    }

    node->obj->accept(*this);
    tree::Exp *obj = visit_exp_result->unEx(method_temp_map)->exp;
    vector<tree::Exp *> *args = new vector<tree::Exp *>();
    args->push_back(obj);

    if (node->par != nullptr)
    {
        for (auto *p : *node->par)
        {
            p->accept(*this);
            args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
        }
    }

    int moff = method_offset_bytes(class_table, node->name->id);
    tree::Exp *maddr = new tree::Binop(tree::Type::PTR, "+", obj, new tree::Const(moff));
    tree::Exp *mfun = new tree::Mem(tree::Type::PTR, maddr);
    tree::Type rt = semant_to_tree_type(semant_map->getSemant(node));
    visit_tree_result = new tree::ExpStm(new tree::Call(rt, node->name->id, mfun, args));
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
    tree::Exp *addr = build_checked_array_index(method_temp_map, arr, idx);
    tree::Eseq *addr_eseq = dynamic_cast<tree::Eseq *>(addr);
    if (addr_eseq != nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Eseq(
            tree::Type::INT,
            addr_eseq->stm,
            new tree::Mem(tree::Type::INT, addr_eseq->exp)));
        return;
    }
    visit_exp_result = new Tr_ex(new tree::Mem(tree::Type::INT, addr));
}

void ASTToTreeVisitor::visit(fdmj::CallExp *node)
{
    if (node == nullptr || node->name == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }

    if (node->obj == nullptr)
    {
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
        return;
    }

    node->obj->accept(*this);
    tree::Exp *obj = visit_exp_result->unEx(method_temp_map)->exp;
    vector<tree::Exp *> *args = new vector<tree::Exp *>();
    args->push_back(obj);
    if (node->par != nullptr)
    {
        for (auto *p : *node->par)
        {
            p->accept(*this);
            args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
        }
    }

    int moff = method_offset_bytes(class_table, node->name->id);
    tree::Exp *maddr = new tree::Binop(tree::Type::PTR, "+", obj, new tree::Const(moff));
    tree::Exp *mfun = new tree::Mem(tree::Type::PTR, maddr);
    tree::Type rt = semant_to_tree_type(semant_map->getSemant(node));
    visit_exp_result = new Tr_ex(new tree::Call(rt, node->name->id, mfun, args));
}

void ASTToTreeVisitor::visit(fdmj::ClassVar *node)
{
    if (node == nullptr || node->obj == nullptr || node->id == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }

    node->obj->accept(*this);
    tree::Exp *obj = visit_exp_result->unEx(method_temp_map)->exp;

    string class_name;
    AST_Semant *obj_s = semant_map->getSemant(node->obj);
    if (obj_s != nullptr && obj_s->get_type() == fdmj::TypeKind::CLASS)
    {
        auto tp = obj_s->get_type_par();
        if (holds_alternative<string>(tp))
        {
            class_name = get<string>(tp);
        }
    }
    if (class_name.empty())
    {
        class_name = current_class;
    }

    Name_Maps *nm = semant_map->getNameMaps();
    string owner_class = find_declaring_class_for_var(nm, class_name, node->id->id);
    if (owner_class.empty())
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }

    int voff = class_table->get_var_pos(owner_class, node->id->id) * bytes_per_slot();
    tree::Type ft = semant_to_tree_type(semant_map->getSemant(node));
    tree::Exp *addr = new tree::Binop(tree::Type::PTR, "+", obj, new tree::Const(voff));
    visit_exp_result = new Tr_ex(new tree::Mem(ft, addr));
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
    tree::Temp *t = method_temp_map->newtemp();
    visit_exp_result = new Tr_ex(new tree::Eseq(
        tree::Type::INT,
        new tree::Move(new tree::TempExp(tree::Type::INT, t), new tree::Mem(tree::Type::INT, arr)),
        new tree::TempExp(tree::Type::INT, t)));
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

    tree::Temp *arr_t = method_temp_map->newtemp();
    vector<tree::Stm *> *sl = new vector<tree::Stm *>();
    vector<tree::Exp *> *malloc_args = new vector<tree::Exp *>();
    tree::Exp *alloc_size = new tree::Binop(
        tree::Type::INT,
        "*",
        new tree::Binop(tree::Type::INT, "+", n, new tree::Const(1)),
        new tree::Const(bytes_per_slot()));
    malloc_args->push_back(alloc_size);
    sl->push_back(new tree::Move(
        new tree::TempExp(tree::Type::PTR, arr_t),
        new tree::ExtCall(tree::Type::PTR, "malloc", malloc_args)));
    sl->push_back(new tree::Move(
        new tree::Mem(tree::Type::INT, new tree::TempExp(tree::Type::PTR, arr_t)),
        n));
    visit_exp_result = new Tr_ex(new tree::Eseq(tree::Type::PTR, make_seq(sl), new tree::TempExp(tree::Type::PTR, arr_t)));
}

void ASTToTreeVisitor::visit(fdmj::NewObject *node)
{
    if (node == nullptr || node->id == nullptr)
    {
        visit_exp_result = new Tr_ex(new tree::Const(0));
        return;
    }

    string class_name = node->id->id;
    int total_slots = class_var_slot_count(class_table) + static_cast<int>(class_table->method_pos_map.size());
    int bytes = total_slots * bytes_per_slot();

    tree::Temp *obj_t = method_temp_map->newtemp();
    vector<tree::Stm *> *sl = new vector<tree::Stm *>();
    vector<tree::Exp *> *malloc_args = new vector<tree::Exp *>();
    malloc_args->push_back(new tree::Const(bytes));
    sl->push_back(new tree::Move(
        new tree::TempExp(tree::Type::PTR, obj_t),
        new tree::ExtCall(tree::Type::PTR, "malloc", malloc_args)));

    Name_Maps *nm = semant_map->getNameMaps();
    for (const auto &entry : class_table->method_pos_map)
    {
        const string &mname = entry.first;
        string impl = find_impl_class_for_method(nm, class_name, mname);
        if (impl.empty())
        {
            continue;
        }
        int moff = method_offset_bytes(class_table, mname);
        tree::Exp *slot = new tree::Binop(
            tree::Type::PTR,
            "+",
            new tree::TempExp(tree::Type::PTR, obj_t),
            new tree::Const(moff));
        sl->push_back(new tree::Move(
            new tree::Mem(tree::Type::PTR, slot),
            new tree::Name(method_temp_map->newstringlabel(impl + "^" + mname))));
    }

    visit_exp_result = new Tr_ex(new tree::Eseq(tree::Type::PTR, make_seq(sl), new tree::TempExp(tree::Type::PTR, obj_t)));
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

    tree::Temp *shadow = ensure_shadow_temp(this, node->id);
    if (shadow != nullptr)
    {
        tree::Label *l_ok = method_temp_map->newlabel();
        tree::Label *l_bad = method_temp_map->newlabel();

        vector<tree::Stm *> *sl = new vector<tree::Stm *>();
        sl->push_back(new tree::Cjump(
            "==",
            new tree::TempExp(tree::Type::INT, shadow),
            new tree::Const(1),
            l_ok,
            l_bad));
        sl->push_back(new tree::LabelStm(l_bad));

        vector<tree::Exp *> *exit_args = new vector<tree::Exp *>();
        exit_args->push_back(new tree::Const(-101));
        sl->push_back(new tree::ExpStm(new tree::ExtCall(tree::Type::INT, "exit", exit_args)));

        sl->push_back(new tree::LabelStm(l_ok));

        visit_exp_result = new Tr_ex(
            new tree::Eseq(
                ty,
                make_seq(sl),
                new tree::TempExp(ty, t)));
        return;
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
