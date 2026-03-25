#ifndef _AST2TREE_HH
#define _AST2TREE_HH

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "semant.hh"
#include "temp.hh"
#include "treep.hh"
#include "tr_exp.hh"
#include "config.hh" // IWYU pragma: keep

using namespace std;
//using namespace fdmj;
//using namespace tree;

//forward declaration
class Class_table;
class Method_var_table;
class Patch_list;
class ASTToTreeVisitor;

tree::Program* ast2tree(fdmj::Program* prog, AST_Semant_Map* semant_map);
Class_table* generate_class_table(AST_Semant_Map* semant_map);
Method_var_table* generate_method_var_table(string class_name, string method_name, Name_Maps* nm, Temp_map* tm);

//class table is to map each class var and method to an address offset
//this uses a "universal class" method, i.e., all the classes use the
//same class table, with all the possible vars and methods of all classes
//listed in the same record layout.
//to support variable hiding, we use class_name^var_name as the key
class Class_table {
public:
     map<string, int> var_pos_map;  // key: "class_name^var_name" for variable hiding support
     map<string, int> method_pos_map;
     Class_table() {
          var_pos_map = map<string, int>();
          method_pos_map = map<string, int>();
     };
     ~Class_table() {
          var_pos_map.clear();
          method_pos_map.clear();
     }
     // Get variable position for a specific class (supports variable hiding)
     // uses "class_name^var_name" as key
     int get_var_pos(string class_name, string var_name) {
          string key = class_name + "^" + var_name;
          if (var_pos_map.find(key) == var_pos_map.end()) {
               cerr << "Error: var " << key << " not found in class table!" << endl;
               return 0;
          }
          return var_pos_map[key];
     }
     // Check if a class has a variable
     bool has_var(string class_name, string var_name) {
          string key = class_name + "^" + var_name;
          return var_pos_map.find(key) != var_pos_map.end();
     }
     int get_method_pos(string method_name) {
          return method_pos_map[method_name];
     }
     void print_class_table() {
          cout << "======Class Table:" << endl;
          for (auto it = var_pos_map.begin(); it != var_pos_map.end(); ++it) {
               printf("var %s has pos %d\n", it->first.c_str(), it->second);
          }
          for (auto it = method_pos_map.begin(); it != method_pos_map.end(); ++it) {
               printf("method %s has pos %d\n", it->first.c_str(), it->second);
          }
          cout << "======End of Class Table" << endl;
     }
};

//For each method, there is a var table, including formal and local var.
//(if a method local has a conflict in var name with formal, then local var
//is used (ignore the formal))
//Hence, local var will override formal in the Method_var_table.
//Note: each local var and formal has a type as well (INT or PTR)
//The return of a method is also taken as a formal, with a special name _^return^_method_name.
class Method_var_table {
public:
     map<string, tree::Temp*> *var_temp_map;
     map<string, tree::Type> *var_type_map;
     Method_var_table() {
          var_temp_map = new map<string, tree::Temp*>();
          var_type_map = new map<string, tree::Type>();
     };
     ~Method_var_table() {
          var_temp_map->clear();
          var_type_map->clear();
          delete var_temp_map;
     }
     tree::Temp* get_var_temp(string var_name) {
          if (var_temp_map->find(var_name) == var_temp_map->end()) {
               return nullptr;
          }
          return var_temp_map->at(var_name);
     }
     tree::Type get_var_type(string var_name) {
          if (var_type_map->find(var_name) == var_type_map->end()) {
               cerr << "Error: var " << var_name << " not found in var_type_map!" << endl;
               return tree::Type::INT; //default type
          }
          return var_type_map->at(var_name);
     }
     void print_var_table() {
          //cout << "======Method Var Table:" << endl;
          for (auto it = var_temp_map->begin(); it!= var_temp_map->end(); ++it) {
               printf("var %s has temp %d, with type %s.\n", it->first.c_str(), it->second->num, 
                         (*var_type_map)[it->first]==tree::Type::INT?"INT":"PTR");
          }
          cout << "======End of Method Var Table" << endl;
     }    
};

class ASTToTreeVisitor : public fdmj::AST_Visitor {
public:
     //Below are two global variables set up at the Program Node
     AST_Semant_Map* semant_map; //semant map
     Class_table* class_table; //global class table
     //Below are two local variables set up at the Method Node
     Method_var_table* method_var_table; //var table used within a method (in a class)
     Temp_map* method_temp_map; //temp map used within a method (in a class)
     //Below are two vars for getting results back from the visit (of lower level nodes)
     tree::Tree* visit_tree_result; //this is to store the result of a visit with only one tree
     Tr_Exp *visit_exp_result;
     //Below are for knowing where the current node is (which class and method)
     string current_class;
     string current_method;
     Label *continue_label; //the continue label for the current while loop
     Label *break_label;  //the break label for the current while loop
     string class_var_class_name; //the class name of the object in ClassVar (for looking up var offset)

     ~ASTToTreeVisitor() {
          delete semant_map;
          //delete visit_tree_result; //can't delete this, as it is used by the caller
          //delete visit_exp_result; //may be part of the tree, so don't delete it here
          delete class_table;
          delete method_temp_map;
          delete method_var_table;
          current_class.clear();
          current_method.clear();
          delete continue_label;
          delete break_label;
     }

     ASTToTreeVisitor() {
          semant_map = nullptr;
          class_table = nullptr;
          visit_tree_result = nullptr;
          visit_exp_result = nullptr;
          method_temp_map = nullptr;
          method_var_table = nullptr;
          current_class.clear();
          current_method.clear();
          continue_label = nullptr;
          break_label = nullptr;
          class_var_class_name.clear();
     }

     tree::Tree* getTree() { return visit_tree_result; } //return the tree from a single visit (program returns a single tree)

     void visit(fdmj::Program* node) override;
     void visit(fdmj::MainMethod* node) override;
     void visit(fdmj::ClassDecl* node) override;
     void visit(fdmj::Type* node) override;
     void visit(fdmj::VarDecl* node) override;
     void visit(fdmj::MethodDecl* node) override;
     void visit(fdmj::Formal* node) override;
     void visit(fdmj::Nested* node) override;
     void visit(fdmj::If* node) override;
     void visit(fdmj::While* node) override;
     void visit(fdmj::Assign* node) override;
     void visit(fdmj::CallStm* node) override;
     void visit(fdmj::Continue* node) override;
     void visit(fdmj::Break* node) override;
     void visit(fdmj::Return* node) override;
     void visit(fdmj::PutInt* node) override;
     void visit(fdmj::PutCh* node) override;
     void visit(fdmj::PutArray* node) override;
     void visit(fdmj::Starttime* node) override;
     void visit(fdmj::Stoptime* node) override;
     void visit(fdmj::BinaryOp* node) override;
     void visit(fdmj::UnaryOp* node) override;
     void visit(fdmj::ArrayExp* node) override;
     void visit(fdmj::CallExp* node) override;
     void visit(fdmj::ClassVar* node) override;
     void visit(fdmj::This* node) override;
     void visit(fdmj::Length* node) override;
     void visit(fdmj::NewArray* node) override;
     void visit(fdmj::NewObject* node) override;
     void visit(fdmj::GetInt* node) override;
     void visit(fdmj::GetCh* node) override;
     void visit(fdmj::GetArray* node) override;
     void visit(fdmj::IdExp* node) override;
     void visit(fdmj::OpExp* node) override;
     void visit(fdmj::IntExp* node) override;
};

#endif