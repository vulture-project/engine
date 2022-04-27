#include <cassert>
#include <GLFW/glfw3.h>

#include "window.hpp"

const char* Window::kDefaultTitle = "Success is inevitable";

Window::Window(size_t width, size_t height, const char* title)
{
    if (!glfwInit())
    {
        assert(!"Can't init glfw while creating a window");
    }

    window_ = glfwCreateWindow(width, height, title, NULL, NULL);
    assert(window_ && "Can't create a window");

    glfwMakeContextCurrent(window_);
}

void Window::SetTitle(const char* title)
{
    assert(window_);
    assert(title);

    glfwSetWindowTitle(window_, title);
}

GLFWwindow* Window::GetNativeWindow()
{
    return window_;
}

Window::~Window()
{
    assert(window_);
    glfwDestroyWindow(window_);

    glfwTerminate();
}