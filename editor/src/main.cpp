#include <veditor/editor_app.hpp>

int main(int, char**) {
  vulture::EditorApp app;
  app.Init();
  app.Run();

  return 0;
}
