#ifndef AST_OPTIMIZER_INCLUDE_AST_DATATYPES_H_
#define AST_OPTIMIZER_INCLUDE_AST_DATATYPES_H_

#include <string>
#include <map>
#include "Node.h"

enum class Types {
  INT, FLOAT, STRING, BOOL
};

class Datatype : public Node {
 private:
  Node *createClonedNode(bool keepOriginalUniqueNodeId) override;

  Types val;
  bool encrypted{false};

 public:
  explicit Datatype(Types di) : val(di) {}

  explicit Datatype(Types di, bool isEncrypted) : val(di), encrypted(isEncrypted) {}

  explicit Datatype(const std::string &type);

  static std::string enumToString(Types identifiers);

  explicit operator std::string() const;

  explicit operator Types() const;

  [[nodiscard]] std::string toString() const override;

  bool operator==(const Datatype &rhs) const;

  bool operator!=(const Datatype &rhs) const;

  [[nodiscard]] Types getType() const;

  [[nodiscard]] bool isEncrypted() const;

  void setEncrypted(bool encrypted);
};

#endif //AST_OPTIMIZER_INCLUDE_AST_DATATYPES_H_
