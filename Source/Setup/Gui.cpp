#pragma once
#include "Gui.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#include "Language.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <imgui_internal.h>
#include "resource.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter );

namespace gui 
{
	static bool showMainWindow = true;
	ImFont* Small = new ImFont();
	ImFont* Regular = new ImFont();
	ImFont* Big = new ImFont();
	Helper* helper = nullptr;
	bool isRunning = true, isVerifying = false;
	HWND window = nullptr;
	WNDCLASSEX windowClass = {};
	POINTS position = {};
	PDIRECT3D9 d3d = nullptr;
	LPDIRECT3DDEVICE9 device = nullptr;
	D3DPRESENT_PARAMETERS presentParameters = {};
	Helper::Config savedConfigs = Helper::Config();
	std::vector<std::pair<std::string, std::string>> supportedResolutions = {};
	const char* sound_mode[] = { "44100 Hz", "44800 Hz" };
	const int sound_values[] = { 44100, 44800 };
	const char* screenshot_mode[] = { "JPG", "PNG", "BMP" };
}

long __stdcall WindowProcess( HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter )
{
	if ( ImGui_ImplWin32_WndProcHandler( window, message, wideParameter, longParameter ) )
		return true;
	switch ( message )
	{
		case WM_SIZE: 
		{
			if ( gui::device && wideParameter != SIZE_MINIMIZED )
			{
				gui::presentParameters.BackBufferWidth = LOWORD( longParameter );
				gui::presentParameters.BackBufferHeight = HIWORD( longParameter );
				gui::ResetDevice();
			}
		} return 0;
		case WM_SYSCOMMAND: 
		{
			if ( ( wideParameter & 0xfff0 ) == SC_KEYMENU )
				return 0;
		} break;
		case WM_DESTROY: 
		{
			ExitProcess(0);
		} return 0;
		case WM_LBUTTONDOWN: 
		{
			gui::position = MAKEPOINTS( longParameter );
		} return 0;
		case WM_MOUSEMOVE: 
		{
			if ( wideParameter == MK_LBUTTON )
			{
				const auto points = MAKEPOINTS( longParameter );
				auto rect = ::RECT{};
				GetWindowRect( gui::window, &rect );
				rect.left += points.x - gui::position.x;
				rect.top += points.y - gui::position.y;
				if (gui::position.x >= 0 && gui::position.x <= gui::WIDTH && gui::position.y >= 0 && gui::position.y <= 19 )
					SetWindowPos( gui::window, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER );
			}

		} return 0;
	}
	return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow( const WCHAR* windowName ) noexcept
{
	windowClass.cbSize = sizeof( WNDCLASSEX );
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA( 0 );
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = L"class001";
	windowClass.hIconSm = 0;
	RegisterClassEx( &windowClass );
	window = CreateWindowExW( 0, L"class001", windowName, WS_POPUP, 100, 100, WIDTH, HEIGHT, 0, 0, windowClass.hInstance, 0 );
	ShowWindow( window, SW_SHOWDEFAULT );
	UpdateWindow( window );
}

void gui::DestroyHWindow() noexcept
{
	DestroyWindow( window );
	UnregisterClass( windowClass.lpszClassName, windowClass.hInstance );
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9( D3D_SDK_VERSION) ;
	if ( !d3d )
		return false;
	ZeroMemory( &presentParameters, sizeof( presentParameters ) );
	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	if ( d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParameters, &device ) < 0 )
		return false;
	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = device->Reset( &presentParameters );
	if ( result == D3DERR_INVALIDCALL )
		IM_ASSERT( 0 );
	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if ( device )
	{
		device->Release();
		device = nullptr;
	}
	if ( d3d )
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::SetupImGuiStyle() noexcept
{
	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;
	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.f, 0.f, 0.f, 0.f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
	style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
	style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

void gui::AdminPermissions() noexcept
{
	BOOL isAdmin = FALSE;
	PSID adminGroup = nullptr;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup))
	{
		CheckTokenMembership(NULL, adminGroup, &isAdmin);
		FreeSid(adminGroup);
	}
	if (!isAdmin)
	{
		char path[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);
		SHELLEXECUTEINFOA sei = { sizeof(sei) };
		sei.lpVerb = "runas";
		sei.lpFile = path;
		sei.hwnd = NULL;
		sei.nShow = SW_NORMAL;
		if (!ShellExecuteExA(&sei))
		{
			MessageBoxA(NULL, lang::GetString("setup_amin_err").c_str(), "Error!", MB_OK);
			PostQuitMessage(0);
		}
		showMainWindow = false;
	}
}

