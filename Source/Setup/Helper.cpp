#include "Helper.h"
#include "Gui.h"
#include "Language.h"
#include <direct.h>
#include <Shlwapi.h>
#include <filesystem>

namespace fs = std::filesystem;

Helper::Helper()
{
    subKey = "SOFTWARE\\Ntreev\\Trickster_CG\\Setup";
}

void Helper::PopulateSupportedResolutions()
{
    int modeNum = 0;
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);
    while (EnumDisplaySettings(NULL, modeNum, &devMode)) 
    {
        std::pair<std::string, std::string> tempPair = std::make_pair(std::to_string(devMode.dmPelsWidth), std::to_string(devMode.dmPelsHeight));
        auto it = std::find(gui::supportedResolutions.begin(), gui::supportedResolutions.end(), tempPair);
        if (it != gui::supportedResolutions.end())
        {
            modeNum++;
            continue;
        }
        gui::supportedResolutions.emplace_back(tempPair);
        modeNum++;
    }
}

bool Helper::SetRegistryDWORD(HKEY hKey, const char* name, DWORD value)
{
    return RegSetValueExA(hKey, name, 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value)) == ERROR_SUCCESS;
}

bool Helper::SetRegistryString(HKEY hKey, const char* name, const char* value)
{
    return RegSetValueExA(hKey, name, 0, REG_SZ,
        reinterpret_cast<const BYTE*>(value), strlen(value) + 1) == ERROR_SUCCESS;
}

bool Helper::GetRegistryDWORD(HKEY hKey, const char* name, DWORD& value)
{
    DWORD data;
    DWORD dataSize = sizeof(data);
    DWORD type;
    LONG res = RegQueryValueExA(hKey, name, nullptr, &type, reinterpret_cast<LPBYTE>(&data), &dataSize);
    if (res == ERROR_SUCCESS && type == REG_DWORD) 
    {
        value = data;
        return true;
    }
    return false;
}

bool Helper::GetRegistryString(HKEY hKey, const char* name, std::string& outValue)
{
    char buffer[256];
    DWORD dataSize = sizeof(buffer);
    DWORD type;
    LONG res = RegQueryValueExA(hKey, name, nullptr, &type, reinterpret_cast<LPBYTE>(buffer), &dataSize);
    if (res == ERROR_SUCCESS && type == REG_SZ) 
    {
        outValue = buffer;
        return true;
    }
    return false;
}

bool Helper::LoadTricksterConfig()
{
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) 
        return false;
    DWORD width = 1024, height = 768, fullScreen = 0, stereo = 1, frequency = 44100, soundBit = 16;
    std::string format;
    GetRegistryDWORD(hKey, "widthPixel", width);
    GetRegistryDWORD(hKey, "heightPixel", height);
    GetRegistryDWORD(hKey, "Full Screen", fullScreen);
    GetRegistryDWORD(hKey, "SoundFrequency", frequency);
    GetRegistryDWORD(hKey, "SoundStereo", stereo);
    GetRegistryDWORD(hKey, "SoundBit", soundBit);
    GetRegistryString(hKey, "CaptureScreenFormat", format);
    gui::savedConfigs.currentResolution = 
    {
        std::to_string(width),
        std::to_string(height)
    };
    gui::savedConfigs.isFullScreen = (fullScreen != 0);
    gui::savedConfigs.soundBitrate = frequency;
    gui::savedConfigs.isSoundStereo = (stereo != 0);
    gui::savedConfigs.screenshotFormat = format.empty() ? "JPG" : format;
    RegCloseKey(hKey);
    return true;
}

