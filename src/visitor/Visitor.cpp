#include "../../include/visitor/Visitor.h"
#include "Ast.h"
#include <Function.h>
#include <Operator.h>
#include <If.h>
#include <LiteralInt.h>
#include <LiteralBool.h>
#include <LogicalExpr.h>
#include <VarAssignm.h>
#include <Group.h>
#include <Block.h>
#include <Return.h>
#include <BinaryExpr.h>
#include <CallExternal.h>
#include <While.h>
#include <UnaryExpr.h>
#include <Call.h>
#include <Class.h>
#include "../utilities/Scope.h"

void Visitor::visit(Ast &elem) {
    // assumption: AST is always the enclosing object that points to the root
    this->currentScope = new Scope("global", nullptr);
    elem.getRootNode()->accept(*this);
}

void Visitor::visit(BinaryExpr &elem) {
    elem.getLeft()->accept(*this);
    elem.getOp().accept(*this);
    elem.getRight()->accept(*this);
}

void Visitor::visit(Block &elem) {
    currentScope->addStatement(&elem);
    changeToInnerScope(elem.getUniqueNodeId());
    for (auto &stat : *elem.getStatements()) {
        stat->accept(*this);
    }
    changeToOuterScope();
}

void Visitor::visit(Call &elem) {
    currentScope->addStatement(&elem);
    changeToInnerScope(elem.getUniqueNodeId());
    // callee
    elem.getCallee()->accept(*this);
    changeToOuterScope();
    // arguments
    for (auto arg : elem.getArguments()) {
        arg->accept(*this);
    }
}

void Visitor::visit(CallExternal &elem) {
    currentScope->addStatement(&elem);
    // arguments for calling function
    if (elem.getArguments() != nullptr) {
        for (auto &fp : *elem.getArguments()) {
            fp.accept(*this);
        }
    }
}

void Visitor::visit(Class &elem) {
    currentScope->addStatement(&elem);
    // functions
    // TODO Think if it makes sense to represent classes at all because this output does not represent the execution
    // flow; maybe it's enough to have a list of function names in a Class?
    for (Function f : elem.getMethods()) {
        f.accept(*this);
    }
}

void Visitor::visit(Function &elem) {
    currentScope->addStatement(&elem);
    changeToInnerScope(elem.getUniqueNodeId());
    // visit FunctionParameter
    for (auto fp : elem.getParams()) {
        fp.accept(*this);
    }
    // visit Body statements
    for (auto &stmt : elem.getBody()) {
        stmt->accept(*this);
    }
    changeToOuterScope();
}

void Visitor::visit(FunctionParameter &elem) {

}

void Visitor::visit(Group &elem) {
    elem.getExpr()->accept(*this);
}

void Visitor::visit(If &elem) {
    currentScope->addStatement(&elem);

    // condition
    elem.getCondition()->accept(*this);

    // if the user supplied only a single AbstractStatement as then- or else-branch, instead of a Block
    // consisting of multiple commands, we need in the following manually open a new scope

    // thenBranch
    if (auto *thenBranch = dynamic_cast<Block *>(elem.getThenBranch())) {
        elem.getThenBranch()->accept(*this);
    } else {
        // if thenBranch is no Block we need to manually open a new scope here
        Node *thenNode = dynamic_cast<Node *>(elem.getThenBranch());
        assert(thenNode != nullptr); // this should never happen
        changeToInnerScope(thenNode->getUniqueNodeId());
        elem.getThenBranch()->accept(*this);
        changeToOuterScope();
    }

    if (elem.getElseBranch() != NULL) {
        // elseBranch
        if (auto *elseBranch = dynamic_cast<Node *>(elem.getElseBranch())) {
            elem.getElseBranch()->accept(*this);
        } else {
            Node *elseNode = dynamic_cast<Node *>(elem.getElseBranch());
            assert(elseNode != nullptr);
            changeToInnerScope(elseBranch->getUniqueNodeId());
            elem.getElseBranch()->accept(*this);
            changeToOuterScope();
        }
    }
}

void Visitor::visit(LiteralBool &elem) {

}

void Visitor::visit(LiteralInt &elem) {

}

void Visitor::visit(LiteralString &elem) {

}

void Visitor::visit(LogicalExpr &elem) {
    // left
    elem.getLeft()->accept(*this);
    // operator
    elem.getOp()->accept(*this);
    // right
    elem.getRight()->accept(*this);
}

void Visitor::visit(Operator &elem) {

}

void Visitor::visit(Return &elem) {
    currentScope->addStatement(&elem);
    elem.getValue()->accept(*this);
}

void Visitor::visit(UnaryExpr &elem) {
    // operator
    elem.getOp()->accept(*this);
    // rhs operand
    elem.getRight()->accept(*this);
}

void Visitor::visit(VarAssignm &elem) {
    currentScope->addStatement(&elem);
    elem.getValue()->accept(*this);
}

void Visitor::visit(VarDecl &elem) {
    currentScope->addStatement(&elem);
    // visit initializer
    if (elem.getInitializer() != nullptr) {
        elem.getInitializer()->accept(*this);
    }
}

void Visitor::visit(Variable &elem) {

}

void Visitor::visit(While &elem) {
    currentScope->addStatement(&elem);

    // condition
    elem.getCondition()->accept(*this);

    // then-block
    // if statements following While are nested in a Block, a new scope will be created automatically;
    // if only a single statement is following, we manually need to open a new scope
    if (auto *thenBlock = dynamic_cast<Block *>(elem.getBody())) {
        elem.getBody()->accept(*this);
    } else {
        Node *block = dynamic_cast<Node *>(elem.getBody());
        assert(block != nullptr);
        changeToInnerScope(block->getUniqueNodeId());
        elem.getBody()->accept(*this);
        changeToOuterScope();
    }
}

void Visitor::changeToOuterScope() {
    auto temp = currentScope->getOuterScope();
    this->currentScope = temp;
}

void Visitor::changeToInnerScope(const std::string &nodeId) {
    auto temp = currentScope->getOrCreateInnerScope(nodeId);
    this->currentScope = temp;
}