void gui::InitFonts( float baseFontSize ) noexcept
{
	const char* regularFontPath = "Fontes/NotoSans-Regular.ttf";
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();
	const ImWchar* glyphRanges = io.Fonts->GetGlyphRangesChineseFull();
	float smallFontSize = baseFontSize * 0.8f;
	auto LoadFont = [&](const char* path, float size) -> ImFont* 
	{
		ImFontConfig fontCfg;
		fontCfg.MergeMode = false;
		ImFont* font = io.Fonts->AddFontFromFileTTF(path, size, &fontCfg, glyphRanges);
		if (!font) return nullptr;
		return font;
	};
	Small = LoadFont(regularFontPath, smallFontSize);
	Regular = LoadFont(regularFontPath, baseFontSize);
	Big = LoadFont(regularFontPath, 20);
	io.FontDefault = Regular ? Regular : io.Fonts->Fonts[0];
	io.Fonts->Build();
}

void gui::CreateImGui() noexcept
{
	std::string maintenanceCheck = "";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	if (helper == nullptr)
	{
		helper = new Helper();
	}
	if (!helper->EnsureTricksterRegistry())
	{
		MessageBoxA(NULL, lang::GetString("setup_load_err").c_str(), "Error!", MB_OK);
		PostQuitMessage(0);
	}
	helper->PopulateSupportedResolutions();
    InitFonts();
    ImGuiIO& io = ImGui::GetIO();
    SetupImGuiStyle();
	io.IniFilename = NULL;
	ImGui_ImplWin32_Init( window );
	ImGui_ImplDX9_Init( device );
}

void gui::CleanupDeviceD3D() noexcept
{
	if (device) { device->Release(); device = NULL; }
	if (d3d) { d3d->Release(); d3d = NULL; }
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	CleanupDeviceD3D();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while ( PeekMessage( &message, 0, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage( &message );
		DispatchMessage( &message );
		if ( message.message == WM_QUIT )
		{
			isRunning = !isRunning;
			return;
		}
	}
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();
	device->SetRenderState( D3DRS_ZENABLE, FALSE );
	device->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	device->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
	device->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA( 0, 0, 0, 255 ), 1.0f, 0 );
	if ( device->BeginScene() >= 0 )
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData() );
		device->EndScene();
	}
	const auto result = device->Present( 0, 0, 0, 0 );
	if ( result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET )
		ResetDevice();
}