bool Helper::EnsureTricksterRegistry()
{
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_READ | KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        result = RegCreateKeyExA(HKEY_CURRENT_USER, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
        if (result != ERROR_SUCCESS)
            return false;
    }
    else
        return LoadTricksterConfig();
    SetRegistryDWORD (hKey, "2DSample", 8);
    SetRegistryDWORD (hKey, "3DSample", 8);
    SetRegistryString(hKey, "CaptureScreenFormat", "JPG");
    SetRegistryDWORD (hKey, "Full Screen", 0);
    SetRegistryDWORD (hKey, "heightPixel", 768);
    SetRegistryDWORD (hKey, "SoundBit", 16);
    SetRegistryDWORD (hKey, "SoundFrequency", 44100);
    SetRegistryDWORD (hKey, "SoundStereo", 1);
    SetRegistryDWORD (hKey, "StreamSample", 2);
    SetRegistryDWORD (hKey, "Use3DEffect", 0);
    SetRegistryDWORD (hKey, "UseSound", 1);
    SetRegistryDWORD (hKey, "widthPixel", 1024);
    RegCloseKey(hKey);
    return true;
}

bool Helper::SaveTricksterConfig()
{
    HKEY hKey;
    LONG result = RegCreateKeyExA(HKEY_CURRENT_USER, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (result != ERROR_SUCCESS)
        return false;
    std::string widthStr, heightStr;
    std::tie(widthStr, heightStr) = gui::savedConfigs.currentResolution;
    DWORD width = std::stoi(widthStr);
    DWORD height = std::stoi(heightStr);
    SetRegistryDWORD (hKey, "widthPixel", width);
    SetRegistryDWORD (hKey, "heightPixel", height);
    SetRegistryDWORD (hKey, "Full Screen", gui::savedConfigs.isFullScreen ? 1 : 0);
    SetRegistryDWORD (hKey, "SoundFrequency", gui::savedConfigs.soundBitrate);
    SetRegistryDWORD (hKey, "SoundStereo", gui::savedConfigs.isSoundStereo ? 1 : 0);
    SetRegistryString(hKey, "CaptureScreenFormat", gui::savedConfigs.screenshotFormat.c_str());
    RegCloseKey(hKey);
    ApplyResolutionFiles();
    UpdateUIFiles();
    if (gui::savedConfigs.changeChatSize)
        changeChat();
    return true;
}

bool Helper::CopyFileSafe(const std::string& src, const std::string& dst, bool overwrite)
{
    if (!CopyFileA(src.c_str(), dst.c_str(), overwrite ? FALSE : TRUE)) 
    {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND)
            return false;
    }
    return true;
}

bool Helper::CopyDirectorySafe(const std::string& srcDir, const std::string& dstDir, bool overwrite)
{
    CreateDirectoryA(dstDir.c_str(), NULL);
    std::string searchPath = srcDir + "\\*";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;
    do 
    {
        std::string name = findData.cFileName;
        if (name == "." || name == "..")
            continue;
        std::string srcPath = srcDir + "\\" + name;
        std::string dstPath = dstDir + "\\" + name;
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
        {
            if (!CopyDirectorySafe(srcPath, dstPath, overwrite))
                return false;
        }
        else 
        {
            if (!CopyFileSafe(srcPath, dstPath, overwrite))
                return false;
        }
    } 
    while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
    return true;
}

