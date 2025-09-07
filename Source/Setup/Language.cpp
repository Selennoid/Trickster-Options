#include "Language.h"

namespace lang
{
    Dictionary Languages = 
    {
        {"setup_resolution",     "Resolution Settings"},
        {"setup_res_select",     "Select a resolution:"},
        {"setup_res_fullscreen", "Fullscreen"},
        {"setup_sound",          "Sound Settings"},
        {"setup_sound_bitrate",  "Select the sound bitrate:"},
        {"setup_sound_stereo",   "Enable stereo sound"},
        {"setup_screenshot",     "Screenshot Settings"},
        {"setup_scrn_format",    "Select the screenshot format:"},
        {"setup_save_btn",       "SAVE SETTINGS"},
        {"setup_close_btn",      "CLOSE"},
        {"setup_save_ok",        "Settings saved to the registry!"},
        {"setup_save_err",       "Failed to save settings on the registry!"},
        {"setup_load_err",       "Failed to load settings from registry!"},
        {"setup_amin_err",       "Failed to elevate privileges!"},
        {"setup_chat",           "Chat Bar Settings"},
        {"setup_chat_changesize","Change bar chat size"},
        {"setup_chat_size",      "Chat bar size:"},
        {"setup_chat_restore",   "Restore default chat bar size"},
    };
}

std::string lang::GetString(const std::string& key) noexcept 
{
    auto it = Languages.find(key);
    if (it != Languages.end())
        return it->second;
    return "";
}