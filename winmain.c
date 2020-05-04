#include "wima.h"

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd)
{
    int argc;
    LPWSTR *wargv;
    wargv = CommandLineToArgvW(GetCommandLineW(), &argc);

    return MyMain(argc, wargv);
}