void Helper::ApplyResolutionFiles()
{
    int width = std::stoi(gui::savedConfigs.currentResolution.first);
    if (width > 800)
    {
        CopyFileSafe("data\\UI_nori\\intro_img\\intro_base1024.nri", "data\\UI_nori\\intro_img\\intro_base" + gui::savedConfigs.currentResolution.first + ".nri");
        CopyFileSafe("data\\UI_nori\\intro_img\\intro_load1024.nri", "data\\UI_nori\\intro_img\\intro_load" + gui::savedConfigs.currentResolution.first + ".nri");
        CopyDirectorySafe("data\\UI_nori\\LoadingImg_1024\\", "data\\UI_nori\\LoadingImg_" + gui::savedConfigs.currentResolution.first + "\\");
        CopyFileSafe("data\\UI_nori\\CharSelectUI_1024.nri", "data\\UI_nori\\CharSelectUI_" + gui::savedConfigs.currentResolution.first + ".nri");
        CopyFileSafe("data\\UI_nori\\CharSelectUI_1024.ucf", "data\\UI_nori\\CharSelectUI_" + gui::savedConfigs.currentResolution.first + ".ucf");
        CopyFileSafe("data\\UI_nori\\FortuneBg_1024.nri", "data\\UI_nori\\FortuneBg_" + gui::savedConfigs.currentResolution.first + ".nri");
        CopyFileSafe("data\\UI_nori\\InstructionUI_1024.nri", "data\\UI_nori\\InstructionUI_" + gui::savedConfigs.currentResolution.first + ".nri");
        CopyFileSafe("data\\UI_nori\\InstructionUI_1024.ucf", "data\\UI_nori\\InstructionUI_" + gui::savedConfigs.currentResolution.first + ".ucf");
        CopyFileSafe("data\\UI_nori\\LoginUI_1024.ucf", "data\\UI_nori\\LoginUI_" + gui::savedConfigs.currentResolution.first + ".ucf");
        CopyFileSafe("data\\UI_nori\\map_loading_1024.jpg", "data\\UI_nori\\map_loading_" + gui::savedConfigs.currentResolution.first + ".jpg");
    }
    else
    {
        CopyFileSafe("data\\UI_nori\\intro_img\\intro_base800.nri", "data\\UI_nori\\intro_img\\intro_base" + gui::savedConfigs.currentResolution.first + ".nri");
        CopyFileSafe("data\\UI_nori\\intro_img\\intro_load800.nri", "data\\UI_nori\\intro_img\\intro_load" + gui::savedConfigs.currentResolution.first + ".nri");
        CopyDirectorySafe("data\\UI_nori\\LoadingImg_800\\", "data\\UI_nori\\LoadingImg_" + gui::savedConfigs.currentResolution.first + "\\");
        CopyFileSafe("data\\UI_nori\\CharSelectUI_800.nri", "data\\UI_nori\\CharSelectUI_" + gui::savedConfigs.currentResolution.first + ".nri");
        CopyFileSafe("data\\UI_nori\\CharSelectUI_800.ucf", "data\\UI_nori\\CharSelectUI_" + gui::savedConfigs.currentResolution.first + ".ucf");
        CopyFileSafe("data\\UI_nori\\FortuneBg_800.nri", "data\\UI_nori\\FortuneBg_" + gui::savedConfigs.currentResolution.first + ".nri");
        CopyFileSafe("data\\UI_nori\\InstructionUI_800.nri", "data\\UI_nori\\InstructionUI_" + gui::savedConfigs.currentResolution.first + ".nri");
        CopyFileSafe("data\\UI_nori\\InstructionUI_800.ucf", "data\\UI_nori\\InstructionUI_" + gui::savedConfigs.currentResolution.first + ".ucf");
        CopyFileSafe("data\\UI_nori\\LoginUI_1800.ucf", "data\\UI_nori\\LoginUI_" + gui::savedConfigs.currentResolution.first + ".ucf");
        CopyFileSafe("data\\UI_nori\\map_loading_800.jpg", "data\\UI_nori\\map_loading_" + gui::savedConfigs.currentResolution.first + ".jpg");
    }
}

