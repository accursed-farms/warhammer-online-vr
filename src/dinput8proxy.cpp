// dinput8.dll next to WAR.exe: loads WARVR.dll when the game starts, so no injector is needed.
//
// WAR.exe imports DirectInput8Create from dinput8.dll, and Windows looks for that DLL in the game's folder
// first. This one starts WARVR.dll (from the same folder) and passes every call on to the real
// dinput8.dll in the system folder. Remove it (or rename it) to start the game without the mod.
//
// WARVR.dll is loaded from a short thread rather than from DllMain, so nothing heavy runs under the
// loader lock. The thread runs as soon as the game's own start-up DLLs are loaded, well before the game
// creates its Direct3D device, which is what WARVR waits for (and earlier than the injector manages).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HMODULE g_self = nullptr;
static HMODULE g_real = nullptr;

static HMODULE Real()
{
    if (!g_real) {
        wchar_t path[MAX_PATH];
        UINT n = GetSystemDirectoryW(path, MAX_PATH);
        if (n == 0 || n + 14 >= MAX_PATH) return nullptr;
        lstrcatW(path, L"\\dinput8.dll");
        g_real = LoadLibraryW(path);
    }
    return g_real;
}

template <class F> static F Fn(const char* name)
{
    HMODULE m = Real();
    return m ? reinterpret_cast<F>(reinterpret_cast<void*>(GetProcAddress(m, name))) : nullptr;
}

static DWORD WINAPI LoadMod(void*)
{
    wchar_t path[MAX_PATH];
    DWORD n = GetModuleFileNameW(g_self, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return 1;
    wchar_t* slash = path;
    for (wchar_t* p = path; *p; ++p) if (*p == L'\\' || *p == L'/') slash = p + 1;
    *slash = 0;
    if ((slash - path) + 10 >= MAX_PATH) return 1;
    lstrcatW(path, L"WARVR.dll");
    SetEnvironmentVariableW(L"WARVR_LOADED_BY", L"dinput8.dll");   // WARVR.log says how it was started
    return LoadLibraryW(path) ? 0 : 2;
}

extern "C" {

HRESULT WINAPI DirectInput8Create(HINSTANCE inst, DWORD version, REFIID iid, LPVOID* out, void* outer)
{
    using F = HRESULT(WINAPI*)(HINSTANCE, DWORD, REFIID, LPVOID*, void*);
    F f = Fn<F>("DirectInput8Create");
    if (!f) { if (out) *out = nullptr; return E_FAIL; }
    return f(inst, version, iid, out, outer);
}

HRESULT WINAPI DllCanUnloadNow()
{
    using F = HRESULT(WINAPI*)();
    F f = Fn<F>("DllCanUnloadNow");
    return f ? f() : S_FALSE;
}

HRESULT WINAPI DllGetClassObject(REFCLSID clsid, REFIID iid, LPVOID* out)
{
    using F = HRESULT(WINAPI*)(REFCLSID, REFIID, LPVOID*);
    F f = Fn<F>("DllGetClassObject");
    if (!f) { if (out) *out = nullptr; return E_FAIL; }
    return f(clsid, iid, out);
}

HRESULT WINAPI DllRegisterServer()
{
    using F = HRESULT(WINAPI*)();
    F f = Fn<F>("DllRegisterServer");
    return f ? f() : E_FAIL;
}

HRESULT WINAPI DllUnregisterServer()
{
    using F = HRESULT(WINAPI*)();
    F f = Fn<F>("DllUnregisterServer");
    return f ? f() : E_FAIL;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = hinst;
        DisableThreadLibraryCalls(hinst);
        HANDLE t = CreateThread(nullptr, 0, LoadMod, nullptr, 0, nullptr);
        if (t) CloseHandle(t);
    }
    return TRUE;
}

}   // extern "C"
