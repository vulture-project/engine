#pragma once

#include <fennecs/core/assert.hpp>

namespace fennecs {

/**
 * @breif Base class for all intrusive linked objects.
 */
template <typename Type>
class Node {
 public:
  /**
   * @brief Construct node that is not linked to any list.
   */
  Node();

  /**
   * @brief Construct node with given next node.
   */
  Node(Node* next, Node* prev);

  /**
   * @brief Returns next node.
   */
  [[nodiscard]] Node* Next();

  /**
   * @brief Returns previous node.
   */
  [[nodiscard]] Node* Prev();

  /**
   * @brief Casts intrusive node back to derived type.
   */
  [[nodiscard]] Type* AsContent();

  /**
   * @brief Checks whether node is in list.
   */
  [[nodiscard]] bool IsLinked() const;

  /**
   * @brief Links this node after given node.
   */
  void LinkAfter(Node* node);

  /**
   * @brief Links this node before given node.
   */
  void LinkBefore(Node* node);

  /**
   * @brief Unlinks node from list.
   */
  void Unlink();

 private:
  /**
   * @brief Checks internal class invariants.
   */
  [[nodiscard]] bool IsValid() const;

 private:
  Node* next_;
  Node* prev_;
};

}  // namespace fennecs

#define NODE_IMPL
#include <fennecs/intrusive/node.ipp>
#undef NODE_IMPL