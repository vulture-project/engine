/**
 * @author Sergey Zelenkin (https://github.com/vssense)
 * @file event.hpp
 * @date 2022-04-27
 * 
 * The MIT License (MIT)
 * Copyright (c) vulture-project
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <queue>

#include "platform/window.hpp"

namespace input
{

enum EventType
{
    kNoEvent,
    kQuit,
    kKey,
    kMouseButton,
    kMouseMove,
    kMouseScroll,

    // kMouseEnter,
    // kMouseLeave,
};

enum Action
{
    kPress,
    kRelease,
    kHold
};

enum MouseButton
{
    kLeftMouseButton,
    kRightMouseButton,
    kMiddleMouseButton,
    kX1MouseButton,
    kX2MouseButton
};

struct ButtonMods
{
    ButtonMods() = default;
    explicit ButtonMods(int mods);

    bool shift_pressed : 1;
    bool ctrl_pressed  : 1;
    bool alt_pressed   : 1;
};

struct ScrollEventData
{
    int dx;
    int dy;
};

struct MouseMoveEventData         // Do we need dx and dy here?
{
    int x;
    int y;
};

struct MouseButtonEventData        // Do we need coords here?
{
    MouseButtonEventData() = default;
    MouseButtonEventData(int button, int action, int mods);

    MouseButton button;
    Action action;
    ButtonMods mods;
};

struct KeyEventData
{
    KeyEventData() = default;
    KeyEventData(int key, int scancode, int action, int mods);

    int key;
    int scancode;
    Action action;
    ButtonMods mods;
};

union EventData
{
    EventData() = default;
    explicit EventData(const MouseMoveEventData&   data) : move(data) {}
    explicit EventData(const MouseButtonEventData& data) : button(data) {}
    explicit EventData(const KeyEventData&         data) : key(data) {}
    explicit EventData(const ScrollEventData&      data) : scroll(data) {}

    MouseMoveEventData   move;
    MouseButtonEventData button;
    KeyEventData         key;
    ScrollEventData      scroll;
};

class Event
{
  public:
    Event() = default;
    explicit Event(EventType type) : type_(type) {}
    Event(EventType type, const EventData& data)
        : type_(type), data_(data) {}

    EventType& GetType()
    {
        return type_;
    }

    EventData& GetData()
    {
        return data_;
    }

    MouseButtonEventData& GetButton()
    {
        return data_.button;
    }

    MouseMoveEventData& GetMove()
    {
        return data_.move;
    }

    KeyEventData& GetKey()
    {
        return data_.key;
    }

    ScrollEventData& GetScroll()
    {
        return data_.scroll;
    }

  private:
    EventType type_{kNoEvent};
    EventData data_;
};

bool PollEvent(Event* event);

class EventQueue
{
  private:
    EventQueue() = default;

  public:
    static void SetWindow(Window* window);

    static void PostEvent(const Event& event);
    static bool PollEvent(Event* event);

  private:
    static void KeyCallback        (GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void MouseMoveCallback  (GLFWwindow* window, double xpos, double ypos);
    static void CloseCallback      (GLFWwindow* window);
    static void ScrollCallback     (GLFWwindow* window, double dx, double dy);

    static Window* window_;
    static std::queue<Event> queue_;
};

} // namespace input