void Helper::UpdateUIFiles() 
{
    int CharSelectXMinus = std::stoi(gui::savedConfigs.currentResolution.first) - 582;
    int CharSelectX = CharSelectXMinus / 2;
    int CharSelectYMinus = std::stoi(gui::savedConfigs.currentResolution.second) - 458;
    int CharSelectY = CharSelectYMinus / 2;
    int TimeCapsuleX = CharSelectX;
    int TimeCapsuleY = CharSelectY + 346;
    std::string charSelectPath = ".\\data\\UI_nori\\CharSelectUI.ucf";
    std::string charSelectBak = charSelectPath + "_bak";
    std::string timeCapsulePath = ".\\data\\UI_nori\\TimeCapsuleUI.ucf";
    std::string timeCapsuleBak = timeCapsulePath + "_bak";
    if (!fs::exists(charSelectBak) && fs::exists(charSelectPath))
        fs::copy_file(charSelectPath, charSelectBak, fs::copy_options::skip_existing);
    if (!fs::exists(timeCapsuleBak) && fs::exists(timeCapsulePath))
        fs::copy_file(timeCapsulePath, timeCapsuleBak, fs::copy_options::skip_existing);
    {
        std::ofstream out(charSelectPath, std::ios::trunc);

        out << "iBase_CoordX_800 = 109\n";
        out << "iBase_CoordY_800 = 80\n";
        out << "iBase_CoordX_1024 = 221\n";
        out << "iBase_CoordY_1024 = 155\n";
        out << "iBase_CoordX_" << std::stoi(gui::savedConfigs.currentResolution.first) << " = " << CharSelectX << "\n";
        out << "iBase_CoordY_" << std::stoi(gui::savedConfigs.currentResolution.first) << " = " << CharSelectY << "\n\n";
        out << "iDeleteBtn_CoordX = 24\n";
        out << "iDeleteBtn_CoordY = 314\n\n";
        out << "iCreateBtn_CoordX = 86\n";
        out << "iCreateBtn_CoordY = 314\n\n";
        out << "iResetLvUpPtsBtn_CoordY = 312\n\n";
        out << "iBasicBtn0_CoordX = 449\n";
        out << "iBasicBtn0_CoordY = 314\n\n";
        out << "iBasicBtn1_CoordX = 507\n";
        out << "iBasicBtn1_CoordY = 314\n\n";
        out << "iSlot0_CoordX = 30\n";
        out << "iSlot0_CoordY = 65\n\n";
        out << "iSlot1_CoordX = 211\n";
        out << "iSlot1_CoordY = 65\n\n";
        out << "iSlot2_CoordX = 392\n";
        out << "iSlot2_CoordY = 65\n\n";
        out << "iNameStat_CoordX = 12\n";
        out << "iNameStat_CoordY = 6\n";
        out << "iNameStat_DimX = 138\n";
        out << "iNameStat_DimY = 24\n\n";
        out << "iTypeCaption_CoordX = 28\n";
        out << "iTypeCaption_CoordY = 132\n";
        out << "iTypeStatic_CoordX = 66\n";
        out << "iTypeStatic_CoordY = 132\n\n";
        out << "iJobCaption_CoordX = 28\n";
        out << "iJobCaption_CoordY = 146\n";
        out << "iJobStatic_CoordX = 66\n";
        out << "iJobStatic_CoordY = 146\n\n";
        out << "iLevelCaption_CoordX = 28\n";
        out << "iLevelCaption_CoordY = 160\n";
        out << "iLevelStatic_CoordX = 66\n";
        out << "iLevelStatic_CoordY = 160\n\n";
        out << "iHPCaption_CoordX = 28\n";
        out << "iHPCaption_CoordY = 174\n";
        out << "iHPStatic_CoordX = 66\n";
        out << "iHPStatic_CoordY = 174\n\n";
        out << "iMPCaption_CoordX = 28\n";
        out << "iMPCaption_CoordY = 188\n";
        out << "iMPStatic_CoordX = 66\n";
        out << "iMPStatic_CoordY = 188\n\n";
        out << "iMoneyUnit_CoordX = 121\n";
        out << "iMoneyUnit_CoordY = 202\n";
        out << "iMoneyUnit_DimX = 10\n";
        out << "iMoneyUnit_DimY = 14\n\n";
        out << "iMoneyStatic_CoordX = 28\n";
        out << "iMoneyStatic_CoordY = 202\n";
        out << "iMoneyStatic_DimX = 90\n";
        out << "iMoneyStatic_DimY = 14\n\n";
        out << "iCaption_DimX = 35\n";
        out << "iCaption_DimY = 14\n";
        out << "iStatic_DimX = 65\n";
        out << "iStatic_DimY = 14\n\n";
        out << "iSelectFrame0_CoordX = 22\n";
        out << "iSelectFrame0_CoordY = 44\n\n";
        out << "iSelectFrame1_CoordX = 203\n";
        out << "iSelectFrame1_CoordY = 44\n\n";
        out << "iSelectFrame2_CoordX = 384\n";
        out << "iSelectFrame2_CoordY = 44\n\n";
        out << "iCharPic_CoordX = 58\n";
        out << "iCharPic_CoordY = 106\n";
        out << "iCharPic_DimX = 43\n";
        out << "iCharPic_DimY = 15\n\n";
        out << "iMsgBox_DimX = 210\n";
        out << "iMsgBox_DimY = 100\n\n";
        out << "iMsgBoxStatic_DimX = 150\n";
        out << "iMsgBoxStatic_DimY = 32\n\n";
        out << "iMsgBoxStatic_CoordX = 52\n";
        out << "iMsgBoxStatic_CoordY = 24\n";
    }
    {
        std::ofstream out(timeCapsulePath, std::ios::trunc);
        out << "iMenu_CoordX_800 = 109\n";
        out << "iMenu_CoordY_800 = 426\n";
        out << "iMenu_CoordX_1024 = 221\n";
        out << "iMenu_CoordY_1024 = 501\n";
        out << "iMenu_CoordX_" << std::stoi(gui::savedConfigs.currentResolution.first) << " = " << TimeCapsuleX << "\n";
        out << "iMenu_CoordY_" << std::stoi(gui::savedConfigs.currentResolution.first) << " = " << TimeCapsuleY << "\n\n";
        out << "iMenuBtn_CoordX_0 = 35\n";
        out << "iMenuBtn_CoordY_0 = 52\n";
        out << "iMenuBtn_CoordX_1 = 316\n";
        out << "iMenuBtn_CoordY_1 = 40\n";
        out << "iMenuBtn_CoordX_2 = 435\n";
        out << "iMenuBtn_CoordY_2 = 40\n\n";
        out << "iFormViewDimX = 480\n";
        out << "iFormViewDimY = 300\n\n";
        out << "iListView_CoordX = 10\n";
        out << "iListView_CoordY = 45\n\n";
        out << "iListView_DimX = 460\n";
        out << "iListView_DimY = 220\n\n";
        out << "iListView_Field_DimX_0 = 32\n";
        out << "iListView_Field_DimX_1 = 175\n";
        out << "iListView_Field_DimX_2 = 51\n";
        out << "iListView_Field_DimX_3 = 187\n";
    }
}

