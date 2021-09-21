/**
 * @name Unsafe vector access
 * @description std::vector::operator[] does not do any runtime
 *              bounds-checking, so it is safer to use std::vector::at()
 * @kind problem
 * @problem.severity warning
 * @id cpp/unsafe-vector-access
 * @tags security
 *       external/cwe/cwe-125
 */

import cpp
import semmle.code.cpp.controlflow.Guards
import semmle.code.cpp.dataflow.DataFlow
import semmle.code.cpp.rangeanalysis.SimpleRangeAnalysis
import semmle.code.cpp.rangeanalysis.RangeAnalysisUtils
import semmle.code.cpp.valuenumbering.GlobalValueNumbering
import semmle.code.cpp.valuenumbering.HashCons

// A call to `operator[]`.
class ArrayIndexCall extends FunctionCall {
  ClassTemplateInstantiation ti;
  TemplateClass tc;

  ArrayIndexCall() {
    this.getTarget().getName() = "operator[]" and
    ti = this.getQualifier().getType().getUnspecifiedType() and
    tc = ti.getTemplate() and
    tc.getSimpleName() != "map" and
    tc.getSimpleName() != "match_results"
  }

  ClassTemplateInstantiation getClassTemplateInstantiation() { result = ti }

  TemplateClass getTemplateClass() { result = tc }
}

// A call to `size` or `length`.
class SizeCall extends FunctionCall {
  string fname;

  SizeCall() {
    fname = this.getTarget().getName() and
    (fname = "size" or fname = "length")
  }
}

// `x[i]` is safe if `x` is a `std::array` (fixed-size array)
// and `i` within the bounds of the array.
predicate indexK_with_fixedarray(ClassTemplateInstantiation t, ArrayIndexCall call) {
  t = call.getClassTemplateInstantiation() and
  exists(Expr idx |
    t.getSimpleName() = "array" and
    idx = call.getArgument(0) and
    lowerBound(idx) >= 0 and
    upperBound(idx) < t.getTemplateArgument(1).(Literal).getValue().toInt()
  )
}

// Holds if `cond` is a Boolean condition that checks the size of
// the array. It handles the following code patterns:
//
// 1.  `if (!x.empty()) { ... }`
// 2.  `if (x.length()) { ... }`
// 3.  `if (x.size() > 2) { ... }`
// 4.  `if (x.size() == 2) { ... }`
// 5.  `if (x.size() != 0) { ... }`
//
// If it safe to assume that `x.size() >= minsize` on the exit `branch`.
predicate minimum_size_cond(Expr cond, Expr arrayExpr, int minsize, boolean branch) {
  // `if (!x.empty()) { ...x[0]... }`
  exists(FunctionCall emptyCall |
    cond = emptyCall and
    arrayExpr = emptyCall.getQualifier() and
    emptyCall.getTarget().getName() = "empty" and
    minsize = 1 and
    branch = false
  )
  or
  // `if (x.length()) { ...x[0]... }`
  exists(SizeCall sizeCall |
    cond = sizeCall and
    arrayExpr = sizeCall.getQualifier() and
    minsize = 1 and
    branch = true
  )
  or
  // `if (x.size() > 2) { ...x[2]... }`
  exists(SizeCall sizeCall, Expr k, RelationStrictness strict |
    arrayExpr = sizeCall.getQualifier() and
    relOpWithSwapAndNegate(cond, sizeCall, k, Greater(), strict, branch)
  |
    strict = Strict() and minsize = 1 + k.getValue().toInt()
    or
    strict = Nonstrict() and minsize = k.getValue().toInt()
  )
  or
  // `if (x.size() == 2) { ...x[1]... }`
  exists(SizeCall sizeCall, Expr k |
    arrayExpr = sizeCall.getQualifier() and
    eqOpWithSwapAndNegate(cond, sizeCall, k, true, branch) and
    minsize = k.getValue().toInt()
  )
  or
  // `if (x.size() != 0) { ...x[0]... }`
  exists(SizeCall sizeCall, Expr k |
    arrayExpr = sizeCall.getQualifier() and
    eqOpWithSwapAndNegate(cond, sizeCall, k, false, branch) and
    k.getValue().toInt() = 0 and
    minsize = 1
  )
}

// Array accesses like these are safe:
// `if (!x.empty()) { ... x[0] ... }`
// `if (x.size() > 2) { ... x[2] ... }`
predicate indexK_with_check(GuardCondition guard, ArrayIndexCall call) {
  exists(Expr arrayExpr, BasicBlock block, int i, int minsize, boolean branch |
    minimum_size_cond(guard, arrayExpr, minsize, branch) and
    (
      globalValueNumber(arrayExpr) = globalValueNumber(call.getQualifier()) or
      hashCons(arrayExpr) = hashCons(call.getQualifier())
    ) and
    guard.controls(block, branch) and
    block.contains(call) and
    i = call.getArgument(0).getValue().toInt() and
    0 <= i and
    i < minsize
  )
}

// Array accesses like this are safe:
// `if (i < x.size()) { ... x[i] ... }`
predicate indexI_with_check(GuardCondition guard, ArrayIndexCall call) {
  exists(Expr idx, SizeCall sizeCall, BasicBlock block, boolean branch |
    relOpWithSwapAndNegate(guard, idx, sizeCall, Lesser(), Strict(), branch) and
    (
      globalValueNumber(sizeCall.getQualifier()) = globalValueNumber(call.getQualifier()) and
      globalValueNumber(idx) = globalValueNumber(call.getArgument(0))
      or
      hashCons(sizeCall.getQualifier()) = hashCons(call.getQualifier()) and
      hashCons(idx) = hashCons(call.getArgument(0))
    ) and
    guard.controls(block, branch) and
    block.contains(call)
  )
}

// Array accesses like this are safe:
// `if (!x.empty()) { ... x[x.size() - 1] ... }`
predicate index_last_with_check(GuardCondition guard, ArrayIndexCall call) {
  exists(Expr arrayExpr, SubExpr minusExpr, SizeCall sizeCall, BasicBlock block, boolean branch |
    minimum_size_cond(guard, arrayExpr, _, branch) and
    (
      globalValueNumber(arrayExpr) = globalValueNumber(call.getQualifier()) or
      hashCons(arrayExpr) = hashCons(call.getQualifier())
    ) and
    guard.controls(block, branch) and
    block.contains(call) and
    minusExpr = call.getArgument(0) and
    minusExpr.getRightOperand().getValue().toInt() = 1 and
    DataFlow::localExprFlow(sizeCall, minusExpr.getLeftOperand()) and
    (
      globalValueNumber(sizeCall.getQualifier()) = globalValueNumber(call.getQualifier()) or
      hashCons(sizeCall.getQualifier()) = hashCons(call.getQualifier())
    )
  )
}

from ArrayIndexCall call
where
  not indexK_with_fixedarray(_, call) and
  not indexK_with_check(_, call) and
  not indexI_with_check(_, call) and
  not index_last_with_check(_, call) and
  // Ignore accesses like this: `vsnprintf(&buffer[0], buffer.size(), format, args)`
  // That's pointer arithmetic, not a deref, so it's usually a false positive.
  not exists(AddressOfExpr addrExpr | addrExpr.getOperand() = call) and
  // Ignore results in the xmpsdk directory.
  not call.getLocation().getFile().getRelativePath().matches("xmpsdk/%")
select call, "Unsafe use of operator[]. Use the at() method instead."
