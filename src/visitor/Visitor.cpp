#include "Visitor.h"
#include "Ast.h"
#include "AbstractNode.h"
#include "AbstractExpr.h"
#include "AbstractStatement.h"
#include "BinaryExpr.h"
#include "Block.h"
#include "Call.h"
#include "CallExternal.h"
#include "Function.h"
#include "If.h"
#include "LiteralBool.h"
#include "LiteralInt.h"
#include "LiteralFloat.h"
#include "LogicalExpr.h"
#include "Operator.h"
#include "Return.h"
#include "Scope.h"
#include "UnaryExpr.h"
#include "VarAssignm.h"
#include "While.h"

void Visitor::visit(Ast &elem) {
  // assumption: AST is always the enclosing object that points to the root
  this->curScope = new Scope("global", nullptr);
  elem.getRootNode()->accept(*this);
}

void Visitor::visit(AbstractNode &elem) {
  auto children = elem.getChildren();
  for(auto &c : children) {
    c->accept(*this);
  }
}

void Visitor::visit(AbstractExpr &elem) {
  auto children = elem.getChildren();
  for(auto &c : children) {
    c->accept(*this);
  }
}

void Visitor::visit(AbstractStatement &elem) {
  auto children = elem.getChildren();
  for(auto &c : children) {
    c->accept(*this);
  }
}

void Visitor::visit(BinaryExpr &elem) {
  elem.getLeft()->accept(*this);
  elem.getOp()->accept(*this);
  elem.getRight()->accept(*this);
}

void Visitor::visit(Block &elem) {
  curScope->addStatement(&elem);
  changeToInnerScope(elem.getUniqueNodeId());
  for (auto &stat : *elem.getStatements()) {
    stat->accept(*this);
  }
  changeToOuterScope();
}

void Visitor::visit(Call &elem) {
  curScope->addStatement(&elem);
  changeToInnerScope(elem.AbstractStatement::getUniqueNodeId());
  // callee
  elem.getFunc()->accept(*this);
  changeToOuterScope();
  // arguments
  for (auto arg : elem.getArguments()) {
    arg->accept(*this);
  }
}

void Visitor::visit(CallExternal &elem) {
  curScope->addStatement(&elem);
  // arguments for calling function
  if (!elem.getArguments().empty()) {
    for (auto &fp : elem.getArguments()) {
      fp->accept(*this);
    }
  }
}

void Visitor::visit(Function &elem) {
  curScope->addStatement(&elem);
  changeToInnerScope(elem.getUniqueNodeId());
  // visit FunctionParameter
  for (auto fp : elem.getParams()) {
    fp->accept(*this);
  }
  // visit Body statements
  for (auto &stmt : elem.getBody()) {
    stmt->accept(*this);
  }
  changeToOuterScope();
}

void Visitor::visit(FunctionParameter &elem) {
  elem.getValue()->accept(*this);
}

void Visitor::visit(If &elem) {
  curScope->addStatement(&elem);

  // condition
  elem.getCondition()->accept(*this);

  // if the user supplied only a single AbstractStatement as then- or else-branch, instead of a Block
  // consisting of multiple commands, we need in the following manually open a new scope

  // thenBranch
  if (auto *thenBranch = dynamic_cast<Block *>(elem.getThenBranch())) {
    thenBranch->accept(*this);
  } else {
    // if thenBranch is no Block we need to manually open a new scope here
    AbstractNode *thenNode = dynamic_cast<AbstractNode *>(elem.getThenBranch());
    assert(thenNode != nullptr); // this should never happen
    changeToInnerScope(thenNode->getUniqueNodeId());
    elem.getThenBranch()->accept(*this);
    changeToOuterScope();
  }

  if (elem.getElseBranch() != nullptr) {
    // elseBranch
    if (auto *elseBranch = dynamic_cast<AbstractNode *>(elem.getElseBranch())) {
      elem.getElseBranch()->accept(*this);
    } else {
      AbstractNode *elseNode = dynamic_cast<AbstractNode *>(elem.getElseBranch());
      assert(elseNode != nullptr);
      changeToInnerScope(elseBranch->getUniqueNodeId());
      elem.getElseBranch()->accept(*this);
      changeToOuterScope();
    }
  }
}

void Visitor::visit(LiteralBool &elem) {}

void Visitor::visit(LiteralInt &elem) {}

void Visitor::visit(LiteralString &elem) {}

void Visitor::visit(LiteralFloat &elem) {}

void Visitor::visit(LogicalExpr &elem) {
  // left
  elem.getLeft()->accept(*this);
  // operator
  elem.getOp()->accept(*this);
  // right
  elem.getRight()->accept(*this);
}

void Visitor::visit(Operator &elem) {}

void Visitor::visit(Return &elem) {
  curScope->addStatement(&elem);
  for (auto &expr : elem.getReturnExpressions()) expr->accept(*this);
}

void Visitor::visit(UnaryExpr &elem) {
  // operator
  elem.getOp()->accept(*this);
  // rhs operand
  elem.getRight()->accept(*this);
}

void Visitor::visit(VarAssignm &elem) {
  curScope->addStatement(&elem);
  elem.getValue()->accept(*this);
}

void Visitor::visit(VarDecl &elem) {
  curScope->addStatement(&elem);
  // visit initializer
  if (elem.getInitializer() != nullptr) {
    elem.getInitializer()->accept(*this);
  }
}

void Visitor::visit(Variable &elem) {}

void Visitor::visit(While &elem) {
  curScope->addStatement(&elem);

  // condition
  elem.getCondition()->accept(*this);

  // then-block
  // if statements following While are nested in a Block, a new scope will be created automatically;
  // if only a single statement is following, we manually need to open a new scope
  if (auto *thenBlock = dynamic_cast<Block *>(elem.getBody())) {
    thenBlock->accept(*this);
  } else {
    AbstractNode *block = dynamic_cast<AbstractNode *>(elem.getBody());
    assert(block != nullptr);
    changeToInnerScope(block->getUniqueNodeId());
    elem.getBody()->accept(*this);
    changeToOuterScope();
  }
}

void Visitor::changeToOuterScope() {
  auto temp = curScope->getOuterScope();
  this->curScope = temp;
}

void Visitor::changeToInnerScope(const std::string &nodeId) {
  auto temp = curScope->getOrCreateInnerScope(nodeId);
  this->curScope = temp;
}

Visitor::Visitor() {
  curScope = nullptr;
}

