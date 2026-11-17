#include <windows.h>

struct RenderWindow
{
    int m_0000;
    HWND window;
    int m_0008;
    HINSTANCE instance;
    char pad_0010[0x48];
    char title[0x64];
    char className[0x64];
    int m_0120;
    int m_0124;
    void RegisterWindowClass(HINSTANCE instance);
    void Create(HINSTANCE instance);
};

extern "C" RenderWindow *DAT_10f1a860;
extern "C" int DAT_10f65514;
extern "C" int DAT_10202b24;
extern "C" int DAT_10202b28;

void RenderWindow::Create(HINSTANCE instance)
{
    m_0124 = 0;
    m_0120 = 0;
    m_0008 = 0;
    RegisterWindowClass(instance);
    this->instance = instance;
    DAT_10f1a860 = this;
    if (DAT_10f65514 != 0)
        window = CreateWindowExA(0, className, title, WS_POPUP, CW_USEDEFAULT, 0, DAT_10202b24, DAT_10202b28, 0, 0, this->instance, 0);
    else
        window = CreateWindowExA(0, className, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, DAT_10202b24, DAT_10202b28, 0, 0, this->instance, 0);
}
