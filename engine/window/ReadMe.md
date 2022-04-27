
## Создание системного окна
```C++
Window::Window(size_t width, size_t height, const char* title);
```
 Также в конструкторе вызывается ```glfwInit()```, в деструкторе ```glfwTerminate()```, когда определимся с библиотеками, возможно, будет модуль отвечающий за инициализацию.

#### Установка нового заголовка
```C++
void Window::SetTitle(const char* title);
```

#### Получение базового окна
```C++
typedef GLFWwindow NativeWindow;

NativeWindow* Window::GetNativeWindow();
```