void Helper::changeChat() 
{
    if (gui::savedConfigs.restoreChatDefaultSize)
        return changeChatDefault();
    int result1 = std::stoi(gui::savedConfigs.currentResolution.first) * gui::savedConfigs.chatSize;
    int iViewDimX = result1 % 1000;
    int iEditBoxDimX = iViewDimX - 76;
    int iEditBoxMinDimX = iEditBoxDimX - 118;
    int iComboBoxDimX = iEditBoxDimX - 135;
    int iCloseBtnCoordX = iEditBoxDimX + 59;
    int iModeButtonDimXMinus3 = iViewDimX - 3;
    int iModeButtonDimX = iModeButtonDimXMinus3 / 5;
    int iDummyModeButtonDimXMinus3 = iViewDimX - 3;
    int iDummyModeButtonDimXDivide5 = iDummyModeButtonDimXMinus3 / 5;
    int iDummyModeButtonDimX = iDummyModeButtonDimXDivide5 - 1;
    int iStatViewDimX = iViewDimX - 2;
    fs::path chatFile = "./data/UI_nori/ChatUI.ucf";
    fs::path backupFile = "./data/UI_nori/ChatUI.ucf_bak";
    if (!fs::exists(backupFile) && fs::exists(chatFile))
        fs::copy(chatFile, backupFile);
    std::ofstream out(chatFile, std::ios::trunc);
    if (!out) 
    {
        return;
    }
    out << "iViewDimX\t= " << iViewDimX << "\n";
    out << "iViewDimY\t= 22\n\n";
    out << "iEditBoxDimX\t= " << iEditBoxDimX << "\n";
    out << "iEditBoxDimY\t= 18\n";
    out << "iEditBoxCoordX\t= 58\n";
    out << "iEditBoxCoordY\t= 3\n";
    out << "iEditBoxMinDimX\t= " << iEditBoxMinDimX << "\n";
    out << "iEditBoxMinDimY\t= 18\n";
    out << "iEditBoxMinCoordX\t= 176\n";
    out << "iEditBoxMinCoordY\t= 3\n\n";
    out << "iComboBoxDimX\t= " << iComboBoxDimX << "\n";
    out << "iComboBoxDimY\t= 18\n";
    out << "iComboBoxCoordX\t= 58\n";
    out << "iComboBoxCoordY\t= 3\n\n";
    out << "iEmoticonBtnCoordX\t= 5\n";
    out << "iEmoticonBtnCoordY\t= 5\n";
    out << "iMemoBtnCoordX\t= 22\n";
    out << "iMemoBtnCoordY\t= 5\n";
    out << "iWhisperBtnCoordX\t= 40\n";
    out << "iWhisperBtnCoordY\t= 5\n";
    out << "iCloseBtnCoordX\t= " << iCloseBtnCoordX << "\n";
    out << "iCloseBtnCoordY\t= 6\n\n";
    out << "iStatViewDimX\t= " << iStatViewDimX << "\n";
    out << "iStatViewDimY\t= 78\n\n";
    out << "iStatViewMaxDimX\t= " << iStatViewDimX << "\n";
    out << "iStatViewMaxDimY\t= 478\n\n";
    out << "iModeButtonDimX\t= " << iModeButtonDimX << "\n";
    out << "iModeButtonDimY\t= 18\n\n";
    out << "iDummyModeButtonDimX\t= " << iDummyModeButtonDimX << "\n";
    out << "iDummyModeButtonDimY\t= 18\n\n";
}

