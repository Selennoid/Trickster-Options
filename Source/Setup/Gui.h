#pragma once
#define NOMINMAX
#include <Windows.h>
#include <algorithm>
#include <string>
#include <d3d9.h>
#include <d3dx9tex.h>
#include <tchar.h>
#include "imgui.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include "Helper.h"

namespace gui
{
	extern ImFont* Small;
	extern ImFont* Regular;
	extern ImFont* Big;
	extern Helper* helper;
	constexpr int WIDTH = 300;
	constexpr int HEIGHT = 650;
	extern bool isRunning;
	extern HWND window;
	extern WNDCLASSEX windowClass;
	extern POINTS position;
	extern PDIRECT3D9 d3d;
	extern LPDIRECT3DDEVICE9 device;
	extern D3DPRESENT_PARAMETERS presentParameters;
	extern Helper::Config savedConfigs;
	extern std::vector<std::pair<std::string, std::string>> supportedResolutions;
	extern const char* sound_mode[];
	extern const char* screenshot_mode[];
	extern const int sound_values[];
	void CleanupDeviceD3D() noexcept;
	void CreateHWindow(const WCHAR* windowName) noexcept;
	void DestroyHWindow() noexcept;
	bool CreateDevice() noexcept;
	void ResetDevice() noexcept;
	void DestroyDevice() noexcept;
	void CreateImGui() noexcept;
	void DestroyImGui() noexcept;
	void BeginRender() noexcept;
	void EndRender() noexcept;
	void Render() noexcept;
	void SetupImGuiStyle() noexcept;
	void InitFonts( float baseFontSize = 14.0f ) noexcept;
	void AdminPermissions() noexcept;
}
