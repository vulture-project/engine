#include "scene_node.hpp"

class Scene {
  public:
    Scene(ecs::Entity* root_entity) : root{new SceneNode{root_entity}} {}
  public:
    SceneNode* root = nullptr;
    ecs::Entity* camera = nullptr;
    Vector<ecs::Entity*> light_sources;
};