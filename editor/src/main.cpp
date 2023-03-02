#include "editor_app.hpp"

#include "asset/asset_loader_registrar.hpp"

int main(int, char**) {
  vulture::EditorApp app;
  app.Init();
  app.Run();

  return 0;
}
