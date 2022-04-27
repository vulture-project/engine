#include <cassert>
#include <iostream>
#include <GLFW/glfw3.h>

#include "event.hpp"

namespace input
{

Window* EventQueue::window_ = nullptr;
std::queue<Event> EventQueue::queue_{};

ButtonMods::ButtonMods(int mods)
{
    shift_pressed = static_cast<bool>(mods & GLFW_MOD_SHIFT);
    ctrl_pressed  = static_cast<bool>(mods & GLFW_MOD_CONTROL);
    alt_pressed   = static_cast<bool>(mods & GLFW_MOD_ALT);
}

MouseButtonEventData::MouseButtonEventData(int button, int action, int mods)
    : button(static_cast<MouseButton>(button)),
      action(static_cast<Action>(action)),
      mods(mods) {}

KeyEventData::KeyEventData(int key, int scancode, int action, int mods)
    : key(key),
      scancode(scancode),
      action(static_cast<Action>(action)),
      mods(mods) {}

void EventQueue::ScrollCallback(GLFWwindow* window, double dx, double dy)
{
    queue_.emplace(kMouseScroll, EventData(ScrollEventData{static_cast<int>(dx),
                                                           static_cast<int>(dy)}));

    // std::cout << '[' << dx << ';' << dy << ']' << '\n';
}

void EventQueue::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    queue_.emplace(kMouseButton, MouseButtonEventData(button, action, mods));

    // std::cout << "button = " << button << '\n';
    // std::cout << "action = " << action << '\n';
    // std::cout << "mods   = " << mods << '\n';
}

void EventQueue::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    queue_.emplace(kKey, KeyEventData(key, scancode, action, mods));

    // std::cout << "key      = " << char(key) << '\n';
    // std::cout << "scancode = " << scancode << '\n';
    // std::cout << "action   = " << action << '\n';
    // std::cout << "mods     = " << mods << '\n';
}

void EventQueue::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
    queue_.emplace(kMouseMove, MouseMoveEventData{static_cast<int>(xpos),
                                                  static_cast<int>(xpos)});

    // std::cout << '[' << xpos << ';' << ypos << ']' << '\n';
}

void EventQueue::CloseCallback(GLFWwindow* window)
{
    queue_.emplace(kQuit);

    // std::cout << "Closing!1!1!1!1!1\n";
}

void EventQueue::SetWindow(Window* window)
{
    assert(window);

    window_ = window;
    GLFWwindow* native_window = window_->GetNativeWindow();

    glfwSetKeyCallback        (native_window, KeyCallback);
    glfwSetWindowCloseCallback(native_window, CloseCallback);
    glfwSetCursorPosCallback  (native_window, MouseMoveCallback);
    glfwSetMouseButtonCallback(native_window, MouseButtonCallback);
    glfwSetScrollCallback     (native_window, ScrollCallback);
}

void EventQueue::PostEvent(const Event& event)
{
    queue_.push(event);
}

bool EventQueue::PollEvent(Event* event)
{
    glfwPollEvents();

    if (!queue_.empty())
    {
        *event = queue_.front();
        queue_.pop();

        return true;
    }

    return false;
}

bool PollEvent(Event* event)
{
    return EventQueue::PollEvent(event);
}

} // namespace input
