#include <fennecs/entity/stream.hpp>

namespace fennecs {

EntityHandle EntityStream::Next() {
  if (curr_entity_node_ == nullptr && curr_array_node_ == nullptr) {
    return EntityHandle::Null();
  }

  EntityArray* array = curr_array_node_->AsContent();
  Entity* entity = curr_entity_node_->AsContent();

  curr_entity_node_ = curr_entity_node_->Next();
  if (curr_entity_node_ == tail_entity_node_) {
    do {
      curr_array_node_ = curr_array_node_->Next();
    } while (!(curr_array_node_ == tail_array_node_ ||
              (curr_array_node_->AsContent()->Archetype().Is(archetype_) &&
               !curr_array_node_->AsContent()->IsEmpty())));

    if (curr_array_node_ == tail_array_node_) {
      curr_array_node_ = nullptr;
      curr_entity_node_ = nullptr;
    } else {
      curr_entity_node_ = curr_array_node_->AsContent()->Head();
      tail_entity_node_ = curr_array_node_->AsContent()->Tail();
    }
  }

  return EntityHandle{&array->Archetype(), &array->Layout(), entity};
}

EntityStream::EntityStream(const EntityArchetype& archetype,
                           Node<EntityArray>* head,
                           Node<EntityArray>* tail)
    : archetype_{archetype},
      curr_array_node_{head},
      tail_array_node_{tail},
      curr_entity_node_{nullptr},
      tail_entity_node_{nullptr} {
  while (!(curr_array_node_ == tail_array_node_ ||
          (curr_array_node_->AsContent()->Archetype().Is(archetype_) &&
           !curr_array_node_->AsContent()->IsEmpty()))) {
    curr_array_node_ = curr_array_node_->Next();
  }

  if (curr_array_node_ == tail_array_node_) {
    curr_array_node_ = nullptr;
    curr_entity_node_ = nullptr;
  } else {
    curr_entity_node_ = curr_array_node_->AsContent()->Head();
    tail_entity_node_ = curr_array_node_->AsContent()->Tail();
  }
}

}  // namespace fennecs
