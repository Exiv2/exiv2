// Utility for generating a fuzzing dictionary for Exiv2.
// See README.md (in this directory) for more information.

import cpp
import semmle.code.cpp.dataflow.DataFlow

predicate parser_string(string s, StringLiteral l) {
  s = l.getValue() and
  exists(FunctionCall call, string fcnName |
    DataFlow::localExprFlow(l, call.getAChild+()) and
    fcnName = call.getTarget().getName()
  |
    fcnName.matches("%cmp%") or
    fcnName.matches("%find%") or
    fcnName = "startsWith" or
    fcnName = "operator==" or
    fcnName = "operator!="
  )
}

from string s
where parser_string(s, _)
select s
