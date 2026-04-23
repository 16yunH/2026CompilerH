#define DEBUG
#undef DEBUG

#include <iostream>
#include <map>
#include <vector>
#include "semant.hh" // IWYU pragma: keep
#include "namemaps.hh"

using namespace std;
using namespace fdmj;

/*
Name_Maps* makeNameMaps(fdmj::Program* node) {
    AST_Name_Map_Visitor name_visitor;
    node->accept(name_visitor);
    return name_visitor.getNameMaps();
}*/

bool Name_Maps::is_class(string class_name) {
    return classes.find(class_name) != classes.end();
}

bool Name_Maps::add_class(string class_name) {
    if (Name_Maps::is_class(class_name)) {
        return false;
    }
    classes.insert(class_name);
    return true;
}

set<string>* Name_Maps::get_class_list() {
    return &classes;
}

bool Name_Maps::add_class_hiearchy(string class_name, string parent_name) {
    if (!Name_Maps::is_class(class_name) || !Name_Maps::is_class(parent_name)) {
        return false;
    }
    classHierachy[class_name] = parent_name;
    return true;
}

string Name_Maps::get_parent(string class_name) {
    if (classHierachy.find(class_name) == classHierachy.end()) {
        return "";
    }
    return classHierachy[class_name];
}

bool Name_Maps::is_method(string class_name, string method_name) {
    pair<string, string> p(class_name, method_name);
    return methods.find(p) != methods.end();
}

bool Name_Maps::add_method(string class_name, string method_name) {
    if (Name_Maps::is_method(class_name, method_name)) {
        return false;
    }
    methods.insert(pair<string, string>(class_name, method_name));
    return true;
}

set<string>* Name_Maps::get_method_list(string class_name) {
    set<string>* methods = new set<string>();
    for (auto it = this->methods.begin(); it != this->methods.end(); it++) {
        if (it->first == class_name) {
            methods->insert(it->second);
        }
    }
    return methods;
}

bool Name_Maps::is_class_var(string class_name, string var_name) {
    pair<string, string> p(class_name, var_name);
    return classVar.find(p) != classVar.end();
}

bool Name_Maps::add_class_var(string class_name, string var_name, VarDecl* varDecl) {
    pair<string, string> p(class_name, var_name);
    if (Name_Maps::is_class_var(class_name, var_name)) {
        return false;
    }
    classVar[p] = varDecl;
    return true;
}

set<string>* Name_Maps::get_class_var_list(string class_name) {
    set<string>* vars = new set<string>();
    for (auto it = classVar.begin(); it != classVar.end(); it++) {
        if (it->first.first == class_name) {
            vars->insert(it->first.second);
        }
    }
    return vars;
}

VarDecl* Name_Maps::get_class_var(string class_name, string var_name) {
    if (!Name_Maps::is_class_var(class_name, var_name)) {
        return nullptr;
    }
    pair<string, string> p(class_name, var_name);
    return classVar[p];
}

bool Name_Maps::is_method_var(string class_name, string method_name, string var_name) {
    tuple<string, string, string> t(class_name, method_name, var_name);
    return methodVar.find(t) != methodVar.end();
}

bool Name_Maps::add_method_var(string class_name, string method_name, string var_name, VarDecl* vd) {
    if (Name_Maps::is_method_var(class_name, method_name, var_name)) {
        return false;
    }
    methodVar[tuple<string, string, string>(class_name, method_name, var_name)] = vd;
    return true;
}

VarDecl* Name_Maps::get_method_var(string class_name, string method_name, string var_name) {
    if (!Name_Maps::is_method_var(class_name, method_name, var_name)) {
        return nullptr;
    }
    return methodVar[tuple<string, string, string>(class_name, method_name, var_name)];
}

bool Name_Maps::is_method_formal(string class_name, string method_name, string var_name) {
    tuple <string, string, string> t(class_name, method_name, var_name);
    return methodFormal.find(t) != methodFormal.end();
}

bool Name_Maps::add_method_formal(string class_name, string method_name, string var_name, Formal* f) {
    if (Name_Maps::is_method_formal(class_name, method_name, var_name)) {
        return false;
    }
    methodFormal[tuple<string, string, string>(class_name, method_name, var_name)] = f;
    return true;
}

Formal* Name_Maps::get_method_formal(string class_name, string method_name, string var_name) {
    if (!Name_Maps::is_method_formal(class_name, method_name, var_name)) {
        return nullptr;
    }
    return methodFormal[tuple<string, string, string>(class_name, method_name, var_name)];
}

