#ifndef WORLD_IMPL
#error Do not include this file directly
#endif

#include <iostream>

namespace fennecs {

// 1. Check whether component contains in entity.
// 2. Extract entity from the array where it is.
// 3. Find appropriate new array.
// 4. Allocate memory for new entity.
// 5. Move component from entity to new entity.
// 6. Instantiate additional component.
// 7. Insert entity to new array.
// 8. Destroy entity's components.
// 9. Deallocate entity's memory.
// 10. Return handle for new entity.
template <typename Component, typename... ArgTypes>
EntityHandle EntityWorld::Attach(EntityHandle handle, ArgTypes&& ... args) {
  if (handle.template Has<Component>()) {
    return handle;
  }

  handle.entity_->Unlink();

  EntityArray* array = FindArrayOnAttach<Component>(handle);

  Entity* entity = entity_allocator_.Allocate(array->Layout());

  Move(*handle.archetype_,
       *handle.layout_,
       handle.entity_,
       array->Archetype(),
       array->Layout(),
       entity);

  Uint8* place = entity->Data() + array->Layout().template OffsetOf<Component>();
  new(place) Component(::std::forward<ArgTypes>(args)...);

  array->Insert(entity);

  Destroy(*handle.archetype_, *handle.layout_, handle.entity_);

  entity_allocator_.Deallocate(handle.entity_);

  return EntityHandle{&array->Archetype(), &array->Layout(), entity};
}

template <typename Component>
EntityHandle EntityWorld::Detach(EntityHandle handle) {
  FENNECS_ASSERT(false, "TODO: Implement it!");

  return handle;
}

template <typename Component>
EntityArray* EntityWorld::FindArrayOnAttach(EntityHandle handle) {
  EntityArray* array = entity_registry_.FindArray(handle.archetype_->template Attach<Component>());
  if (array != nullptr) {
    return array;
  }

  if (!component_registry_.template HasVirtualTableOf<Component>()) {
    component_registry_.template RegisterVirtualTableOf<Component>();
  }

  return entity_registry_.AddArray(handle.archetype_->template Attach<Component>(),
                                   handle.layout_->template Attach<Component>());
}

template <typename Component>
EntityArray* EntityWorld::FindArrayOnDetach(EntityHandle handle) {
  EntityArray* array = entity_registry_.FindArray(handle.archetype_->template Detach<Component>());
  if (array != nullptr) {
    return array;
  }

  return entity_registry_.AddArray(handle.archetype_->template Detach<Component>(),
                                   handle.layout_->template Detach<Component>());
}

template <typename... Components>
EntityStream EntityWorld::Query() {
  return EntityStream{EntityArchetype::ConsistsOf<Components...>(),
                      entity_registry_.Head(),
                      entity_registry_.Tail()};
}

}  // namespace fennecs