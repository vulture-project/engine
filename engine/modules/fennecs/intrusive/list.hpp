#pragma once

#include <fennecs/intrusive/node.hpp>

namespace fennecs {

/**
 * @brief Intrusive list of memory chunks.
 */
template <typename Type>
class IntrusiveList {
 public:
  /**
   * @brief Constructs empty list.
   */
  IntrusiveList();

  /**
   * @brief Checkes whether list empty or not.
   */
   [[nodiscard]] bool IsEmpty() const;

  /**
   * @brief Returns head of list.
   */
  [[nodiscard]] Node<Type>* Head();

  /**
   * @brief Returns tail of list.
   */
  [[nodiscard]] Node<Type>* Tail();

  /**
   * @brief Insert node to the end of list.
   */
  void Insert(Node<Type>* node);

 private:
  Node<Type> node_;
};

}  // namespace fennecs

#define LIST_IMPL
#include <fennecs/intrusive/list.ipp>
#undef LIST_IMPL