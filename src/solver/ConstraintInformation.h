/*
 * ConstraintInformation.h
 *
 *  Created on: Jun 7, 2016
 *      Author: baki
 *   Copyright: Copyright 2015 The ABC Authors. All rights reserved. 
 *              Use of this source code is governed license that can
 *              be found in the COPYING file.
 */

#ifndef SRC_SOLVER_CONSTRAINTINFORMATION_H_
#define SRC_SOLVER_CONSTRAINTINFORMATION_H_

#include <map>
#include <set>
#include <utility>

#include "../smt/typedefs.h"

namespace Vlab {
namespace Solver {

class ConstraintInformation {
 public:
  ConstraintInformation();
  virtual ~ConstraintInformation();

  bool is_component(const SMT::Visitable_ptr) const;
  void add_component(const SMT::Visitable_ptr);
  void remove_component(const SMT::Visitable_ptr);
  const std::set<SMT::Visitable_ptr> get_components() const;
  
  bool has_arithmetic_constraint(const SMT::Visitable_ptr) const;
  void add_arithmetic_constraint(const SMT::Visitable_ptr);

  bool has_string_constraint(const SMT::Visitable_ptr) const;
  void add_string_constraint(const SMT::Visitable_ptr);

  bool has_mixed_constraint(const SMT::Visitable_ptr) const;
  void add_mixed_constraint(const SMT::Visitable_ptr);

 private:
  std::set<SMT::Visitable_ptr> components_;
  std::set<SMT::Visitable_ptr> arithmetic_constraints_;
  std::set<SMT::Visitable_ptr> string_constraints_;
  std::set<SMT::Visitable_ptr> mixed_constraints_;
};

using ConstraintInformation_ptr = ConstraintInformation*;

} /* namespace Solver */
} /* namespace Vlab */

#endif /* SRC_SOLVER_CONSTRAINTINFORMATION_H_ */
