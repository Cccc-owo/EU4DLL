// Simplified version of Ultimate ASI Loader
// https://github.com/ThirteenAG/Ultimate-ASI-Loader
// Removed: auto_update, SHGetKnownFolderPath (mod_download removed)

#include <windows.h>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

extern "C" {
    struct
    {
        FARPROC GetFileVersionInfoA;
        FARPROC GetFileVersionInfoByHandle;
        FARPROC GetFileVersionInfoExA;
        FARPROC GetFileVersionInfoExW;
        FARPROC GetFileVersionInfoSizeA;
        FARPROC GetFileVersionInfoSizeExA;
        FARPROC GetFileVersionInfoSizeExW;
        FARPROC GetFileVersionInfoSizeW;
        FARPROC GetFileVersionInfoW;
        FARPROC VerFindFileA;
        FARPROC VerFindFileW;
        FARPROC VerInstallFileA;
        FARPROC VerInstallFileW;
        FARPROC VerLanguageNameA;
        FARPROC VerLanguageNameW;
        FARPROC VerQueryValueA;
        FARPROC VerQueryValueW;

        void LoadOriginalLibrary(HMODULE module)
        {
            GetFileVersionInfoA = GetProcAddress(module, "GetFileVersionInfoA");
            GetFileVersionInfoByHandle = GetProcAddress(module, "GetFileVersionInfoByHandle");
            GetFileVersionInfoExA = GetProcAddress(module, "GetFileVersionInfoExA");
            GetFileVersionInfoExW = GetProcAddress(module, "GetFileVersionInfoExW");
            GetFileVersionInfoSizeA = GetProcAddress(module, "GetFileVersionInfoSizeA");
            GetFileVersionInfoSizeExA = GetProcAddress(module, "GetFileVersionInfoSizeExA");
            GetFileVersionInfoSizeExW = GetProcAddress(module, "GetFileVersionInfoSizeExW");
            GetFileVersionInfoSizeW = GetProcAddress(module, "GetFileVersionInfoSizeW");
            GetFileVersionInfoW = GetProcAddress(module, "GetFileVersionInfoW");
            VerFindFileA = GetProcAddress(module, "VerFindFileA");
            VerFindFileW = GetProcAddress(module, "VerFindFileW");
            VerInstallFileA = GetProcAddress(module, "VerInstallFileA");
            VerInstallFileW = GetProcAddress(module, "VerInstallFileW");
            VerLanguageNameA = GetProcAddress(module, "VerLanguageNameA");
            VerLanguageNameW = GetProcAddress(module, "VerLanguageNameW");
            VerQueryValueA = GetProcAddress(module, "VerQueryValueA");
            VerQueryValueW = GetProcAddress(module, "VerQueryValueW");
        }
    } version;

    void _GetFileVersionInfoA() { version.GetFileVersionInfoA(); }
    void _GetFileVersionInfoByHandle() { version.GetFileVersionInfoByHandle(); }
    void _GetFileVersionInfoExA() { version.GetFileVersionInfoExA(); }
    void _GetFileVersionInfoExW() { version.GetFileVersionInfoExW(); }
    void _GetFileVersionInfoSizeA() { version.GetFileVersionInfoSizeA(); }
    void _GetFileVersionInfoSizeExA() { version.GetFileVersionInfoSizeExA(); }
    void _GetFileVersionInfoSizeExW() { version.GetFileVersionInfoSizeExW(); }
    void _GetFileVersionInfoSizeW() { version.GetFileVersionInfoSizeW(); }
    void _GetFileVersionInfoW() { version.GetFileVersionInfoW(); }
    void _VerFindFileA() { version.VerFindFileA(); }
    void _VerFindFileW() { version.VerFindFileW(); }
    void _VerInstallFileA() { version.VerInstallFileA(); }
    void _VerInstallFileW() { version.VerInstallFileW(); }
    void _VerLanguageNameA() { version.VerLanguageNameA(); }
    void _VerLanguageNameW() { version.VerLanguageNameW(); }
    void _VerQueryValueA() { version.VerQueryValueA(); }
    void _VerQueryValueW() { version.VerQueryValueW(); }
}

void initInjector()
{
    // Load the real version.dll from System32
    wchar_t sysDir[MAX_PATH];
    GetSystemDirectoryW(sysDir, MAX_PATH);
    path versionPath = path{sysDir} / L"version.dll";

    version.LoadOriginalLibrary(LoadLibraryW(versionPath.c_str()));
}

void LoadScripts(const path& folder)
{
    if (!exists(folder)) return;

    directory_iterator dirit{folder};

    while (dirit != directory_iterator{})
    {
        auto _path = dirit->path();

        if (is_regular_file(_path) && _path.extension() == L".dll")
        {
            LoadLibraryW(_path.c_str());
        }

        ++dirit;
    }
}

bool validateProcess() {
    wchar_t pluginpath[MAX_PATH];
    GetModuleFileNameW(NULL, pluginpath, MAX_PATH);

    path exePath{pluginpath};
    auto stem = exePath.stem().wstring();
    auto ext = exePath.extension().wstring();

    // Case-insensitive compare for "eu4.exe"
    for (auto& c : stem) c = towlower(c);
    for (auto& c : ext) c = towlower(c);

    return (stem == L"eu4" && ext == L".exe");
}

void Initialize(HMODULE hSelf)
{
    wchar_t pluginpath[MAX_PATH];
    GetModuleFileNameW(hSelf, pluginpath, MAX_PATH);

    const path pluginsPath = path{pluginpath}.parent_path() / L"plugins";

    initInjector();
    LoadScripts(pluginsPath);
}

BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (validateProcess())
        {
            Initialize(module);
        }
        else {
            initInjector();
        }
    }

    return TRUE;
}
