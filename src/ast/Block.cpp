#include "Block.h"
#include <iostream>
#include "VarDecl.h"
#include "AbstractNode.h"

Block::Block() {
  statements = nullptr;
}

json Block::toJson() const {
  json j = {{"type",       getNodeName()},
            {"statements", *this->statements}};
  return j;
}

Block::Block(AbstractStatement *stat) {
  auto *vec = new std::vector<AbstractStatement *>;
  vec->emplace_back(stat);
  this->statements = vec;
}

Block::Block(std::vector<AbstractStatement *> *statements) {
  if (statements->empty()) {
    std::string errorMsg = "Block statement vector is empty!"
                           "If this is intended, use the parameter-less constructor instead.";
    throw std::logic_error(errorMsg);
  }
  this->statements = statements;
}

void Block::accept(Visitor &v) {
  v.visit(*this);
}

std::string Block::getNodeName() const {
  return "Block";
}

std::vector<AbstractStatement *> *Block::getStatements() const {
  return statements;
}

Block::~Block() {
  delete statements;
}

AbstractNode *Block::createClonedNode(bool keepOriginalUniqueNodeId) {
  auto clonedStatements = new std::vector<AbstractStatement *>();
  for (auto &statement : *this->getStatements()) {
    clonedStatements->push_back(statement->cloneRecursiveDeep(keepOriginalUniqueNodeId)->castTo<AbstractStatement>());
  }
  return new Block(clonedStatements);
}
