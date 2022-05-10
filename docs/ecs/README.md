# entity-component-system #

## integrating to you code ##

Simply include file ecs/api.hpp to your code.

## registry ##

To use entity-component-system you must create a registry. Registry is an entity manager, that provides all functions
to manage entities and they components. But you must not call functions of registry directly. You must only create a
registry instance. You can use CreateEntity, GetView functions and EntityHandle, View, View::Iterator classes to 
manage entities.

## entity ##

To create entity you must call CreateEntity function and pass registry to it. CreateEntity return you EntityHandle.
EntityHandle is a non owning handle to entity that provides all functions to manage entity.

```c++
class EntityHandle {
 public:
  /**
   * @brief Destory entity and its components.
   */
  void Destroy();

  /**
   * @brief Add component of ComponentT type to entity if entity does not have component of ComponentT type.
   */ 
  template <typename ComponentT, typename... Args>
  void AddComponent(Args&&... args);

  /**
   * @brief Remove component of ComponentT type from entity if entity have component of ComponentT type.
   */ 
  template <typename ComponentTy>
  void RemoveComponent();

  /**
   * @brief Return pointer to component of ComponentT type if entity have component of ComponentT type.
   */ 
  template <typename ComponentT>
  ComponentT* GetComponent();

  /**
   * @brief Checks if an entity has a component of ComponentT type.
   */ 
  template <typename ComponentT>
  bool HasComponent();
};

```

## view ##

To get pool of entites with component of ComponentT type you must call GetView function and pass registry to it.
GetView return you view on this pool. You can iterate through this pool and do any work on components.

```c++
template <typename ComponentT>
class View {
 public:
  /**
   * @brief Return iterator to begin of pool of entites with component of ComponentT type.
   */
  Iterator Begin();

  /**
   * @brief Return iterator to end of pool of entites with component of ComponentT type.
   */
  Iterator End();
};
```

```c++
class Iterator {
 public:
  /**
   * @brief Return pointer to component.
   */
  ComponentT* GetComponent() const;

  /**
   * @brief Return handle to entity.
   */
  EntityHandle GetEntityHandle() const;
}
```

## example of usage ##

```c++
#include "ecs/api.hpp"

#include <iostream>

class Component {
 public:
  Component(int field) : field_{field} {
  }

  int GetField() const {
    return field_;
  }

  void SetField(int field) {
    field_ = field;
  }

 private:
  int field_;
};

int main() {
  Registry registry{};

  EntityHandle h1 = CreateEntity(registry);
  h1.AddComponent<Component>(1);
  EntityHandle h2 = CreateEntity(registry);
  h2.AddComponent<Component>(2);
  EntityHandle h3 = CreateEntity(registry);
  h3.AddComponent<Component>(3);

  auto view = GetView<Component>(registry);
  for (auto iter = view.Begin(); iter != view.End(); ++iter) {
    std::cout << iter.GetComponent()->GetField() << std::endl;
  }

  h1.Destroy();
  h2.Destroy();
  h3.Destroy();

  return 0;
}
```
