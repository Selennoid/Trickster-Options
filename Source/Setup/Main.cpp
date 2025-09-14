#define MI_MALLOC_OVERRIDE
#include <mimalloc-new-delete.h>
#include "Gui.h"
#include <thread>
#include <shellapi.h>
#include <fstream>
#include <string>
#include <shlwapi.h>

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE previousInstance, PWSTR arguments, int commandShow)
{
    gui::AdminPermissions();
    gui::CreateHWindow(L"Trickster Settings");
    gui::CreateDevice();
    gui::CreateImGui();
    while (gui::isRunning)
    {
        gui::BeginRender();
        gui::Render();
        gui::EndRender();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    gui::DestroyImGui();
    gui::DestroyDevice();
    gui::DestroyHWindow();
    return EXIT_SUCCESS;
}
