/*
 * ConstraintSorter.cpp
 *
 *  Created on: May 18, 2015
 *      Author: baki
 */

#include "ConstraintSorter.h"

namespace Vlab {
namespace Solver {

using namespace SMT;

const int ConstraintSorter::VLOG_LEVEL = 13;

ConstraintSorter::ConstraintSorter(Script_ptr script, SymbolTable_ptr symbol_table)
        : root_(script), symbol_table_(symbol_table), term_node_(nullptr) {
}

ConstraintSorter::~ConstraintSorter() {
  for(auto it = variable_nodes_.cbegin(); it != variable_nodes_.cend(); it++) {
    delete variable_nodes_[it->first];
    variable_nodes_[it->first] = nullptr;
  }
  if(term_node_ != nullptr) {
    delete term_node_;
    term_node_ = nullptr;
  }
}

void ConstraintSorter::start() {
  Counter counter(root_, symbol_table_);
  counter.start();

  symbol_table_->push_scope(root_);
  visit(root_);
  symbol_table_->pop_scope();

  end();
}

void ConstraintSorter::end() {
#ifndef NDEBUG
//  if (VLOG_IS_ON(VLOG_LEVEL)) {
//    DVLOG(VLOG_LEVEL) << "global dependency info: " << root;
//    for (auto& node : dependency_node_list) {
//      DVLOG(VLOG_LEVEL) << node->str();
//    }
//
//    for (auto& node : variable_nodes) {
//      DVLOG(VLOG_LEVEL) << node.second->str();
//    }
//  }
#endif
}

void ConstraintSorter::visitScript(Script_ptr script) {
  visit_children_of(script);
}

void ConstraintSorter::visitCommand(Command_ptr command) {
}

void ConstraintSorter::visitAssert(Assert_ptr assert_command) {
  visit_children_of(assert_command);
}

void ConstraintSorter::visitTerm(Term_ptr term) {
}

void ConstraintSorter::visitExclamation(Exclamation_ptr exclamation_term) {
}

void ConstraintSorter::visitExists(Exists_ptr exists_term) {
}

void ConstraintSorter::visitForAll(ForAll_ptr for_all_term) {
}

void ConstraintSorter::visitLet(Let_ptr let_term) {
  TermNode_ptr binding_node = nullptr;
  for (auto& term : *(let_term->var_binding_list)) {
    term_node_ = nullptr;
    visit(term);
    if (binding_node == nullptr and term_node_ != nullptr) {
      binding_node = term_node_;
      binding_node->shiftToRight();
    } else if (term_node_ != nullptr) {
      binding_node->addVariableNodes(term_node_->getAllNodes(), false);
    }
  }
  term_node_ = nullptr;
  visit(let_term->term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(binding_node, right_node);
}

void ConstraintSorter::visitAnd(And_ptr and_term) {
  std::vector<TermNode_ptr> local_dependency_node_list;
  for (auto& term : *(and_term->term_list)) {
    term_node_ = nullptr;
    visit(term);
    if (term_node_ == nullptr) {
      term_node_ = new TermNode(term);
    } else {
      term_node_->setNode(term);
    }
    term_node_->addMeToChildVariableNodes();
    local_dependency_node_list.push_back(term_node_);
  }
  term_node_ = nullptr;

  sort_terms(local_dependency_node_list);
#ifndef NDEBUG
  if (VLOG_IS_ON(VLOG_LEVEL)) {
    for (auto& node : local_dependency_node_list) {
      DVLOG(VLOG_LEVEL) << node->str();
    }

    for (auto& node : variable_nodes_) {
      DVLOG(VLOG_LEVEL) << node.second->str();
    }
  }
#endif

  and_term->term_list->clear();
  for(auto it = local_dependency_node_list.cbegin(); it != local_dependency_node_list.cend(); it++) {
    and_term->term_list->push_back((*it)->getNode());
    delete *it;
  }
}

void ConstraintSorter::visitOr(Or_ptr or_term) {
  for (auto& term : *(or_term->term_list)) {
    symbol_table_->push_scope(term);
    visit(term);
    symbol_table_->pop_scope();
  }
}

void ConstraintSorter::visitNot(Not_ptr not_term) {
  term_node_ = nullptr;
  visit_children_of(not_term);
}

void ConstraintSorter::visitUMinus(UMinus_ptr u_minus_term) {
  term_node_ = nullptr;
  visit_children_of(u_minus_term);
}

void ConstraintSorter::visitMinus(Minus_ptr minus_term) {
  term_node_ = nullptr;
  visit(minus_term->left_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(minus_term->right_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitPlus(Plus_ptr plus_term) {
  TermNode_ptr result_node = nullptr;
  for (auto& term : *(plus_term->term_list)) {
    term_node_ = nullptr;
    visit(term);
    if (result_node == nullptr and term_node_ != nullptr) {
      result_node = term_node_;
      result_node->shiftToRight();
    } else if (term_node_ != nullptr) {
      result_node->addVariableNodes(term_node_->getAllNodes(), false);
      delete term_node_;
    }
  }
  term_node_ = result_node;
}

void ConstraintSorter::visitTimes(Times_ptr times_term) {
  TermNode_ptr result_node = nullptr;
  for (auto& term : *(times_term->term_list)) {
    term_node_ = nullptr;
    visit(term);
    if (result_node == nullptr and term_node_ != nullptr) {
      result_node = term_node_;
      result_node->shiftToRight();
    } else if (term_node_ != nullptr) {
      result_node->addVariableNodes(term_node_->getAllNodes(), false);
      delete term_node_;
    }
  }
  term_node_ = result_node;
}

void ConstraintSorter::visitEq(Eq_ptr eq_term) {
  term_node_ = nullptr;
  visit(eq_term->left_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(eq_term->right_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitNotEq(NotEq_ptr not_eq_term) {
  term_node_ = nullptr;
  visit(not_eq_term->left_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(not_eq_term->right_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}


void ConstraintSorter::visitGt(Gt_ptr gt_term) {
  term_node_ = nullptr;
  visit(gt_term->left_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(gt_term->right_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitGe(Ge_ptr ge_term) {
  term_node_ = nullptr;
  visit(ge_term->left_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(ge_term->right_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitLt(Lt_ptr lt_term) {
  term_node_ = nullptr;
  visit(lt_term->left_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(lt_term->right_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitLe(Le_ptr le_term) {
  term_node_ = nullptr;
  visit(le_term->left_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(le_term->right_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitConcat(Concat_ptr concat_term) {
  TermNode_ptr result_node = nullptr;
  for (auto& term : *(concat_term->term_list)) {
    term_node_ = nullptr;
    visit(term);
    if (result_node == nullptr and term_node_ != nullptr) {
      result_node = term_node_;
      result_node->shiftToRight();
    } else if (term_node_ != nullptr) {
      result_node->addVariableNodes(term_node_->getAllNodes(), false);
      delete term_node_;
    }
  }
  term_node_ = result_node;
}

void ConstraintSorter::visitIn(In_ptr in_term) {
  term_node_ = nullptr;
  visit(in_term->left_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(in_term->right_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}


void ConstraintSorter::visitNotIn(NotIn_ptr not_in_term) {
  term_node_ = nullptr;
  visit(not_in_term->left_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(not_in_term->right_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitLen(Len_ptr len_term) {
  term_node_ = nullptr;
  visit_children_of(len_term);
  if (term_node_ != nullptr) {
    term_node_->shiftToRight();
  }
}

void ConstraintSorter::visitContains(Contains_ptr contains_term) {
  term_node_ = nullptr;
  visit(contains_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(contains_term->search_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitNotContains(NotContains_ptr not_contains_term) {
  term_node_ = nullptr;
  visit(not_contains_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(not_contains_term->search_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitBegins(Begins_ptr begins_term) {
  term_node_ = nullptr;
  visit(begins_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(begins_term->search_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitNotBegins(NotBegins_ptr not_begins_term) {
  term_node_ = nullptr;
  visit(not_begins_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(not_begins_term->search_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitEnds(Ends_ptr ends_term) {
  term_node_ = nullptr;
  visit(ends_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(ends_term->search_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitNotEnds(NotEnds_ptr not_ends_term) {
  term_node_ = nullptr;
  visit(not_ends_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(not_ends_term->search_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitIndexOf(IndexOf_ptr index_of_term) {
  term_node_ = nullptr;
  visit(index_of_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(index_of_term->search_term);
  TermNode_ptr right_node = term_node_;
  if (index_of_term->from_index) {
    term_node_ = nullptr;
    visit(index_of_term->from_index);
    TermNode_ptr right_node_1 = right_node;
    TermNode_ptr right_node_2 = term_node_;
    right_node = process_child_nodes(right_node_1, right_node_2);
    if (right_node != nullptr) {
      right_node->shiftToRight();
    }
  }
  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitLastIndexOf(LastIndexOf_ptr last_index_of_term) {
  term_node_ = nullptr;
  visit(last_index_of_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(last_index_of_term->search_term);
  TermNode_ptr right_node = term_node_;
  if (last_index_of_term->from_index) {
    term_node_ = nullptr;
    visit(last_index_of_term->from_index);
    TermNode_ptr right_node_1 = right_node;
    TermNode_ptr right_node_2 = term_node_;
    right_node = process_child_nodes(right_node_1, right_node_2);
    if (right_node != nullptr) {
      right_node->shiftToRight();
    }
  }

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitCharAt(CharAt_ptr char_at_term) {
  term_node_ = nullptr;
  visit(char_at_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(char_at_term->index_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitSubString(SubString_ptr sub_string_term) {
  term_node_ = nullptr;
  visit(sub_string_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(sub_string_term->start_index_term);
  TermNode_ptr right_node = term_node_;
  if (sub_string_term->end_index_term) {
    term_node_ = nullptr;
    visit(sub_string_term->end_index_term);
    TermNode_ptr right_node_1 = right_node;
    TermNode_ptr right_node_2 = term_node_;
    right_node = process_child_nodes(right_node_1, right_node_2);
    if (right_node != nullptr) {
      right_node->shiftToRight();
    }
  }

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitToUpper(ToUpper_ptr to_upper_term) {
  term_node_ = nullptr;
  visit_children_of(to_upper_term);
}

void ConstraintSorter::visitToLower(ToLower_ptr to_lower_term) {
  term_node_ = nullptr;
  visit_children_of(to_lower_term);
}

void ConstraintSorter::visitTrim(Trim_ptr trim_term) {
  term_node_ = nullptr;
  visit_children_of(trim_term);
}

void ConstraintSorter::visitToString(ToString_ptr to_string_term) {
  term_node_ = nullptr;
  visit_children_of(to_string_term);
}

void ConstraintSorter::visitToInt(ToInt_ptr to_int_term) {
  term_node_ = nullptr;
  visit_children_of(to_int_term);
}

void ConstraintSorter::visitReplace(Replace_ptr replace_term) {
  term_node_ = nullptr;
  visit(replace_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(replace_term->search_term);
  TermNode_ptr right_node_1 = term_node_;
  term_node_ = nullptr;
  visit(replace_term->replace_term);
  TermNode_ptr right_node_2 = term_node_;

  TermNode_ptr right_node = process_child_nodes(right_node_1, right_node_2);
  if (right_node != nullptr) {
    right_node->shiftToRight();
  }
  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitCount(Count_ptr count_term) {
  term_node_ = nullptr;
  visit(count_term->subject_term);
  TermNode_ptr left_node = term_node_;
  term_node_ = nullptr;
  visit(count_term->bound_term);
  TermNode_ptr right_node = term_node_;

  term_node_ = process_child_nodes(left_node, right_node);
}

void ConstraintSorter::visitIte(Ite_ptr ite_term) {
}

void ConstraintSorter::visitReConcat(ReConcat_ptr re_concat_term) {
}

void ConstraintSorter::visitReUnion(ReUnion_ptr re_union_term) {
}

void ConstraintSorter::visitReInter(ReInter_ptr re_inter_term) {
}

void ConstraintSorter::visitReStar(ReStar_ptr re_star_term) {
}

void ConstraintSorter::visitRePlus(RePlus_ptr re_plus_term) {
}

void ConstraintSorter::visitReOpt(ReOpt_ptr re_opt_term) {
}

void ConstraintSorter::visitToRegex(ToRegex_ptr to_regex_term) {
}

void ConstraintSorter::visitUnknownTerm(Unknown_ptr unknown_term) {
  TermNode_ptr result_node = nullptr;
  for (auto& term : *(unknown_term->term_list)) {
    term_node_ = nullptr;
    visit(term);
    if (result_node == nullptr and term_node_ != nullptr) {
      result_node = term_node_;
      result_node->shiftToRight();
    } else if (term_node_ != nullptr) {
      result_node->addVariableNodes(term_node_->getAllNodes(), false);
    }
  }
  term_node_ = result_node;
}

void ConstraintSorter::visitAsQualIdentifier(AsQualIdentifier_ptr as_qid_term) {
}

void ConstraintSorter::visitQualIdentifier(QualIdentifier_ptr qi_term) {
  Variable_ptr variable = symbol_table_->get_variable(qi_term->getVarName());
  if (not variable->isLocalLetVar()) {
    VariableNode_ptr variable_node = get_variable_node(variable);
    term_node_ = new TermNode();
    term_node_->addVariableNode(variable_node, false);
  }
}

void ConstraintSorter::visitTermConstant(TermConstant_ptr term_constant) {
}

void ConstraintSorter::visitIdentifier(Identifier_ptr identifier) {
}

void ConstraintSorter::visitPrimitive(Primitive_ptr primitive) {
}

void ConstraintSorter::visitTVariable(TVariable_ptr t_variable) {
}

void ConstraintSorter::visitTBool(TBool_ptr t_bool) {
}

void ConstraintSorter::visitTInt(TInt_ptr t_int) {
}

void ConstraintSorter::visitTString(TString_ptr t_string) {
}

void ConstraintSorter::visitVariable(Variable_ptr variable) {
}

void ConstraintSorter::visitSort(Sort_ptr sort) {
}

void ConstraintSorter::visitAttribute(Attribute_ptr attribute) {
}

void ConstraintSorter::visitSortedVar(SortedVar_ptr sorted_var) {
}

void ConstraintSorter::visitVarBinding(VarBinding_ptr var_binding) {
  term_node_ = nullptr;
  visit_children_of(var_binding);
}

ConstraintSorter::VariableNode_ptr ConstraintSorter::get_variable_node(Variable_ptr variable) {
  auto it = variable_nodes_.find(variable);
  if (it != variable_nodes_.end()) {
    return it->second;
  }
  VariableNode_ptr variable_node = new VariableNode(variable);
  variable_nodes_[variable] = variable_node;
  return variable_node;
}

ConstraintSorter::TermNode_ptr ConstraintSorter::process_child_nodes(TermNode_ptr left_node,
        TermNode_ptr right_node) {
  TermNode_ptr result_node = nullptr;
  if (left_node != nullptr and right_node != nullptr) {
    right_node->shiftToRight();
    right_node->addVariableNodes(left_node->getAllNodes(), true);
    delete left_node;
    result_node = right_node;
  } else if (left_node != nullptr) {
    left_node->shiftToLeft();
    result_node = left_node;
  } else if (right_node != nullptr) {
    right_node->shiftToRight();
    result_node = right_node;
  }
  return result_node;
}

void ConstraintSorter::sort_terms(std::vector<TermNode_ptr>& term_node_list) {
  std::stable_sort(term_node_list.begin(), term_node_list.end(),
          [](TermNode_ptr left_node, TermNode_ptr right_node) -> bool {
            return (left_node->numOfTotalVars() < right_node->numOfTotalVars());
          });
  DVLOG(VLOG_LEVEL) << "node list sorted";
}

ConstraintSorter::TermNode::TermNode()
        : node_(nullptr) {
}

ConstraintSorter::TermNode::TermNode(Term_ptr node)
        : node_(node) {
}

ConstraintSorter::TermNode::~TermNode() {
}

std::string ConstraintSorter::TermNode::str() {
  std::stringstream ss;
  ss << this->node_ << " -> l:" << left_child_node_list_.size() << " r:" << right_child_node_list_.size();

  ss << " l:";
  for (auto& variable_node : left_child_node_list_) {
    ss << " " << *(variable_node->getVariable());
  }

  ss << " r:";
  for (auto& variable_node : right_child_node_list_) {
    ss << " " << *(variable_node->getVariable());
  }

  return ss.str();
}

void ConstraintSorter::TermNode::setNode(Term_ptr node) {
  this->node_ = node;
}

Term_ptr ConstraintSorter::TermNode::getNode() {
  return node_;
}

void ConstraintSorter::TermNode::addVariableNode(ConstraintSorter::VariableNode_ptr variable, bool is_left_side) {
  is_left_side ? left_child_node_list_.push_back(variable) : right_child_node_list_.push_back(variable);
}

void ConstraintSorter::TermNode::addVariableNodes(std::vector<VariableNode_ptr>& var_node_list, bool is_left_side) {
  is_left_side ?
          merge_vectors(left_child_node_list_, var_node_list) : merge_vectors(right_child_node_list_, var_node_list);
}

std::vector<ConstraintSorter::VariableNode_ptr>& ConstraintSorter::TermNode::getAllNodes() {
  all_child_node_list_.clear();
  all_child_node_list_.insert(all_child_node_list_.begin(), left_child_node_list_.begin(), left_child_node_list_.end());
  all_child_node_list_.insert(all_child_node_list_.end(), right_child_node_list_.begin(), right_child_node_list_.end());
  return all_child_node_list_;
}

std::vector<ConstraintSorter::VariableNode_ptr>& ConstraintSorter::TermNode::getLeftNodes() {
  return left_child_node_list_;
}

std::vector<ConstraintSorter::VariableNode_ptr>& ConstraintSorter::TermNode::getRightNodes() {
  return right_child_node_list_;
}

void ConstraintSorter::TermNode::shiftToLeft() {
  left_child_node_list_.insert(left_child_node_list_.end(), right_child_node_list_.begin(),
          right_child_node_list_.end());
  right_child_node_list_.clear();
}

void ConstraintSorter::TermNode::shiftToRight() {
  right_child_node_list_.insert(right_child_node_list_.begin(), left_child_node_list_.begin(),
          left_child_node_list_.end());
  left_child_node_list_.clear();
}

void ConstraintSorter::TermNode::addMeToChildVariableNodes() {
  for (auto& left_node : left_child_node_list_) {
    left_node->addTermNode(this, true);
  }
  for (auto& right_node : right_child_node_list_) {
    right_node->addTermNode(this, false);
  }
}

int ConstraintSorter::TermNode::numOfTotalVars() {
  return left_child_node_list_.size() + right_child_node_list_.size();
}

int ConstraintSorter::TermNode::numOfLeftVars() {
  return left_child_node_list_.size();
}

int ConstraintSorter::TermNode::numOfRightVars() {
  return right_child_node_list_.size();
}

void ConstraintSorter::TermNode::merge_vectors(std::vector<VariableNode_ptr>& vector_1,
        std::vector<VariableNode_ptr>& vector_2) {
  vector_1.insert(vector_1.end(), vector_2.begin(), vector_2.end());
}

ConstraintSorter::VariableNode::VariableNode(Variable_ptr variable)
        : variable_(variable) {
}

ConstraintSorter::VariableNode::~VariableNode() {
}

std::string ConstraintSorter::VariableNode::str() {
  std::stringstream ss;
  ss << *(this->variable_) << " -> l:" << left_side_var_appearance_list_.size() << " r:"
          << right_side_var_appearance_list_.size();

  ss << " l:";
  for (auto& node : left_side_var_appearance_list_) {
    ss << " " << node->getNode();
  }

  ss << " r:";
  for (auto& node : right_side_var_appearance_list_) {
    ss << " " << node->getNode();
  }
  return ss.str();
}

Variable_ptr ConstraintSorter::VariableNode::getVariable() {
  return variable_;
}

void ConstraintSorter::VariableNode::addTermNode(ConstraintSorter::TermNode_ptr node, bool is_left_side) {
  all_var_appearance_list_.push_back(node);
  is_left_side ? left_side_var_appearance_list_.push_back(node) : right_side_var_appearance_list_.push_back(node);
}

} /* namespace Solver */
} /* namespace Vlab */
