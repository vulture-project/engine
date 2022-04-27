#ifndef _EVENT_HPP_INCLUDED
#define _EVENT_HPP_INCLUDED

// Study what is Text input. What is codepoints? Didn't get it.

#include <queue>

#include "window/window.hpp"

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

    int key;              // Actually ascii codes
    int scancode;
    Action action;
    ButtonMods mods;
};

union EventData
{
    EventData() = default;
    EventData(const MouseMoveEventData&   data) : move(data) {}
    EventData(const MouseButtonEventData& data) : button(data) {}
    EventData(const KeyEventData&         data) : key(data) {}
    EventData(const ScrollEventData&      data) : scroll(data) {}

    MouseMoveEventData   move;
    MouseButtonEventData button;
    KeyEventData         key;
    ScrollEventData      scroll;
};

class Event
{
  public:
    Event() = default;
    Event(EventType type) : type_(type) {}
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

} // namespace event

#endif // _EVENT_HPP_INCLUDED