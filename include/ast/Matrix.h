#ifndef AST_OPTIMIZER_INCLUDE_AST_MATRIX_H_
#define AST_OPTIMIZER_INCLUDE_AST_MATRIX_H_

#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "AbstractMatrix.h"
#include "Dimension.h"
#include "Operator.h"

using json = nlohmann::json;

template<typename T>
static Matrix<T> *applyComponentwise(Matrix<T> *A, Matrix<T> *B, std::function<T(T, T)> f) {
  // check that A and B have valid dimensions:
  // - if A and B have same dimensions -> apply f componentwise
  // - if either A or B is a scalar -> apply scalar on each component of matrix/vector
  bool isScalarA = A->getDimensions().equals(1, 1);
  bool isScalarB = B->getDimensions().equals(1, 1);
  if (A->getDimensions()!=B->getDimensions() && !(isScalarA ^ isScalarB)) {
    throw std::logic_error("Matrices A and B must have the same dimensions to apply an operator componentwise, or "
                           "either one of A and B must be a scalar to apply the scalar product.");
  } else if (isScalarA && !isScalarB) {
    A->expandAndFillMatrix(B->getDimensions(), A->getScalarValue());
  } else if (isScalarB && !isScalarA) {
    B->expandAndFillMatrix(A->getDimensions(), B->getScalarValue());
  }

  // compute new matrix by applying function f componentwise, e.g., f(a11, b11), f(a12, b12), ..., f(aNM, bNM)
  std::vector<std::vector<T>> result;
  for (int i = 0; i < A->values.size(); ++i) {
    result.push_back(std::vector<T>());
    for (int j = 0; j < A->values[0].size(); ++j) {
      result[i].push_back(f((*A)(i, j), (*B)(i, j)));
    }
  }
  return new Matrix<T>(result);
}

template<typename T>
static Matrix<T> *applyOnEachElement(Matrix<T> *A, std::function<T(T)> f) {
  // compute new matrix by applying function f on each element of A, e.g., f(a11), f(a12), ..., f(aNM)
  std::vector<std::vector<T>> result;
  for (int i = 0; i < A->values.size(); ++i) {
    result.push_back(std::vector<T>());
    for (int j = 0; j < A->values[0].size(); ++j) {
      result[i].push_back(f((*A)(i, j)));
    }
  }
  return new Matrix<T>(result);
}

template<typename T>
static Matrix<T> *applyMatrixMultiplication(Matrix<T> *matrixA, Matrix<T> *matrixB) {
  // check that #column of matrixA equals #rows of matrixB
  if (!matrixA->getDimensions().equals(-1, matrixB->getDimensions().numRows)) {
    std::stringstream ss;
    ss << "matrixA has dimensions " << matrixA->getDimensions() << " and matrixB has dimensions "
       << matrixB->getDimensions() << ". ";
    ss << "To apply matrix multiplication, #columns of matrixA must be equal to the #rows of matrixB." << std::endl;
    throw std::logic_error(ss.str());
  }

  // define result matrix having dimension (#rowsA, #columnsB)
  std::vector<std::vector<T>> result(matrixA->getDimensions().numRows,
                                     std::vector<T>(matrixB->getDimensions().numColumns));

  // matrixA has dim (M,N), matrixB has dim (N,P)
  int dimM = matrixA->getDimensions().numRows;
  int dimN = matrixA->getDimensions().numColumns;
  int dimP = matrixB->getDimensions().numColumns;
  T sum = 0;
  // Very inefficient: O(n^3)! Replace it, for example, by Strassen's multiplication algorithm.
  for (int k = 0; k < dimN; ++k) {
    for (int i = 0; i < dimM; ++i) {
      for (int j = 0; j < dimP; ++j) {
        result[i][j] += (*matrixA)(i, k)*(*matrixB)(k, j);
      }
    }
  }
  return new Matrix<T>(result);
}

template<typename T>
class Matrix : public AbstractMatrix {
 private:
  template<typename Iterator>
  Iterator computeRotationTarget(Iterator itBegin, size_t vectorSize, int rotationFactor) {
    return (rotationFactor > 0) ? (itBegin + vectorSize - rotationFactor) : (itBegin - rotationFactor);
  }

 public:
  // a matrix of row vectors
  std::vector<std::vector<T>> values;

  Dimension dim;

  Matrix(std::vector<std::vector<T>> inputMatrix)  /* NOLINT intentionally not explicit */
      : values(std::move(inputMatrix)), dim(Dimension(values.size(), values.at(0).size())) {
    int elementsPerRow = values.at(0).size();
    for (auto const &rowVector : values) {
      if (rowVector.size()!=elementsPerRow) {
        throw std::invalid_argument("Vector rows must all have the same number of elements!");
      }
    }
  }

  Matrix(T scalarValue) : dim({1, 1}) {  /* NOLINT intentionally not explicit */
    values = {{scalarValue}};
  }

  Matrix(Matrix<T> &other) : dim(Dimension(other.getDimensions().numRows, other.getDimensions().numColumns)) {
    values = other.values;
  }

  [[nodiscard]] bool isScalar() const override {
    return dim.equals(1, 1);
  }

  T getScalarValue() const {
    if (isScalar()) return values[0][0];
    throw std::logic_error("getScalarValue() not allowed on non-scalars!");
  }

  bool operator==(const Matrix &rhs) const {
    return values==rhs.values && dim==rhs.dim;
  }

