#pragma once
#include <Windows.h>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include "imgui.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <cstring>
#include <cstdlib>
#include <map>

class Helper
{
    public:
        struct Config
        {
            std::pair<std::string, std::string> currentResolution;
            bool isFullScreen;
            int soundBitrate;
            bool isSoundStereo;
            std::string screenshotFormat;
            bool changeChatSize;
            bool restoreChatDefaultSize;
            int chatSize;
            Config()
            : currentResolution("1024", "768")
            , isFullScreen(false)
            , soundBitrate(44100)
            , isSoundStereo(true)
            , screenshotFormat("JPG") 
            , changeChatSize(false)
            , restoreChatDefaultSize(false)
            , chatSize(319)
            {};
        };
        Helper();
        void PopulateSupportedResolutions();
        bool LoadTricksterConfig();
        bool EnsureTricksterRegistry();
        bool SetRegistryDWORD(HKEY hKey, const char* name, DWORD value);
        bool SetRegistryString(HKEY hKey, const char* name, const char* value);
        bool GetRegistryDWORD(HKEY hKey, const char* name, DWORD& value);
        bool GetRegistryString(HKEY hKey, const char* name, std::string& outValue);
        bool SaveTricksterConfig();
        bool CopyFileSafe(const std::string& src, const std::string& dst, bool overwrite = true);
        bool CopyDirectorySafe(const std::string& srcDir, const std::string& dstDir, bool overwrite = true);
        void ApplyResolutionFiles();
        void UpdateUIFiles();
        void changeChat();
        void changeChatDefault();
        bool IsWidthSupported(int iWidth);
        LPCSTR subKey;
};