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

predicate metadataDeref(Expr metadata) {
  exists(Call call | call.getQualifier() = metadata)
  or
  exists(FunctionCall call, int argIndex, Function f |
    call.getArgument(argIndex) = metadata and
    f = call.getTarget() and
    metadataDeref(f.getParameter(argIndex).getAnAccess())
  )
}

predicate unsafePointerParam(Function f, int paramIndex, Expr use) {
  exists(Parameter p |
    p = f.getParameter(paramIndex) and
    use = p.getAnAccess() and
    unsafePointerExpr(use) and
    not exists(GuardCondition nonNullCheck, BasicBlock block, boolean branch |
      validCheckExpr(nonNullCheck, p) and
      nonNullCheck.controls(block, branch) and
      block.contains(use)
    )
  )
}

predicate unsafePointerExpr(Expr e) {
  exists(Call call |
    call.getQualifier() = e and
    e.getType().getUnspecifiedType() instanceof PointerType
  )
  or
  exists(FunctionCall call, int argIndex, Function f |
    call.getArgument(argIndex) = e and
    f = call.getTarget() and
    unsafePointerParam(f, argIndex, _)
  )
}

from PrintFunction printfcn, Parameter p, Expr metadata
where unsafePointerParam(printfcn, 2, metadata)
select metadata, "Print functions need to check that the metadata isn't null."
