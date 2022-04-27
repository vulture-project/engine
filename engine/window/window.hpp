#ifndef _WINDOW_HPP_INCLUDED
#define _WINDOW_HPP_INCLUDED

class GLFWwindow;

typedef GLFWwindow NativeWindow;

class Window
{
  public:
    Window(size_t width = kDefaultWidth, size_t height = kDefaultHeight,
           const char* title = kDefaultTitle);
    ~Window();

    NativeWindow* GetNativeWindow();
    void SetTitle(const char* title);

  public:
    static const size_t kDefaultWidth = 640;
    static const size_t kDefaultHeight = 480;
    
    static const char* kDefaultTitle;

  private:
    NativeWindow* window_;
};

#endif // _WINDOW_HPP_INCLUDED