void Helper::changeChatDefault() 
{
    fs::path chatFile = "./data/UI_nori/ChatUI.ucf";
    std::ofstream out(chatFile, std::ios::trunc);
    if (!out) 
    {
        return;
    }
    out << "iViewDimX\t= 327\n";
    out << "iViewDimY\t= 22\n\n";
    out << "iEditBoxDimX\t= 251\n";
    out << "iEditBoxDimY\t= 18\n";
    out << "iEditBoxCoordX\t= 58\n";
    out << "iEditBoxCoordY\t= 3\n";
    out << "iEditBoxMinDimX\t= 133\n";
    out << "iEditBoxMinDimY\t= 18\n";
    out << "iEditBoxMinCoordX\t= 176\n";
    out << "iEditBoxMinCoordY\t= 3\n\n";
    out << "iComboBoxDimX\t= 116\n";
    out << "iComboBoxDimY\t= 18\n";
    out << "iComboBoxCoordX\t= 58\n";
    out << "iComboBoxCoordY\t= 3\n\n";
    out << "iEmoticonBtnCoordX\t= 5\n";
    out << "iEmoticonBtnCoordY\t= 5\n";
    out << "iMemoBtnCoordX\t= 22\n";
    out << "iMemoBtnCoordY\t= 5\n";
    out << "iWhisperBtnCoordX\t= 40\n";
    out << "iWhisperBtnCoordY\t= 5\n";
    out << "iCloseBtnCoordX\t= 310\n";
    out << "iCloseBtnCoordY\t= 6\n\n";
    out << "iStatViewDimX\t= 325\n";
    out << "iStatViewDimY\t= 78\n\n";
    out << "iStatViewMaxDimX\t= 325\n";
    out << "iStatViewMaxDimY\t= 478\n\n";
    out << "iModeButtonDimX\t= 65\n";
    out << "iModeButtonDimY\t= 18\n\n";
    out << "iDummyModeButtonDimX\t= 64\n";
    out << "iDummyModeButtonDimY\t= 18\n\n";
}