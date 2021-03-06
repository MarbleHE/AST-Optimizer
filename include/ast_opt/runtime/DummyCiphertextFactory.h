#ifndef AST_OPTIMIZER_INCLUDE_AST_OPT_VISITOR_RUNTIME_DUMMYCIPHERTEXTFACTORY_H_
#define AST_OPTIMIZER_INCLUDE_AST_OPT_VISITOR_RUNTIME_DUMMYCIPHERTEXTFACTORY_H_

#include <memory>
#include "AbstractCiphertextFactory.h"
#include "Cleartext.h"

class DummyCiphertextFactory : public AbstractCiphertextFactory {
 public:
  /// Gets the number of ciphertext slots that each ciphertext generated by this factory consists of.
  /// \return The number of total slots in each ciphertext.
  [[nodiscard]] unsigned int getCiphertextSlotSize() const;

  /// Creates a new Cleartext that encodes the given data (vector value) using the defined encoder.
  /// \param value The values to be encoded into the new plaintext.
  /// \return (A std::unique_ptr) to the newly created Cleartext.
  std::unique_ptr<ICleartext> createPlaintext(const std::vector<int> &value) const;

  /// Creates a new Cleartext object that encodes the given value. Remaining slots in the plaintext are filled up
  /// with the last given value.
  /// \param value The values to be encoded in the plaintext.
  /// \return (A std::unique_ptr to) the Cleartext that encodes the given values.
  std::unique_ptr<ICleartext> createPlaintext(const std::vector<int64_t> &value) const;

  /// Creates a new Cleartext object that encodes the given value. Remaining slots in the plaintext are filled up
  /// with the same value.
  /// \param value The value to be encoded in the plaintext.
  /// \return (A std::unique_ptr to) the Cleartext that encodes the given value.
  std::unique_ptr<ICleartext> createPlaintext(int64_t value) const;

  std::unique_ptr<AbstractCiphertext> createCiphertext(const std::vector<int64_t> &data)  const override;

  std::unique_ptr<AbstractCiphertext> createCiphertext(int64_t data)  const override;

  void decryptCiphertext(AbstractCiphertext &abstractCiphertext, std::vector<int64_t> &ciphertextData)  const override;

  std::string getString(AbstractCiphertext &abstractCiphertext)  const override;

  std::unique_ptr<AbstractCiphertext> createCiphertext(std::unique_ptr<AbstractValue> &&abstractValue)  const override;

  std::unique_ptr<AbstractCiphertext> createCiphertext(const std::vector<int> &data)  const override;
};

#endif //AST_OPTIMIZER_INCLUDE_AST_OPT_VISITOR_RUNTIME_DUMMYCIPHERTEXTFACTORY_H_
