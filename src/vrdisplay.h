// vrdisplay.h - our own D3D11 device + two eye textures, used to hand frames to the OpenVR compositor.
//
// WAR renders with D3D9 (through dgVoodoo2 here). OpenVR wants D3D11 textures, and a plain D3D9 device
// cannot share surfaces with D3D11 (your probe showed shared render targets fail), so frames are read back
// on the D3D9 side and uploaded here. d3d11.dll / dxgi.dll are loaded at runtime (no import libraries).
#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdio>
#include <vector>

class VRDisplay {
public:
    using LogFn = void (*)(const char* fmt, ...);

    // adapterIndex comes from OpenVR (GetDXGIOutputInfo): the GPU the headset is attached to.
    bool Init(int adapterIndex, UINT width, UINT height, LogFn log)
    {
        log_ = log;
        w_ = width; h_ = height;
        HMODULE d3d11 = LoadLibraryA("d3d11.dll");
        HMODULE dxgi = LoadLibraryA("dxgi.dll");
        if (!d3d11 || !dxgi) { log_("VRDisplay: cannot load d3d11.dll/dxgi.dll\n"); return false; }
        auto createDevice = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(reinterpret_cast<void*>(GetProcAddress(d3d11, "D3D11CreateDevice")));
        using PFN_CreateFactory1 = HRESULT (WINAPI*)(REFIID, void**);
        auto createFactory = reinterpret_cast<PFN_CreateFactory1>(reinterpret_cast<void*>(GetProcAddress(dxgi, "CreateDXGIFactory1")));
        if (!createDevice) { log_("VRDisplay: D3D11CreateDevice not found\n"); return false; }

        IDXGIAdapter1* adapter = nullptr;
        if (createFactory) {
            IDXGIFactory1* factory = nullptr;
            if (SUCCEEDED(createFactory(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory))) && factory) {
                if (SUCCEEDED(factory->EnumAdapters1(static_cast<UINT>(adapterIndex), &adapter)) && adapter) {
                    DXGI_ADAPTER_DESC1 d{};
                    adapter->GetDesc1(&d);
                    log_("VRDisplay: headset adapter %d = %ls\n", adapterIndex, d.Description);
                }
                factory->Release();
            }
        }
        const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
        D3D_FEATURE_LEVEL got{};
        HRESULT hr = createDevice(adapter, adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                  levels, 3, D3D11_SDK_VERSION, &dev_, &got, &ctx_);
        if (adapter) adapter->Release();
        if (FAILED(hr) || !dev_ || !ctx_) { log_("VRDisplay: D3D11CreateDevice failed hr=0x%08lX\n", (unsigned long)hr); Shutdown(); return false; }
        log_("VRDisplay: D3D11 device created, feature level 0x%X\n", (unsigned)got);

        D3D11_TEXTURE2D_DESC td{};
        td.Width = w_; td.Height = h_;
        td.MipLevels = td.ArraySize = 1;
        td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;            // D3DFMT_A8R8G8B8 memory order
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        for (int i = 0; i < 2; ++i) {
            hr = dev_->CreateTexture2D(&td, nullptr, &eye_[i]);
            if (FAILED(hr)) { log_("VRDisplay: CreateTexture2D(eye %d, %ux%u) failed hr=0x%08lX\n", i, w_, h_, (unsigned long)hr); Shutdown(); return false; }
        }
        log_("VRDisplay: eye textures %ux%u BGRA8 created\n", w_, h_);
        return true;
    }

    ID3D11Device* device() const { return dev_; }
    ID3D11DeviceContext* context() const { return ctx_; }
    UINT width() const { return w_; }
    UINT height() const { return h_; }
    void* eyeTexture(int eye) const { return eye_[eye & 1]; }
    bool ready() const { return dev_ && ctx_ && eye_[0] && eye_[1]; }

    // bgra: w*h pixels, tightly packed rows of w*4 bytes.
    void Upload(int eye, const void* bgra)
    {
        ctx_->UpdateSubresource(eye_[eye & 1], 0, nullptr, bgra, w_ * 4, 0);
    }
    void Flush() { if (ctx_) ctx_->Flush(); }

    void Shutdown()
    {
        for (auto& t : eye_) if (t) { t->Release(); t = nullptr; }
        if (ctx_) { ctx_->Release(); ctx_ = nullptr; }
        if (dev_) { dev_->Release(); dev_ = nullptr; }
    }

private:
    ID3D11Device* dev_ = nullptr;
    ID3D11DeviceContext* ctx_ = nullptr;
    ID3D11Texture2D* eye_[2] = { nullptr, nullptr };
    UINT w_ = 0, h_ = 0;
    LogFn log_ = nullptr;
};
