/*
 * SymbolTable.h
 *
 *  Created on: Apr 27, 2015
 *      Author: baki
 */

#ifndef SOLVER_SYMBOLTABLE_H_
#define SOLVER_SYMBOLTABLE_H_

#include <algorithm>
#include <bits/functional_hash.h>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <glog/logging.h>

#include "../smt/ast.h"
#include "../smt/typedefs.h"
#include "../theory/IntAutomaton.h"
#include "../theory/StringAutomaton.h"
#include "Ast2Dot.h"
#include "EquivalenceClass.h"
#include "options/Solver.h"
#include "Value.h"

namespace Vlab {
namespace Solver {

using VariableMap = std::map<std::string, SMT::Variable_ptr>;
using VariableCounterMap = std::map<SMT::Variable_ptr, int>;
using VariableCounterTable = std::map<SMT::Visitable_ptr, VariableCounterMap>;
using EquivClassMap = std::map<SMT::Variable_ptr, EquivalenceClass_ptr>;
using EquivClassTable = std::map<SMT::Visitable_ptr, EquivClassMap>;
using GroupMap = std::map<SMT::Variable_ptr, SMT::Variable_ptr>;
using VariableValueMap = std::map<SMT::Variable_ptr, Value_ptr>;
using VariableValueTable = std::map<SMT::Visitable_ptr, VariableValueMap>;



class SymbolTable {
public:
  SymbolTable();
  virtual ~SymbolTable();

  bool isSatisfiable();
  void update_satisfiability_result(bool value);
  void clearLetScopes();

  void add_variable(SMT::Variable_ptr);
  SMT::Variable_ptr get_variable(std::string name);
  SMT::Variable_ptr get_variable(SMT::Term_ptr);
  SMT::Variable_ptr get_variable_unsafe(std::string name);
  VariableMap& get_variables();
  int get_num_of_variables(SMT::Variable::Type type);

  void push_scope(SMT::Visitable_ptr, bool save_scope = true);
  SMT::Visitable_ptr top_scope();
  void pop_scope();


  /*
   * Variable count functions, used for reduction and optimization
   */
  void increment_count(SMT::Variable_ptr);
  void decrement_count(SMT::Variable_ptr);
  int get_local_count(SMT::Variable_ptr);
  int get_total_count(SMT::Variable_ptr);
  void reset_count();

  /*Added function to keep track of the amount of reuse 
  int getReuse();
  void incrementReuse();
  */

  int get_num_of_substituted_variables(SMT::Visitable_ptr scope, SMT::Variable::Type type);
  EquivClassTable& get_equivalance_class_table();
  EquivalenceClass_ptr get_equivalence_class_of(SMT::Variable_ptr);
  EquivalenceClass_ptr get_equivalence_class_of_at_scope(SMT::Visitable_ptr scope, SMT::Variable_ptr);
  void add_variable_equiv_class_mapping(SMT::Variable_ptr, EquivalenceClass_ptr);
  SMT::Variable_ptr get_representative_variable_of_at_scope(SMT::Visitable_ptr scope, SMT::Variable_ptr);

  void add_variable_group_mapping(std::string variable_name, std::string group_name);
  void add_variable_group_mapping(SMT::Variable_ptr, SMT::Variable_ptr);
  SMT::Variable_ptr get_group_variable_of(SMT::Variable_ptr);

  Value_ptr get_value(std::string var_name);
  Value_ptr get_value(SMT::Variable_ptr variable);
  Value_ptr get_value_at_scope(SMT::Visitable_ptr scope, SMT::Variable_ptr variable);
  Value_ptr get_projected_value_at_scope(SMT::Visitable_ptr scope, SMT::Variable_ptr variable);
  VariableValueMap& get_values_at_scope(SMT::Visitable_ptr scope);
  bool set_value(std::string var_name, Value_ptr value);
  bool set_value(SMT::Variable_ptr variable, Value_ptr value);
  bool IntersectValue(std::string var_name, Value_ptr value);
  bool IntersectValue(SMT::Variable_ptr variable, Value_ptr value);
  bool UnionValue(std::string var_name, Value_ptr value);
  bool UnionValue(SMT::Variable_ptr variable, Value_ptr value);

  bool clear_value(std::string var_name, SMT::Visitable_ptr scope);
  bool clear_value(SMT::Variable_ptr variable, SMT::Visitable_ptr scope);

  std::string get_var_name_for_expression(SMT::Visitable_ptr, SMT::Variable::Type);
  std::string get_var_name_for_node(SMT::Visitable_ptr, SMT::Variable::Type);

private:
  std::string generate_internal_name(std::string, SMT::Variable::Type);

  bool global_assertion_result_;
  /**
   * Name to variable map
   */
  VariableMap variables_;

  /**
   * There is a global scope
   * A new scope is generated when there is a disjuction
   */
  std::vector<SMT::Visitable_ptr> scope_stack_;
  std::set<SMT::Visitable_ptr> scopes_;

  /**
   * Number of usages of variables
   */
  VariableCounterTable variable_counts_table_;

  /**
   * Has a mapping from a variable to its corresponding equivalence class if any exists
   */
  EquivClassTable variable_equivalence_table_;

  /**
   * Has a mapping from a variable to its group variable if any; case occurs with multitrack auto
   */
  GroupMap variable_group_map_;

  /**
   * Projected values of variables that appear in multitrack automata
   * Stored only when necessary
   */
  VariableValueTable variable_projected_value_table_;

  /**
   * Values of each variable for each scope
   */
  VariableValueTable variable_value_table_;

  static const int VLOG_LEVEL;
  //int reuse; 

};

using SymbolTable_ptr = SymbolTable*;

} /* namespace Solver */
} /* namespace Vlab */

#endif /* SOLVER_SYMBOLTABLE_H_ */
