#ifndef AST_OPTIMIZER_INCLUDE_AST_OPT_VISITOR_COMPILETIMEEXPRESSIONSIMPLIFIER_H_
#define AST_OPTIMIZER_INCLUDE_AST_OPT_VISITOR_COMPILETIMEEXPRESSIONSIMPLIFIER_H_

#include "ast_opt/utilities/Visitor.h"

class SpecialCompileTimeExpressionSimplifier;

typedef Visitor<SpecialCompileTimeExpressionSimplifier, ScopedVisitor> CompileTimeExpressionSimplifier;

class SpecialCompileTimeExpressionSimplifier : public ScopedVisitor {

};

#endif //AST_OPTIMIZER_INCLUDE_AST_OPT_VISITOR_COMPILETIMEEXPRESSIONSIMPLIFIER_H_