  bool operator!=(const Matrix &rhs) const {
    return !(rhs==*this);
  }

  [[nodiscard]] json toJson() const override {
    // return the scalar value if this is a (1,1) scalar matrix
    if (isScalar()) return json(getScalarValue());
    // if this is a matrix, return an array of arrays like [ [a00, b01, c02], [d10, e11, f12] ]
    json arrayOfArrays = json::array();
    for (int i = 0; i < values.size(); ++i) {
      arrayOfArrays.push_back(json(values[i]));
    }
    return arrayOfArrays;
  }

  // Returning std::vector<T>::reference is required here.
  // Credits to Mike Seymour from stackoverflow.com (https://stackoverflow.com/a/25770060/3017719).
  typename std::vector<T>::reference operator()(int row, int column) {
    if (!dim.isValidAccess(row, column)) {
      std::stringstream ss;
      ss << "Cannot access " << Dimension(row, column) << " because vector has dimensions " << getDimensions() << ".";
      throw std::invalid_argument(ss.str());
    }
    return values[row][column];
  }

  T getElement(int row, int column) {
    return values[row][column];
  }

  Dimension &getDimensions() override {
    return dim;
  }

  void setValues(const std::vector<std::vector<T>> &newValues) {
    values = newValues;
  }

  std::string toString() override {
    std::stringstream outputStr;
    // print boolean values as text (true, false) by default, otherwise the output (0,1) cannot be distinguished from
    // the output generated by integers
    outputStr << std::boolalpha;
    // return the scalar value as string
    if (isScalar()) {
      outputStr << getScalarValue();
      return outputStr.str();
    }
    // use MATLAB' matrice style for matrices, e.g., for a 3x3 matrix: [2 2 33; 3 1 1; 3 11 9]
    const std::string elementDelimiter = " ";
    const std::string rowDelimiter = "; ";
    outputStr << "[";
    for (int i = 0; i < values.size(); ++i) {
      for (int j = 0; j < values[i].size(); ++j) {
        outputStr << values[i][j];
        if (j!=values[i].size() - 1) outputStr << elementDelimiter;
      }
      if (i!=values.size() - 1) outputStr << rowDelimiter;
    }
    outputStr << "]";
    return outputStr.str();
  }

  bool allValuesEqual(T valueToBeComparedWith) {
    for (int i = 0; i < values.size(); ++i) {
      for (int j = 0; j < values[i].size(); ++j) {
        if (values[i][j]!=valueToBeComparedWith) return false;
      }
    }
    return true;
  }

  Matrix<T> *transpose(bool inPlace) {
    Matrix<T> *matrixToTranspose = inPlace ? this : new Matrix<T>(*this);
    std::vector<std::vector<T>> transposedVec(matrixToTranspose->values[0].size(), std::vector<T>());
    for (int i = 0; i < matrixToTranspose->values.size(); ++i) {
      for (int j = 0; j < matrixToTranspose->values[i].size(); ++j) {
        transposedVec[j].push_back(matrixToTranspose->values[i][j]);
      }
    }
    matrixToTranspose->getDimensions().update(matrixToTranspose->values[0].size(), matrixToTranspose->values.size());
    matrixToTranspose->values = transposedVec;
    return matrixToTranspose;
  }

  Matrix<T> *rotate(int rotationFactor, bool inPlace) override {
    Matrix<T> *matrixToRotate = inPlace ? this : new Matrix<T>(*this);
    if (matrixToRotate->getDimensions().equals(1, -1)) {  // a row vector
      auto &vec = matrixToRotate->values[0];
      std::rotate(vec.begin(), computeRotationTarget(vec.begin(), vec.size(), rotationFactor), vec.end());
    } else if (matrixToRotate->getDimensions().equals(-1, 1)) {  // a column vector
      // Transpose the vector, rotate it, transpose it again. This is needed because std::rotate requires all elements
      // in a single vector. As our matrix is represented using row vectors, we need to transform column->row vector.
      matrixToRotate->transpose(true);
      auto &vec = matrixToRotate->values[0];
      std::rotate(vec.begin(), computeRotationTarget(vec.begin(), vec.size(), rotationFactor), vec.end());
      matrixToRotate->transpose(true);
    } else {
      throw std::invalid_argument("Rotation only supported for 1-dimensional vectors.");
    }
    return matrixToRotate;
  }

  void expandAndFillMatrix(Dimension &targetDimension, T scalarValue) {
    values = std::vector<std::vector<T>>(targetDimension.numRows,
                                         std::vector<T>(targetDimension.numColumns, scalarValue));
    getDimensions().update(targetDimension.numRows, targetDimension.numColumns);
  }

  Matrix<T> *clone() {
    // call the Matrix's copy constructor
    return new Matrix<T>(*this);
  }

  AbstractMatrix *applyBinaryOperator(AbstractMatrix *rhsOperand, Operator *os) override {
    return applyBinaryOperatorComponentwise(dynamic_cast<Matrix<T> *>(rhsOperand), os);
  }

  AbstractMatrix *applyBinaryOperatorComponentwise(Matrix<T> *rhsOperand, Operator *os);

  AbstractMatrix *applyUnaryOperator(Operator *os) override {
    return applyUnaryOperatorComponentwise(os);
  }

  AbstractMatrix *applyUnaryOperatorComponentwise(Operator *os);
};

#endif //AST_OPTIMIZER_INCLUDE_AST_MATRIX_H_
