#include <cassert>

#include "ECS/entity.h"
#include "renderer/core.hpp"

class SceneNode {
  public:
    SceneNode(ecs::Entity* entity) : entity_{entity} {}
    void AddChild(SceneNode* node) {
        children.push_back(node);
    }
    void DeleteChild(ecs::Entity::EntityId id) {
        for (auto it = children.begin(); it != children.end(); ++it) {
            if ((*it)->entity_->GetEntityId() == id) {
                children.erase(it);
                return;
            }
        }
        assert(0);
    }
    ecs::Entity* GetEntity() {
        return entity_;
    }
  public:
    List<SceneNode*> children;
  private:
    ecs::Entity* entity_;
};