# Window
## Создание системного окна
```C++
Window::Window(size_t width, size_t height, const char* title);
```
 Также в конструкторе вызывается ```glfwInit()```, в деструкторе ```glfwTerminate()```, когда определимся с библиотеками, возможно, будет модуль отвечающий за инициализацию.
```C++
Window::Window(const char* title);
```
Создает fullscreen окно 
### Установка нового заголовка
```C++
void Window::SetTitle(const char* title);
```

### Получение базового окна
```C++
typedef GLFWwindow NativeWindow;

NativeWindow* Window::GetNativeWindow();
```

### Установить FPS в заголовок окна
```C++
  void SetFPSToTitle(double fps);
```

# Event

- Типы эвентов, которые доступны
```C++
enum EventType {
    kNoEvent,
    kQuit,
    kKey,
    kMouseButton,
    kMouseMove,
    kMouseScroll,
};
```

Чтобы начать работать с эвентами, нужно прикрепить окно, эвенты которого нужно получать

```C++
Window window{};
input::EventQueue::SetWindow(&window);
```
Далее создаём эвент и вызываем свободную функцию ```bool PollEvent(Event*)``` для получения нового эвента

```C++
Event event{};

while (running) {
    while (PollEvent(&event)) {
        ProcessEvent(&event);
    }
}
```

Данные эвента и геттеры
```C++
union EventData {
    MouseMoveEventData   move;
    MouseButtonEventData button;
    KeyEventData         key;
    ScrollEventData      scroll;
};

EventType& Event::GetType();
EventData& Event::GetData();

MouseButtonEventData& Event::GetButton()
MouseMoveEventData&   Event::GetMove()
KeyEventData&         Event::GetKey()
ScrollEventData&      Event::GetScroll()
```
## Event types
### EventType == kQuit
Закрытие окна

### EventType == kKey

Нажатие на клавишу на клавиатуре
```C++
event.GetData().key == event.GetKey()
```
```C++
struct KeyEventData {
    int key;              // Actually ascii codes
    int scancode;
    Action action;
    ButtonMods mods;
};
```

- key - ascii код
- action
```C++
enum Action {
    kPress,
    kRelease,
    kHold
};
```
- ```kPress``` - нажатие, ```kRelease``` - отпускание, ```kHold``` - удержание

- mods - информация о зажатых клавишах
```C++
struct ButtonMods {
    bool shift_pressed : 1;
    bool ctrl_pressed  : 1;
    bool alt_pressed   : 1;
};
```

### EventType == kMouseButton
Нажатие на кнопку мыши

```C++
event.GetData().button == event.GetButton()
```

```C++
struct MouseButtonEventData {
    MouseButton button;
    Action action;
    ButtonMods mods;
};
```

```C++
enum MouseButton {
    kLeftMouseButton,
    kRightMouseButton,
    kMiddleMouseButton,
    kX1MouseButton,
    kX2MouseButton
};
```

### EventType == kMouseMove
Приходит при движении мыши внутри окна
```C++
event.GetData().move == event.GetMove()
```

```C++
struct MouseMoveEventData {
    int x;
    int y;
};
```

### EventType == kMouseScroll
Приходит при скроллинге
```C++
event.GetType().scroll == event.GetScroll()
```
```C++
struct ScrollEventData {
    int dx;
    int dy;
};
```
## Keyboard

```C++
class Keyboard {
 public:
  static bool Pressed(Keys key);
};
```

Возвращает, нажата ли клавиша key в данный момент.
Keys - enum, по надобности может быть дополнен
