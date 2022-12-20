#include <fennecs/entity/registry.hpp>

namespace fennecs {

Node<EntityArray>* EntityRegistry::Head() {
  return arrays_.Head();
}

Node<EntityArray>* EntityRegistry::Tail() {
  return arrays_.Tail();
}

EntityArray* EntityRegistry::AddArray(const EntityArchetype& archetype,
                                  const EntityLayout& layout) {
  EntityArray* array = new EntityArray(archetype, layout);
  arrays_.Insert(array);
  return array;
}

EntityArray* EntityRegistry::FindArray(const EntityArchetype& archetype) {
  for (Node<EntityArray>* node = arrays_.Head(); node != arrays_.Tail(); node = node->Next()) {
    EntityArray* array = node->AsContent();
    if (archetype.Equals(array->Archetype())) {
       return array;
    }
  }
  return nullptr;
}

}  // namespace fennecs