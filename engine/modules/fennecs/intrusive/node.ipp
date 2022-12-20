#ifndef NODE_IMPL
#error Do not include this file directly
#endif

namespace fennecs {

template <typename Type>
Node<Type>::Node()
    : next_{this},
      prev_{this} {
}

template <typename Type>
Node<Type>::Node(Node* next, Node* prev)
    : next_{next},
      prev_{prev} {
}

template <typename Type>
Node<Type>* Node<Type>::Next() {
  FENNECS_ASSERT(IsValid(), "Internal FENNECS_ASSERTion failure");

  return next_;
}

template <typename Type>
Node<Type>* Node<Type>::Prev() {
  FENNECS_ASSERT(IsValid(), "Internal FENNECS_ASSERTion failure");

  return prev_;
}

template <typename Type>
Type* Node<Type>::AsContent() {
  return reinterpret_cast<Type*>(this);
}

template <typename Type>
bool Node<Type>::IsLinked() const {
  FENNECS_ASSERT(IsValid(), "Internal FENNECS_ASSERTion failure");

  return !(prev_ == this && next_ == this);
}

template <typename Type>
void Node<Type>::LinkAfter(Node* node) {
  FENNECS_ASSERT(IsValid(), "Internal FENNECS_ASSERTion failure");
  FENNECS_ASSERT(node != nullptr, "Node must be valid pointer");
  FENNECS_ASSERT(!node->IsLinked(), "Unable to link already linked node");

  next_->prev_ = node;
  node->next_ = next_;
  next_ = node;
  node->prev_ = this;
}

template <typename Type>
void Node<Type>::LinkBefore(Node* node) {
  FENNECS_ASSERT(IsValid(), "Internal FENNECS_ASSERTion failure");
  FENNECS_ASSERT(node != nullptr, "Node must be a valid pointer");
  FENNECS_ASSERT(!node->IsLinked(), "Unable to link already linked node");

  prev_->next_ = node;
  node->prev_ = prev_;
  prev_ = node;
  node->next_ = this;

  FENNECS_ASSERT(IsValid(), "Internal FENNECS_ASSERTion failure");
  FENNECS_ASSERT(node->IsValid(), "Internal FENNECS_ASSERTion failure");
}

template <typename Type>
void Node<Type>::Unlink() {
  FENNECS_ASSERT(IsValid(), "Internal FENNECS_ASSERTion failure");
  FENNECS_ASSERT(IsLinked(), "Unable to unlink already unlinked node");

  prev_->next_ = next_;
  next_->prev_ = prev_;
  prev_ = this;
  next_ = this;
}

template <typename Type>
bool Node<Type>::IsValid() const {
  return next_ != nullptr && prev_ != nullptr
      && ((prev_ == this && next_ == this) || (prev_ != this && next_ != this));
}

}  // namespace fennecs