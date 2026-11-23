// The mod is built as dinput8.dll and goes next to WAR.exe. WAR.exe imports DirectInput8Create from
// dinput8.dll and Windows looks in the game's folder first, so the game loads the mod by itself when it
// starts (DllMain in WARVR.cpp starts the worker). The game's DirectInput calls are passed on to the real
// dinput8.dll in the system folder. Delete dinput8.dll from the game folder to play without the mod.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {

HMODULE RealDinput8()
{
    static HMODULE real = nullptr;
    if (!real) {
        wchar_t path[MAX_PATH];
        UINT n = GetSystemDirectoryW(path, MAX_PATH);
        if (n == 0 || n + 14 >= MAX_PATH) return nullptr;
        lstrcatW(path, L"\\dinput8.dll");
        real = LoadLibraryW(path);   // full path: loads the system DLL, not this one
    }
    return real;
}

template <class F> F Real(const char* name)
{
    HMODULE m = RealDinput8();
    return m ? reinterpret_cast<F>(reinterpret_cast<void*>(GetProcAddress(m, name))) : nullptr;
}

}   // namespace

extern "C" {

HRESULT WINAPI DirectInput8Create(HINSTANCE inst, DWORD version, REFIID iid, LPVOID* out, void* outer)
{
    auto f = Real<HRESULT(WINAPI*)(HINSTANCE, DWORD, REFIID, LPVOID*, void*)>("DirectInput8Create");
    if (!f) { if (out) *out = nullptr; return E_FAIL; }
    return f(inst, version, iid, out, outer);
}

HRESULT WINAPI DllCanUnloadNow()
{
    auto f = Real<HRESULT(WINAPI*)()>("DllCanUnloadNow");
    return f ? f() : S_FALSE;
}

HRESULT WINAPI DllGetClassObject(REFCLSID clsid, REFIID iid, LPVOID* out)
{
    auto f = Real<HRESULT(WINAPI*)(REFCLSID, REFIID, LPVOID*)>("DllGetClassObject");
    if (!f) { if (out) *out = nullptr; return E_FAIL; }
    return f(clsid, iid, out);
}

HRESULT WINAPI DllRegisterServer()
{
    auto f = Real<HRESULT(WINAPI*)()>("DllRegisterServer");
    return f ? f() : E_FAIL;
}

HRESULT WINAPI DllUnregisterServer()
{
    auto f = Real<HRESULT(WINAPI*)()>("DllUnregisterServer");
    return f ? f() : E_FAIL;
}

}   // extern "C"
