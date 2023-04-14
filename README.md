# Vulture Engine
This is a small WIP data-oriented game engine, being developed for mostly educational and experimental purposes. Currently, Vulture Engine is in its earliest steps of development, but our team is working hard on researching and analyzing existing practices in game engine development in order to build a modern engine, full of features and easy to use, not sacrificing the performance.

![editor screenshot](docs/editor_screenshot.png)

## Roadmap
### Implemented features
- [:heavy_check_mark:] Basic 3D OpenGL deferred rendering
- [:heavy_check_mark:] [fennecs](https://github.com/elisfromkirov/fennecs) - an ECS framework written by a member of our team
- [:heavy_check_mark:] Basic scene editor
- [:heavy_check_mark:] Event system
- [:heavy_check_mark:] Logger

### WIP
- [:gear:] Rewrite the entire renderer
  - Vulkan render device
  - Render Graph abstraction
  - Material system
  - Our own shader language
  - PBR shaders
- [:gear:] Multi-threaded architecture
- [:gear:] Scene editor
- [:gear:] Asset management
- [:gear:] Skeletal animation
- [:gear:] Runtime and build system
- [:gear:] Script System
- [:gear:] Physics
- [:gear:] Audio

## Dependencies
| Name                                         | Notes                                                                 |
|----------------------------------------------|-----------------------------------------------------------------------|
| [glm](https://github.com/g-truc/glm)         | Submodule                                                             |
| [glfw](https://github.com/glfw/glfw)         | Submodule                                                             |
| [glad](https://glad.dav1d.de/)               | Probably will be deprecated as soon as Vulkan renderer is implemented |
| [stb_image](https://github.com/nothings/stb) | -                                                                     |
| [fmt](https://github.com/fmtlib/fmt)         | Submodule, used for logging                                           |
| [fennecs](https://github.com/elisfromkirov/fennecs) | Yet another entity component system framework, written specifically for Vulture Engine |
| [imgui](https://github.com/ocornut/imgui)    | -                                                                     |

## Building
```
$ git clone --recursive https://github.com/vulture-project/engine.git
$ git lfs pull
$ sh ./build.sh {Debug|Release}
$ ./veditor
```
