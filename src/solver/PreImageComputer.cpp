/*
 * PreImageComputer.cpp
 *
 *  Created on: Jun 24, 2015
 *      Author: baki
 */

#include "PreImageComputer.h"

namespace Vlab {
namespace Solver {

using namespace SMT;

const int PreImageComputer::VLOG_LEVEL = 12;
// TODO intersect with result post
PreImageComputer::PreImageComputer(SymbolTable_ptr symbol_table, VariablePathTable& variable_path_table, const TermValueMap& post_images)
        : symbol_table(symbol_table), variable_path_table (variable_path_table),
          post_images (post_images) {
}

PreImageComputer::~PreImageComputer() {
  for (auto entry : pre_images) {
    delete entry.second;
  }

  pre_images.clear();
}

void PreImageComputer::start() {
  Value_ptr initial_value = nullptr;
  Term_ptr root_term = nullptr;
  DVLOG(VLOG_LEVEL) << "Pre image computation start";
  for (auto& path_entry : variable_path_table) {
    current_path = path_entry.second;
    root_term = current_path.back();

    initial_value = getTermPreImage(root_term);
    if (initial_value not_eq nullptr) {
      visit(root_term);
      return;
    }

    initial_value = getTermPostImage(root_term);
    setTermPreImage(root_term, initial_value->clone());
    visit(root_term);
  }
  end();
}

void PreImageComputer::end() {
}

void PreImageComputer::visitScript(Script_ptr script) {
}

void PreImageComputer::visitCommand(Command_ptr command) {
}

void PreImageComputer::visitAssert(Assert_ptr assert_command) {
}

void PreImageComputer::visitTerm(Term_ptr term) {
}

void PreImageComputer::visitExclamation(Exclamation_ptr exclamation_term) {
}

void PreImageComputer::visitExists(Exists_ptr exists_term) {
}

void PreImageComputer::visitForAll(ForAll_ptr for_all_term) {
}

void PreImageComputer::visitLet(Let_ptr let_term) {
}

void PreImageComputer::visitAnd(And_ptr and_term) {
  LOG(ERROR) << "Unexpected term: " << *and_term;
}

void PreImageComputer::visitOr(Or_ptr or_term) {
  LOG(ERROR) << "Unexpected term: " << *or_term;
}

void PreImageComputer::visitNot(Not_ptr not_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *not_term;
  popTerm(not_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(not_term);
  child_value = term_value->clone();
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitUMinus(UMinus_ptr u_minus_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *u_minus_term;
  popTerm(u_minus_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(u_minus_term);
  switch (term_value->getType()) {
  case Value::Type::INT_CONSTANT: {
    int value = - term_value->getIntConstant();
    child_value = new Value(Value::Type::INT_CONSTANT, value);
    break;
  }
  case Value::Type::INT_AUTOMATON: {
    if (term_value->getIntAutomaton()->isAcceptingSingleInt()) {
      int value = (- term_value->getIntAutomaton()->getAnAcceptingInt());
      child_value = new Value(Value::Type::INT_CONSTANT, value);
    } else {
      child_value = new Value(Value::Type::INT_AUTOMATON,
              term_value->getIntAutomaton()->uminus());
    }
    break;
  }
  case Value::Type::INTBOOL_AUTOMATON: {
    // do minus operation on automaton
    LOG(FATAL) << "implement me";
    break;
  }
  default:
  break;
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitMinus(Minus_ptr minus_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *minus_term;
  popTerm(minus_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  Value_ptr result = nullptr;

  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(minus_term);
  Value_ptr child_post_value = getTermPostImage(child_term);

  if (child_term == minus_term->left_term) {
    Value_ptr right_child = getTermPostImage(minus_term->right_term);
    result = term_value->plus(right_child);
  } else {
    Value_ptr left_child = getTermPostImage(minus_term->left_term);
    result = left_child->minus(term_value);
  }

  child_value = child_post_value->intersect(result);
  delete result; result = nullptr;

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitPlus(Plus_ptr plus_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *plus_term;
  popTerm(plus_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  Value_ptr result = nullptr;

  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(plus_term);
  Value_ptr child_post_value = getTermPostImage(child_term);

  if (child_term == plus_term->left_term) {
    Value_ptr right_child = getTermPostImage(plus_term->right_term);
    result = term_value->minus(right_child);

  } else {
    Value_ptr left_child = getTermPostImage(plus_term->left_term);
    result = term_value->minus(left_child);
  }

  child_value = child_post_value->intersect(result);
  delete result; result = nullptr;

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitEq(Eq_ptr eq_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *eq_term;
  popTerm(eq_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(eq_term);

  if (Value::Type::BOOl_CONSTANT == term_value->getType()){
    child_value = getTermPostImage(child_term)->clone();
  } else {
    child_value = term_value->clone();
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}


void PreImageComputer::visitNotEq(NotEq_ptr not_eq_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *not_eq_term;
  popTerm(not_eq_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(not_eq_term);

  if (Value::Type::BOOl_CONSTANT == term_value->getType()){
    CHECK_EQ(true, term_value->getBoolConstant());
    child_value = getTermPostImage(child_term)->clone();
  } else {
    if (term_value->isSingleValue()) {
      child_value = getTermPostImage(child_term)->difference(term_value);
    } else {
      child_value = getTermPostImage(child_term)->clone();
    }
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitGt(Gt_ptr gt_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *gt_term;
  popTerm(gt_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr child_post_value = getTermPostImage(child_term);

  if (Value::Type::INT_CONSTANT == child_post_value->getType()) {
    child_value = child_post_value->clone();
  } else {
    if (child_term == gt_term->left_term) {
      Value_ptr param_right = getTermPostImage(gt_term->right_term);
      if (Value::Type::INT_CONSTANT == param_right->getType()) {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictGreaterThanTo(param_right->getIntConstant()));
      } else {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictGreaterThanTo(param_right->getIntAutomaton()));
      }
    } else {
      Value_ptr param_left = getTermPostImage(gt_term->left_term);
      if (Value::Type::INT_CONSTANT == param_left->getType()) {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictLessThanTo(param_left->getIntConstant()));
      } else {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictLessThanTo(param_left->getIntAutomaton()));
      }
    }
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitGe(Ge_ptr ge_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *ge_term;
  popTerm(ge_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr child_post_value = getTermPostImage(child_term);

  if (Value::Type::INT_CONSTANT == child_post_value->getType()) {
    child_value = child_post_value->clone();
  } else {
    if (child_term == ge_term->left_term) {
      Value_ptr param_right = getTermPostImage(ge_term->right_term);
      if (Value::Type::INT_CONSTANT == param_right->getType()) {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictGreaterThanOrEqualTo(param_right->getIntConstant()));
      } else {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictGreaterThanOrEqualTo(param_right->getIntAutomaton()));
      }
    } else {
      Value_ptr param_left = getTermPostImage(ge_term->left_term);
      if (Value::Type::INT_CONSTANT == param_left->getType()) {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictLessThanOrEqualTo(param_left->getIntConstant()));
      } else {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictLessThanOrEqualTo(param_left->getIntAutomaton()));
      }
    }
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitLt(Lt_ptr lt_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *lt_term;
  popTerm(lt_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr child_post_value = getTermPostImage(child_term);

  if (Value::Type::INT_CONSTANT == child_post_value->getType()) {
    child_value = child_post_value->clone();
  } else {
    if (child_term == lt_term->left_term) {
      Value_ptr param_right = getTermPostImage(lt_term->right_term);
      if (Value::Type::INT_CONSTANT == param_right->getType()) {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictLessThanTo(param_right->getIntConstant()));
      } else {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictLessThanTo(param_right->getIntAutomaton()));
      }
    } else {
      Value_ptr param_left = getTermPostImage(lt_term->left_term);
      if (Value::Type::INT_CONSTANT == param_left->getType()) {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictGreaterThanTo(param_left->getIntConstant()));
      } else {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictGreaterThanTo(param_left->getIntAutomaton()));
      }
    }
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitLe(Le_ptr le_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *le_term;
  popTerm(le_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }


  Value_ptr child_post_value = getTermPostImage(child_term);

  if (Value::Type::INT_CONSTANT == child_post_value->getType()) {
    child_value = child_post_value->clone();
  } else {
    if (child_term == le_term->left_term) {
      Value_ptr param_right = getTermPostImage(le_term->right_term);
      if (Value::Type::INT_CONSTANT == param_right->getType()) {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictLessThanOrEqualTo(param_right->getIntConstant()));
      } else {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictLessThanOrEqualTo(param_right->getIntAutomaton()));
      }
    } else {
      Value_ptr param_left = getTermPostImage(le_term->left_term);
      if (Value::Type::INT_CONSTANT == param_left->getType()) {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictGreaterThanOrEqualTo(param_left->getIntConstant()));
      } else {
        child_value = new Value(Value::Type::INT_AUTOMATON,
                child_post_value->getIntAutomaton()->restrictGreaterThanOrEqualTo(param_left->getIntAutomaton()));
      }
    }
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitConcat(Concat_ptr concat_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *concat_term;
  popTerm(concat_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(concat_term);

  // Figure out position of the variable in concat list
  Theory::StringAutomaton_ptr left_of_child = nullptr;
  Theory::StringAutomaton_ptr right_of_child = nullptr;
  Theory::StringAutomaton_ptr current_auto = nullptr;
  for (auto& term_ptr : *(concat_term->term_list)) {
    if (child_term == term_ptr) {
      left_of_child = current_auto;
      current_auto = nullptr;
    } else {
      Value_ptr post_value = getTermPostImage(term_ptr);
      if (current_auto == nullptr) {
        current_auto = post_value->getStringAutomaton()->clone();
      } else {
        Theory::StringAutomaton_ptr tmp = current_auto;
        current_auto = current_auto->concat(post_value->getStringAutomaton());
        delete tmp;
      }
    }
  }
  right_of_child = current_auto;
  // do the preconcat operations
  Theory::StringAutomaton_ptr tmp_parent_auto = term_value->getStringAutomaton();
  Theory::StringAutomaton_ptr child_result_auto = nullptr;

  if (left_of_child != nullptr) {
    child_result_auto = tmp_parent_auto->preConcatRight(left_of_child);
    tmp_parent_auto = child_result_auto;
  }

  if (right_of_child != nullptr) {
    Theory::StringAutomaton_ptr  tmp = child_result_auto;
    child_result_auto = tmp_parent_auto->preConcatLeft(right_of_child);
    delete tmp; tmp = nullptr;
  }

  delete left_of_child; left_of_child = nullptr;
  delete right_of_child; right_of_child = nullptr;

  child_value = new Value(Value::Type::STRING_AUTOMATON, child_result_auto);
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitIn(In_ptr in_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *in_term;
  popTerm(in_term);
  Term_ptr child_term = current_path.back();

  if (child_term == in_term->right_term) {
    return; // in operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(in_term);
  child_value = term_value->clone();
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitNotIn(NotIn_ptr not_in_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *not_in_term;
  popTerm(not_in_term);
  Term_ptr child_term = current_path.back();

  if (child_term == not_in_term->right_term) {
    return; // notIn operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(not_in_term);
  child_value = term_value->clone();
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitLen(Len_ptr len_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *len_term;
  popTerm(len_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(len_term);
  Value_ptr child_post_value = getTermPostImage(child_term);

  if (Value::Type::INT_CONSTANT == term_value->getType()) {
    child_value = new Value(Value::Type::STRING_AUTOMATON,
            child_post_value->getStringAutomaton()->restrictLengthTo(term_value->getIntConstant()));
  } else {
    child_value = new Value(Value::Type::STRING_AUTOMATON,
            child_post_value->getStringAutomaton()->restrictLengthTo(term_value->getIntAutomaton()));
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitContains(Contains_ptr contains_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *contains_term;
  popTerm(contains_term);
  Term_ptr child_term = current_path.back();

  if (child_term == contains_term->search_term) {
    return; // contains operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(contains_term);
  child_value = term_value->clone();
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitNotContains(NotContains_ptr not_contains_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *not_contains_term;
  popTerm(not_contains_term);
  Term_ptr child_term = current_path.back();

  if (child_term == not_contains_term->search_term) {
    return; // notContains operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(not_contains_term);
  child_value = term_value->clone();
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitBegins(Begins_ptr begins_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *begins_term;
  popTerm(begins_term);
  Term_ptr child_term = current_path.back();

  if (child_term == begins_term->search_term) {
    return; // begins operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(begins_term);
  child_value = term_value->clone();
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitNotBegins(NotBegins_ptr not_begins_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *not_begins_term;
  popTerm(not_begins_term);
  Term_ptr child_term = current_path.back();

  if (child_term == not_begins_term->search_term) {
    return; // notBegins operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(not_begins_term);
  child_value = term_value->clone();
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitEnds(Ends_ptr ends_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *ends_term;
  popTerm(ends_term);
  Term_ptr child_term = current_path.back();

  if (child_term == ends_term->search_term) {
    return; // ends operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(ends_term);
  child_value = term_value->clone();
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitNotEnds(NotEnds_ptr not_ends_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *not_ends_term;
  popTerm(not_ends_term);
  Term_ptr child_term = current_path.back();

  if (child_term == not_ends_term->search_term) {
    return; // notEnds operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(not_ends_term);
  child_value = term_value->clone();
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitIndexOf(IndexOf_ptr index_of_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *index_of_term;
  popTerm(index_of_term);
  Term_ptr child_term = current_path.back();

  if (child_term == index_of_term->search_term) {
    return; // indexOf operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(index_of_term);
  Value_ptr child_post_value = getTermPostImage(child_term);
  Value_ptr param_search = getTermPostImage(index_of_term->search_term);

  if (Value::Type::INT_CONSTANT == term_value->getType()) {
    child_value = new Value(Value::Type::STRING_AUTOMATON,
            child_post_value->getStringAutomaton()
            ->restrictIndexOfTo(term_value->getIntConstant(), param_search->getStringAutomaton()));
  } else {
    child_value = new Value(Value::Type::STRING_AUTOMATON,
                child_post_value->getStringAutomaton()
                ->restrictIndexOfTo(term_value->getIntAutomaton(), param_search->getStringAutomaton()));
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitLastIndexOf(SMT::LastIndexOf_ptr last_index_of_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *last_index_of_term;
  popTerm(last_index_of_term);
  Term_ptr child_term = current_path.back();

  if (child_term == last_index_of_term->search_term) {
    return; // lastIndexOf operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(last_index_of_term);
  Value_ptr child_post_value = getTermPostImage(child_term);
  Value_ptr param_search = getTermPostImage(last_index_of_term->search_term);

  if (Value::Type::INT_CONSTANT == term_value->getType()) {
    child_value = new Value(Value::Type::STRING_AUTOMATON,
            child_post_value->getStringAutomaton()
            ->restrictLastIndexOfTo(term_value->getIntConstant(), param_search->getStringAutomaton()));

  } else {
    child_value = new Value(Value::Type::STRING_AUTOMATON,
                child_post_value->getStringAutomaton()
                ->restrictLastIndexOfTo(term_value->getIntAutomaton(), param_search->getStringAutomaton()));
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

/**
 *
 */
void PreImageComputer::visitCharAt(SMT::CharAt_ptr char_at_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *char_at_term;
  popTerm(char_at_term);
  Term_ptr child_term = current_path.back();

  if (child_term == char_at_term->index_term) {
    return; // charAt operation does not have any restriction on right hand side
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(char_at_term);
  Value_ptr child_post_value = getTermPostImage(child_term);
  Value_ptr index_value = getTermPostImage(char_at_term->index_term);
  if (Value::Type::INT_CONSTANT == index_value->getType()) {
    child_value = new Value(Value::Type::STRING_AUTOMATON,
            child_post_value->getStringAutomaton()
            ->restrictAtIndexTo(index_value->getIntConstant(), term_value->getStringAutomaton()));
  } else {
    child_value = new Value(Value::Type::STRING_AUTOMATON,
            child_post_value->getStringAutomaton()
            ->restrictAtIndexTo(index_value->getIntAutomaton(), term_value->getStringAutomaton()));
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

/**
 * TODO check if we can do preimage with endswith ??
 */
void PreImageComputer::visitSubString(SMT::SubString_ptr sub_string_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *sub_string_term;
  popTerm(sub_string_term);
  Term_ptr child_term = current_path.back();

  if (child_term == sub_string_term->start_index_term or
          child_term == sub_string_term->end_index_term) {
    return; // subString operation does not have any restriction on indexes
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Theory::StringAutomaton_ptr child_pre_auto = nullptr;
  Value_ptr term_value = getTermPreImage(sub_string_term);
  Value_ptr child_post_value = getTermPostImage(child_term);
  Value_ptr start_index_value = getTermPostImage(sub_string_term->start_index_term);

  if (sub_string_term->end_index_term == nullptr) {
    if (Value::Type::INT_CONSTANT == start_index_value->getType()) {
      child_value = new Value(Value::Type::STRING_AUTOMATON,
              child_post_value->getStringAutomaton()
              ->restrictFromIndexToEndTo(start_index_value->getIntConstant(), term_value->getStringAutomaton()));
    } else {
      child_value = new Value(Value::Type::STRING_AUTOMATON,
              child_post_value->getStringAutomaton()
              ->restrictFromIndexToEndTo(start_index_value->getIntAutomaton(), term_value->getStringAutomaton()));
    }

  } else { //term_value already contains end index
    if (Value::Type::INT_CONSTANT == start_index_value->getType()) {
      child_value = new Value(Value::Type::STRING_AUTOMATON,
              child_post_value->getStringAutomaton()
              ->restrictAtIndexTo(start_index_value->getIntConstant(), term_value->getStringAutomaton()));
    } else {
      child_value = new Value(Value::Type::STRING_AUTOMATON,
              child_post_value->getStringAutomaton()
              ->restrictAtIndexTo(start_index_value->getIntAutomaton(), term_value->getStringAutomaton()));
    }
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitSubStringFirstOf(SMT::SubStringFirstOf_ptr sub_string_first_of_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *sub_string_first_of_term;
  popTerm(sub_string_first_of_term);
  Term_ptr child_term = current_path.back();

  if (child_term == sub_string_first_of_term->start_index_term) {
    return; // visitSubStringFirstOf operation does not have any restriction on indexes
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Theory::StringAutomaton_ptr child_pre_auto = nullptr;
  Value_ptr term_value = getTermPreImage(sub_string_first_of_term);
  Value_ptr child_post_value = getTermPostImage(child_term);

  child_value = new Value(Value::Type::STRING_AUTOMATON,
          child_post_value->getStringAutomaton()
          ->ends(term_value->getStringAutomaton()));

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitSubStringLastOf(SMT::SubStringLastOf_ptr sub_string_last_of_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *sub_string_last_of_term;
  popTerm(sub_string_last_of_term);
  Term_ptr child_term = current_path.back();

  if (child_term == sub_string_last_of_term->start_index_term) {
    return; // visitSubStringLastOf operation does not have any restriction on indexes
  }

  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Theory::StringAutomaton_ptr child_pre_auto = nullptr;
  Value_ptr term_value = getTermPreImage(sub_string_last_of_term);
  Value_ptr child_post_value = getTermPostImage(child_term);
  Value_ptr start_index_value = getTermPostImage(sub_string_last_of_term->start_index_term);

  child_value = new Value(Value::Type::STRING_AUTOMATON,
          child_post_value->getStringAutomaton()
          ->ends(term_value->getStringAutomaton()));

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

/**
 * TODO improve pre image computation
 *
 */
void PreImageComputer::visitToUpper(SMT::ToUpper_ptr to_upper_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *to_upper_term;
  popTerm(to_upper_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(to_upper_term);
  Value_ptr child_post_value = getTermPostImage(child_term);
  Theory::StringAutomaton_ptr child_pre_auto = term_value->getStringAutomaton()
      ->preToUpperCase(child_post_value->getStringAutomaton());
  child_value = new Value(Value::Type::STRING_AUTOMATON, child_pre_auto);
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitToLower(SMT::ToLower_ptr to_lower_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *to_lower_term;
  popTerm(to_lower_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(to_lower_term);
  Value_ptr child_post_value = getTermPostImage(child_term);
  Theory::StringAutomaton_ptr child_pre_auto = term_value->getStringAutomaton()
      ->preToLowerCase(child_post_value->getStringAutomaton());
  child_value = new Value(Value::Type::STRING_AUTOMATON, child_pre_auto);
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitTrim(SMT::Trim_ptr trim_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *trim_term;
  popTerm(trim_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(trim_term);
  Value_ptr child_post_value = getTermPostImage(child_term);
  Theory::StringAutomaton_ptr child_pre_auto = term_value->getStringAutomaton()
      ->preTrim(child_post_value->getStringAutomaton());
  child_value = new Value(Value::Type::STRING_AUTOMATON, child_pre_auto);
  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitReplace(Replace_ptr replace_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *replace_term;
  popTerm(replace_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr term_value = getTermPreImage(replace_term);
  Value_ptr child_post_value = getTermPostImage(child_term);
  Value_ptr search_auto_value = getTermPostImage(replace_term->search_term);
  Value_ptr replace_auto_value = getTermPostImage(replace_term->replace_term);

  if (child_term == replace_term->replace_term) {
    Theory::StringAutomaton_ptr child_pre_auto = term_value->getStringAutomaton()
        ->preReplace(search_auto_value->getStringAutomaton(),
            replace_auto_value->getStringAutomaton()->getAnAcceptingString(),
            child_post_value->getStringAutomaton());
    child_value = new Value(Value::Type::STRING_AUTOMATON, child_pre_auto);
  } else {
    child_value = child_post_value->clone();
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitCount(Count_ptr count_term) {
  LOG(FATAL) << "implement me";
  visit_children_of(count_term);
}

void PreImageComputer::visitIte(Ite_ptr ite_term) {
}

void PreImageComputer::visitReConcat(ReConcat_ptr re_concat_term) {
}

void PreImageComputer::visitToRegex(ToRegex_ptr to_regex_term) {
}

void PreImageComputer::visitUnknownTerm(Unknown_ptr unknown_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *unknown_term;
  QualIdentifier_ptr qi_term = dynamic_cast<QualIdentifier_ptr>(unknown_term->term);
  LOG(WARNING) << "operation is not known, over-approximate params of operation: '" << qi_term->getVarName() << "'";
  popTerm(unknown_term);
  Term_ptr child_term = current_path.back();
  Value_ptr child_value = getTermPreImage(child_term);
  if (child_value not_eq nullptr) {
    visit(child_term);
    return;
  }

  Value_ptr child_post_value = getTermPostImage(child_term);
  switch (child_post_value->getType()) {
    case Value::Type::STRING_AUTOMATON:
      child_value = new Value(Value::Type::STRING_AUTOMATON, Theory::StringAutomaton::makeAnyString());
      break;
    case Value::Type::INT_CONSTANT:
    case Value::Type::INT_AUTOMATON:
      child_value = new Value(Value::Type::INT_AUTOMATON, Theory::IntAutomaton::makeAnyInt());
      break;
    default:
      child_value = child_post_value->clone();
      break;
  }

  setTermPreImage(child_term, child_value);
  visit(child_term);
}

void PreImageComputer::visitAsQualIdentifier(AsQualIdentifier_ptr as_qid_term) {
}

void PreImageComputer::visitQualIdentifier(QualIdentifier_ptr qi_term) {
  DVLOG(VLOG_LEVEL) << "pop: " << *qi_term;
  popTerm(qi_term);

  Value_ptr term_pre_value = getTermPreImage(qi_term);
  Value_ptr variable_old_value = symbol_table->getValue(qi_term->getVarName());

  Value_ptr variable_new_value = variable_old_value->intersect(term_pre_value);

  symbol_table->setValue(qi_term->getVarName(), variable_new_value);
  delete variable_old_value; variable_old_value = nullptr;
}

void PreImageComputer::visitTermConstant(TermConstant_ptr term_constant) {
}

void PreImageComputer::visitIdentifier(Identifier_ptr identifier) {
}

void PreImageComputer::visitPrimitive(Primitive_ptr primitive) {
}

void PreImageComputer::visitTVariable(TVariable_ptr t_variable) {
}

void PreImageComputer::visitTBool(TBool_ptr t_bool) {
}

void PreImageComputer::visitTInt(TInt_ptr t_int) {
}

void PreImageComputer::visitTString(TString_ptr t_string) {
}

void PreImageComputer::visitVariable(Variable_ptr variable) {
}

void PreImageComputer::visitSort(Sort_ptr sort) {
}

void PreImageComputer::visitAttribute(Attribute_ptr attribute) {
}

void PreImageComputer::visitSortedVar(SortedVar_ptr sorted_var) {
}

void PreImageComputer::visitVarBinding(VarBinding_ptr var_binding) {
}

Value_ptr PreImageComputer::getTermPostImage(SMT::Term_ptr term) {
  auto iter = post_images.find(term);
  if (iter == post_images.end()) {
    LOG(FATAL)<< "post image value is not computed for term: " << *term;
  }
  return iter->second;
}

Value_ptr PreImageComputer::getTermPreImage(SMT::Term_ptr term) {
  auto iter = pre_images.find(term);
  if (iter == pre_images.end()) {
    return nullptr;
  }
  return iter->second;
}

bool PreImageComputer::setTermPreImage(SMT::Term_ptr term, Value_ptr value) {
  auto result = pre_images.insert(std::make_pair(term, value));
  if (result.second == false) {
    LOG(FATAL)<< "value is already computed for term: " << *term;
  }
  return result.second;
}

void PreImageComputer::popTerm(SMT::Term_ptr term) {
  if (current_path.back() == term) {
    current_path.pop_back();
  } else {
    LOG(FATAL) << "expected '" << *term << "', but found '" << *current_path.back() << "'";
  }
}

} /* namespace Solver */
} /* namespace Vlab */