void gui::Render() noexcept
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize(io.DisplaySize);
	bool opened = ImGui::Begin("Setup", &showMainWindow, &showMainWindow, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
	if (opened)
	{
		float itemWidth = 250.0f;
		float offsetX = (ImGui::GetWindowSize().x - itemWidth) * 0.5f;
		ImGui::PushItemWidth(itemWidth);
		if (ImGui::CollapsingHeader(lang::GetString("setup_resolution").c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf))
		{
			ImGui::SetCursorPosX(offsetX);
			ImGui::Text(lang::GetString("setup_res_select").c_str());
			std::vector<std::string> resolutionLabels;
			resolutionLabels.reserve(supportedResolutions.size());
			for (auto& res : supportedResolutions)
				resolutionLabels.push_back(std::get<0>(res) + "x" + std::get<1>(res));
			std::vector<const char*> items;
			items.reserve(resolutionLabels.size());
			for (auto& s : resolutionLabels)
				items.push_back(s.c_str());
			int currentIndex = 0;
			for (size_t i = 0; i < supportedResolutions.size(); i++) 
			{
				if (supportedResolutions[i] == savedConfigs.currentResolution) 
				{
					currentIndex = static_cast<int>(i);
					break;
				}
			}
			ImGui::SetCursorPosX(offsetX);
			if (ImGui::Combo("##resolution", &currentIndex, items.data(), static_cast<int>(items.size()))) 
			{
				savedConfigs.currentResolution = supportedResolutions[currentIndex];

			}
			ImGui::SetCursorPosX(offsetX);
			ImGui::Checkbox(lang::GetString("setup_res_fullscreen").c_str(), &savedConfigs.isFullScreen);
		}
		if (ImGui::CollapsingHeader(lang::GetString("setup_sound").c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf))
		{
			ImGui::SetCursorPosX(offsetX);
			ImGui::Text(lang::GetString("setup_sound_bitrate").c_str());
			int currentBitRate = 0;
			for (int i = 0; i < IM_ARRAYSIZE(sound_values); i++) 
			{
				if (sound_values[i] == savedConfigs.soundBitrate) 
				{
					currentBitRate = i;
					break;
				}
			}
			ImGui::SetCursorPosX(offsetX);
			if (ImGui::Combo("##sound", &currentBitRate, sound_mode, IM_ARRAYSIZE(sound_mode))) 
			{
				savedConfigs.soundBitrate = sound_values[currentBitRate];
			}
			ImGui::SetCursorPosX(offsetX);
			ImGui::Checkbox(lang::GetString("setup_sound_stereo").c_str(), &savedConfigs.isSoundStereo);
		}
		if (ImGui::CollapsingHeader(lang::GetString("setup_screenshot").c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf))
		{
			ImGui::SetCursorPosX(offsetX);
			ImGui::Text(lang::GetString("setup_scrn_format").c_str());
			int currentFormat = 0;
			for (int i = 0; i < IM_ARRAYSIZE(screenshot_mode); i++) 
			{
				if (screenshot_mode[i] == savedConfigs.screenshotFormat) 
				{
					currentFormat = i;
					break;
				}
			}
			ImGui::SetCursorPosX(offsetX);
			if (ImGui::Combo("##screenshot", &currentFormat, screenshot_mode, IM_ARRAYSIZE(screenshot_mode)))
			{
				savedConfigs.screenshotFormat = screenshot_mode[currentFormat];
			}
		}
		if (ImGui::CollapsingHeader(lang::GetString("setup_chat").c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf))
		{
			ImGui::SetCursorPosX(offsetX);
			ImGui::Checkbox(lang::GetString("setup_chat_changesize").c_str(), &savedConfigs.changeChatSize);
			if (savedConfigs.changeChatSize)
			{
				ImGui::SetCursorPosX(offsetX);
				ImGui::Checkbox(lang::GetString("setup_chat_restore").c_str(), &savedConfigs.restoreChatDefaultSize);
				if (!savedConfigs.restoreChatDefaultSize)
				{
					ImGui::SetCursorPosX(offsetX);
					ImGui::Text(lang::GetString("setup_chat_size").c_str());
					ImGui::SetCursorPosX(offsetX);
					ImGui::SliderInt("##charsize", &savedConfigs.chatSize, 319, 595);
				}
				else
				{
					ImGui::BeginDisabled();
					ImGui::SetCursorPosX(offsetX);
					ImGui::Text(lang::GetString("setup_chat_size").c_str());
					ImGui::SetCursorPosX(offsetX);
					ImGui::SliderInt("##charsize_disabled", &savedConfigs.chatSize, 319, 595);
					ImGui::EndDisabled();
				}
			}
			else
			{
				ImGui::BeginDisabled();
				ImGui::SetCursorPosX(offsetX);
				ImGui::Checkbox(lang::GetString("setup_chat_restore").c_str(), &savedConfigs.restoreChatDefaultSize);
				ImGui::SetCursorPosX(offsetX);
				ImGui::Text(lang::GetString("setup_chat_size").c_str());
				ImGui::SetCursorPosX(offsetX);
				ImGui::SliderInt("##charsize_disabled", &savedConfigs.chatSize, 319, 595);
				ImGui::EndDisabled();
			}
		}
		ImGui::PopItemWidth();
		ImGui::Dummy(ImVec2(0.0f, 20.0f));
		ImGui::SetCursorPosX(offsetX);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(89 / 255.f, 139 / 255.f, 59 / 255.f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(73 / 255.f, 163 / 255.f, 93 / 255.f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(54 / 255.f, 133 / 255.f, 67 / 255.f, 1.0f));
		ImGui::PushFont(gui::Big);
		if (ImGui::Button(lang::GetString("setup_save_btn").c_str(), ImVec2(itemWidth, itemWidth / 4)))
		{
			if (helper->SaveTricksterConfig())
				MessageBoxA(NULL, lang::GetString("setup_save_ok").c_str(), "Success!", MB_OK);
			else
				MessageBoxA(NULL, lang::GetString("setup_save_err").c_str(), "Error!", MB_OK);
		}
		ImGui::PopStyleColor(3);
		float closeWidth = 150.0f;
		float offsetCloseX = (ImGui::GetWindowSize().x - closeWidth) * 0.5f;
		ImGui::SetCursorPosX(offsetCloseX);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(139 / 255.f, 59 / 255.f, 59 / 255.f, 1.0f));  // Botão normal (vermelho escuro)
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(163 / 255.f, 73 / 255.f, 73 / 255.f, 1.0f));  // Botão hover (vermelho vivo)
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(133 / 255.f, 54 / 255.f, 54 / 255.f, 1.0f));  // Botão clicado (tom intermediário)
		if (ImGui::Button(lang::GetString("setup_close_btn").c_str(), ImVec2(closeWidth, closeWidth / 3)))
		{
			showMainWindow = false;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
	}
	ImGui::End();

	if (!showMainWindow)
		isRunning = false;
}