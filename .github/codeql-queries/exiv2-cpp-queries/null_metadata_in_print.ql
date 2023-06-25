/**
 * @name Null metadata in print function
 * @description Print functions need to check that the metadata isn't null before calling methods on it.
 * @kind problem
 * @problem.severity warning
 * @id cpp/null-metadata-in-print
 */

import cpp
import semmle.code.cpp.controlflow.Guards
import semmle.code.cpp.controlflow.Nullness
import semmle.code.cpp.rangeanalysis.RangeAnalysisUtils

// Find all the print functions by looking for TagInfo initializers
// like this one:
// https://github.com/Exiv2/exiv2/blob/6b186a4cd276ac11b3ea69951c2112f4c4814b9a/src/canonmn_int.cpp#L660-L679
class PrintFunction extends Function {
  PrintFunction() {
    exists(Initializer i, Field f |
      i.getExpr().(ArrayAggregateLiteral).getAChild().(ClassAggregateLiteral).getAFieldExpr(f) =
        this.getAnAccess() and
      f.getName() = "printFct_"
    )
  }
}

from PrintFunction f, Parameter p, Call call, Expr qualifier
where
  p = f.getParameter(2) and
  qualifier = p.getAnAccess() and
  call.getQualifier() = qualifier and
  // Don't complain if the access is protected by a null check.
  not exists(GuardCondition nonNullCheck, BasicBlock block, boolean branch |
    validCheckExpr(nonNullCheck, p) and
    nonNullCheck.controls(block, branch) and
    block.contains(call)
  )
select qualifier, "Print functions need to check that the metadata isn't null."
