#ifndef LIST_IMPL
#error Do not include this file directly
#endif

namespace fennecs {

template <typename Type>
IntrusiveList<Type>::IntrusiveList()
    : node_{} {
}

template <typename Type>
bool IntrusiveList<Type>::IsEmpty() const {
  return !node_.IsLinked();
}

template <typename Type>
Node<Type>* IntrusiveList<Type>::Head() {
  return node_.Next();
}

template <typename Type>
Node<Type>* IntrusiveList<Type>::Tail() {
  return &node_;
}

template <typename Type>
void IntrusiveList<Type>::Insert(Node<Type>* node) {
  node_.LinkBefore(node);
}

}  // namespace fennecs