bool Name_Maps::add_method_formal_list(string class_name, string method_name, vector<string> vl) {
    if (!Name_Maps::is_method(class_name, method_name)) {
        return false;
    }
    methodFormalList[pair<string, string>(class_name, method_name)] = vl;
    return true;
}

vector<Formal*>* Name_Maps::get_method_formal_list(string class_name, string method_name) {
    vector<Formal*>* fl = new vector<Formal*>();
    pair<string, string> p(class_name, method_name);
    if (methodFormalList.find(p) == methodFormalList.end()) {
        //Jun 14 2025: Here we need to see if the method is declared in an ancestor class
        string parent = get_parent(class_name);
        if (parent != "" && methodFormalList.find(pair<string, string>(parent, method_name)) != methodFormalList.end()) {
            auto & vl = methodFormalList[pair<string, string>(parent, method_name)];
            for (auto v : vl) {
                fl->push_back(Name_Maps::get_method_formal(parent, method_name, v));
            }
            return fl;
        }
        return fl;
    }
    vector<string> vl = methodFormalList[pair<string, string>(class_name, method_name)];
    for (auto v : vl) {
        if (Name_Maps::is_method_formal(class_name, method_name, v)) {
            fl->push_back(Name_Maps::get_method_formal(class_name, method_name, v));
        }
        else {
            cerr << "Error: Method Formal: " << class_name << "->" << method_name << "->" << v << " not found" << endl;
            return nullptr;
        }
    }
    return fl;
}

vector<string>* Name_Maps::get_method_formal_list_string(string class_name, string method_name) {
    vector<string>* var_list = new vector<string>();
    pair<string, string> p(class_name, method_name);
    if (methodFormalList.find(p) == methodFormalList.end()) {
        //Jun 14 2025: Here we need to see if the method is declared in an ancestor class
        string parent = get_parent(class_name);
        if (parent != "" && methodFormalList.find(pair<string, string>(parent, method_name)) != methodFormalList.end()) {
            auto & vl = methodFormalList[pair<string, string>(parent, method_name)];
            for (auto v : vl) {
                var_list->push_back(v);
            }
            return var_list;
        }
        return var_list;
    }
    vector<string> vl = methodFormalList[pair<string, string>(class_name, method_name)];
    for (auto v : vl) {
        var_list->push_back(v);
    }
    return var_list;
}

set<string>* Name_Maps::get_method_var_list(string class_name, string method_name) {
    set<string>* vars = new set<string>();
    for (auto it = methodVar.begin(); it != methodVar.end(); it++) {
        if (get<0>(it->first) == class_name && get<1>(it->first) == method_name) {
            vars->insert(get<2>(it->first));
        }
    }
    return vars;
}

void Name_Maps::print() {
    cout << "Classes: ";
    for (auto c : classes) {
        cout << c << " ; ";
    }
    cout << endl;
    cout << "Class Hiearchy: ";
    for (auto it = classHierachy.begin(); it != classHierachy.end(); it++) {
        cout << it->first << "->" << it->second << " ; ";
    }
    cout << endl;
    cout << "Methods: ";
    for (auto m : methods) {
        cout << m.first << "->" << m.second << " ; ";
    }
    cout << endl;
    cout << "Class Variables: ";
    for (auto it = classVar.begin(); it != classVar.end(); it++) {
        VarDecl* vd = it->second;
        cout << (it->first).first << "->" << (it->first).second << " with type=" << 
            type_kind_string(vd->type->typeKind) << " ; ";
    }
    cout << endl;
    cout << "Method Variables: ";
    for (auto it = methodVar.begin(); it != methodVar.end(); it++) {
        VarDecl* vd = it->second;
        cout << get<0>(it->first) << "->" << get<1>(it->first) << "->" << get<2>(it->first) << 
            " with type=" << type_kind_string(vd->type->typeKind) <<  " ; ";
    }
    cout << endl;
    cout << "Method Formals: ";
    for (auto it = methodFormal.begin(); it != methodFormal.end(); it++) {
        fdmj::Type *t = it->second->type;
        cout << get<0>(it->first) << "->" << get<1>(it->first) << "->" << get<2>(it->first) <<
            " with type=" << type_kind_string(t->typeKind) << " ; ";
    }
    cout << endl;
}