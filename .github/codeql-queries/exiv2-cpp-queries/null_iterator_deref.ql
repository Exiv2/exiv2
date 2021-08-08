/**
 * @name NULL iterator deref
 * @description Dereferencing an iterator without checking that it's valid could cause a crash.
 * @kind problem
 * @problem.severity warning
 * @id cpp/null-iterator-deref
 * @tags security
 *       external/cwe/cwe-476
 */

import cpp
import semmle.code.cpp.controlflow.Guards
import semmle.code.cpp.dataflow.DataFlow

// Holds if `cond` is a condition like `use == table.end()`.
// `eq_is_true` is `true` for `==`, `false` for '!=`.
// Note: the `==` is actually an overloaded `operator==`.
predicate end_condition(GuardCondition cond, Expr use, FunctionCall endCall, boolean eq_is_true) {
  exists(FunctionCall eq |
    exists(string opName | eq.getTarget().getName() = opName |
      opName = "operator==" and eq_is_true = true
      or
      opName = "operator!=" and eq_is_true = false
    ) and
    DataFlow::localExprFlow(use, eq.getAnArgument()) and
    DataFlow::localExprFlow(endCall, eq.getAnArgument()) and
    endCall.getTarget().getName() = "end" and
    DataFlow::localExprFlow(eq, cond)
  )
}

from FunctionCall call, Expr use
where
  call.getTarget().getName() = "findKey" and
  DataFlow::localExprFlow(call, use) and
  use != call and
  not use.(AssignExpr).getRValue() = call and
  not end_condition(_, use, _, _) and
  not exists(
    Expr cond_use, FunctionCall endCall, GuardCondition cond, BasicBlock block, boolean branch
  |
    end_condition(cond, cond_use, endCall, branch) and
    DataFlow::localExprFlow(call, cond_use) and
    cond.controls(block, branch.booleanNot()) and
    block.contains(use)
  )
select call, "Iterator returned by findKey might cause a null deref $@.", use, "here"
