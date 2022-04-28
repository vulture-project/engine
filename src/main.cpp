/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file main.cpp
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022
 */

#include <alloca.h>
#include <fstream>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer/renderer3d.hpp"
#include "renderer/scene.hpp"
#include "renderer/resource_loader.hpp"
#include "scene/scene.hpp"

Scene g_Scene;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    float speed = 0.5;

    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position += speed * g_Scene.camera->forward;
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }

    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position -= speed * g_Scene.camera->forward;
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }

    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position -= speed * glm::cross(g_Scene.camera->forward, glm::vec3{0, 1, 0});
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }

    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position += speed * glm::cross(g_Scene.camera->forward, glm::vec3{0, 1, 0});
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }

    if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position.y += 1;
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }

    if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position.y -= 1;
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }
}

void mouseMoveCallback(GLFWwindow* window, double newX, double newY)
{
    static bool skip = true;
    if (skip)
    {
        glfwSetCursorPos(window, 0, 0);
        skip = false;
        return;
    }

    float dx = -newX;
    float dy = -newY;

    g_Scene.camera->forward = glm::normalize(glm::rotate(glm::identity<glm::mat4>(), 0.001f * dx, glm::vec3{0, 1, 0}) * glm::vec4(g_Scene.camera->forward, 1));
    g_Scene.camera->forward.y += 0.001f * dy;
    g_Scene.camera->forward = glm::normalize(g_Scene.camera->forward);
    // g_Scene.camera->forward = glm::normalize(glm::rotate(glm::identity<glm::mat4>(), 0.001f * dy, glm::vec3{1, 0, 1}) * glm::vec4(g_Scene.camera->forward, 1));

    // prevX = newX;
    // prevY = newY;

    glfwSetCursorPos(window, 0, 0);
}

#include "platform/window.hpp"
#include "platform/event.hpp"

using namespace input;

void ProcessMoveEvent(Event* event)
{
    assert(event);

    static float prev_x = 0;
    static float prev_y = 0;

    float dx = prev_x - event->GetMove().x;
    float dy = prev_y - event->GetMove().y;
    std::cout << dx << ' ' << dy << '\n';

    g_Scene.camera->forward = glm::normalize(glm::rotate(glm::identity<glm::mat4>(), 0.001f * dx, glm::vec3{0, 1, 0}) * glm::vec4(g_Scene.camera->forward, 1));
    g_Scene.camera->forward.y += 0.001f * dy;
    g_Scene.camera->forward = glm::normalize(g_Scene.camera->forward);

    prev_x = event->GetMove().x;
    prev_y = event->GetMove().y;
}

void ProcessKeyEvent(Event* event)
{
    assert(event);

    int key = event->GetKey().key;
    int action = (int)event->GetKey().action;

    float speed = 0.5;

    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position += speed * g_Scene.camera->forward;
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }

    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position -= speed * g_Scene.camera->forward;
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }

    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position -= speed * glm::cross(g_Scene.camera->forward, glm::vec3{0, 1, 0});
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }

    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position += speed * glm::cross(g_Scene.camera->forward, glm::vec3{0, 1, 0});
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }

    if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position.y += 1;
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }

    if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        g_Scene.camera->props.position.y -= 1;
        g_Scene.camera->props.RecalculateModelTransform(); // FIXME:
    }
}

void ProcessEvent(Event* event, bool* running)
{
    assert(event);
    assert(running);

    switch (event->GetType())
    {
    case kQuit:
        *running = false;
        break;
    
    case kKey:
        ProcessKeyEvent(event);
        break;

    case kMouseMove:
        ProcessMoveEvent(event);
        break;

    default:
        break;
    }
}

int main()
{
    Window _window{};
    EventQueue::SetWindow(&_window);
    NativeWindow* window = _window.GetNativeWindow();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int32_t frameBufferWidth = 0;
    int32_t frameBufferHeight = 0;
    glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    g_Scene.camera = CreateShared<Camera>(glm::vec3{10, 5, 10}, glm::vec3{-1, -0.5, -1});
    g_Scene.light = CreateShared<LightSource>(glm::vec3{10, 0, 15}, glm::vec3{1, 1, 0});
    g_Scene.meshes.push_back(CreateShared<MeshInstance>(glm::vec3{0, 0, 0}, ParseMeshObj("res/meshes/skameiki.obj")));

    SharedPtr<Shader> shader = Shader::Create("res/shaders/basic.shader");

    Renderer3D::Init();
    Renderer3D::SetViewport(Viewport{0, 0, static_cast<uint32_t>(frameBufferWidth),
                                           static_cast<uint32_t>(frameBufferHeight)});

    //FIXME:
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Event event{};
    bool running = true;

    while (running)
    {
        while (PollEvent(&event))
        {
            ProcessEvent(&event, &running);
        }

        Renderer3D::RenderScene(g_Scene, shader);
        glfwSwapBuffers(window);
    }

    return 0;
}

// int main(int argc, const char* argv[])
// {
//     GLFWwindow* window = nullptr;
//     if (!glfwInit())
//     {
//         std::cout << "Failed to initialize GLFW" << std::endl;
//         return -1;
//     }

//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// #ifdef __APPLE__
//     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
// #endif

//     window = glfwCreateWindow(640, 480, "Hello, OpenGL!", /*monitor=*/nullptr, /*share=*/nullptr);
//     if (!window)
//     {
//         std::cout << "Failed to create window" << std::endl;
//         glfwTerminate();
//         return -1;
//     }

//     int32_t frameBufferWidth, frameBufferHeight;
//     glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

//     glfwMakeContextCurrent(window);

//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//     {
//         std::cout << "Failed to initialize GLAD" << std::endl;
//         glfwTerminate();
//         return -1;
//     }

//     g_Scene.camera = CreateShared<Camera>(glm::vec3{10, 5, 10}, glm::vec3{-1, -0.5, -1});
//     g_Scene.light = CreateShared<LightSource>(glm::vec3{10, 0, 15}, glm::vec3{1, 1, 0});
//     g_Scene.meshes.push_back(CreateShared<MeshInstance>(glm::vec3{0, 0, 0}, ParseMeshObj("res/meshes/skameiki.obj")));

//     SharedPtr<Shader> shader = Shader::Create("res/shaders/basic.shader");

//     Renderer3D::Init();
//     Renderer3D::SetViewport(Viewport{0, 0, static_cast<uint32_t>(frameBufferWidth),
//                                            static_cast<uint32_t>(frameBufferHeight)});

//     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
//     glfwSetKeyCallback(window, keyCallback);
//     glfwSetCursorPosCallback(window, mouseMoveCallback);

//     while (!glfwWindowShouldClose(window))
//     {
//         Renderer3D::RenderScene(g_Scene, shader);

//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }

//     glfwTerminate();

//     return 0;
// }