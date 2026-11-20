// gpupath.h - hand frames to SteamVR without touching the CPU.
//
// WAR renders with Direct3D 9, but through dgVoodoo2, which translates every D3D9 call into Direct3D 11 on a device
// we do not own. A plain D3D9 device cannot share surfaces with D3D11, but we do not need sharing: if we get hold of
// dgVoodoo's own ID3D11Device / ID3D11DeviceContext we can
//     1. read the render target it currently has bound (that is the game's main target),
//     2. CopySubresourceRegion each eye's half into a texture created on THAT SAME device (a pure GPU copy),
//     3. give those textures to the OpenVR compositor (it accepts a texture from any D3D11 device).
//
// HOW WE GET THE DEVICE (phase 7b):
//   Phase 7a patched a vtable entry of a dummy context of our own and waited for dgVoodoo to call through it. On the
//   test machine that never fired (hits=0): the vtable of our dummy lives in private memory, i.e. something wraps the
//   D3D11 objects, so the game's context does not share it. Phase 7b instead hooks the two creation functions in
//   d3d11.dll (D3D11CreateDevice and D3D11CreateDeviceAndSwapChain) and keeps the device + immediate context that
//   dgVoodoo is handed back. No vtable assumption is involved, so it works whatever wrappers are in between.
//   The old vtable hook stays as a second chance for the case where this DLL was injected after the device existed.
//   If dgVoodoo is set to its Direct3D 12 output there is no D3D11 device at all; the caller reports that.
#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdint>
#include <atomic>
#include "hookutil.h"

namespace gpu {

using LogFn = void (*)(const char* fmt, ...);
using OMSetRT_t = void (STDMETHODCALLTYPE*)(ID3D11DeviceContext*, UINT, ID3D11RenderTargetView* const*, ID3D11DepthStencilView*);
using OMSetRTUAV_t = void (STDMETHODCALLTYPE*)(ID3D11DeviceContext*, UINT, ID3D11RenderTargetView* const*, ID3D11DepthStencilView*,
                                               UINT, UINT, ID3D11UnorderedAccessView* const*, const UINT*);

constexpr size_t kSlotOMSetRenderTargets = 33;      // ID3D11DeviceContext vtable slots, taken from the real d3d11.h
constexpr size_t kSlotOMSetRenderTargetsAndUAV = 34;
constexpr int kMaxCandidates = 6;

struct Candidate {
    ID3D11Device* dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;                  // immediate context (we hold a reference)
    UINT flags = 0;
    int driverType = 0;
    const char* how = "";
};

struct State {
    LogFn log = nullptr;
    ID3D11Device* ownDev = nullptr;
    ID3D11DeviceContext* ownCtx = nullptr;
    Candidate cand[kMaxCandidates];
    int nCand = 0;
    ID3D11DeviceContext* preferred = nullptr;            // the candidate that last produced a good copy
    SRWLOCK lock = SRWLOCK_INIT;
    OMSetRT_t origOM = nullptr;
    OMSetRTUAV_t origOMUAV = nullptr;
    bool installed = false;                              // vtable hook (second chance)
    bool createHooked = false;                           // creation hooks
    unsigned hits = 0, learned = 0, createCalls = 0, createCaptured = 0;
    // eye textures, created on the device of the candidate that works
    ID3D11Device* eyeDev = nullptr;
    ID3D11DeviceContext* eyeCtx = nullptr;
    ID3D11Texture2D* eye[2][2] = {};                     // [double-buffer set][eye]
    UINT eyeW = 0, eyeH = 0;
    DXGI_FORMAT eyeFmt = DXGI_FORMAT_UNKNOWN;
    ID3D11Query* query = nullptr;
    bool queryPending = false;
    int64_t waitTicks = 0;                               // QPC ticks the CPU spent waiting for the previous frame's GPU copy (diagnostics)
};
inline State g_s;

struct Lock { Lock() { AcquireSRWLockExclusive(&g_s.lock); } ~Lock() { ReleaseSRWLockExclusive(&g_s.lock); } };

// ---------------------------------------------------------------- candidate list
// Remember a device + immediate context somebody created (the newest is tried first). Takes its own references.
inline void AddCandidate(ID3D11Device* dev, ID3D11DeviceContext* ctxIn, UINT flags, int driverType, const char* how)
{
    if (!dev) return;
    ID3D11DeviceContext* ctx = ctxIn;
    if (ctx) ctx->AddRef(); else dev->GetImmediateContext(&ctx);      // GetImmediateContext adds a reference
    if (!ctx) return;
    Lock l;
    for (int i = 0; i < g_s.nCand; ++i) if (g_s.cand[i].ctx == ctx) { ctx->Release(); return; }   // already known
    if (g_s.nCand == kMaxCandidates) {                                // drop the oldest
        g_s.cand[0].ctx->Release(); g_s.cand[0].dev->Release();
        for (int i = 1; i < kMaxCandidates; ++i) g_s.cand[i - 1] = g_s.cand[i];
        --g_s.nCand;
    }
    dev->AddRef();
    g_s.cand[g_s.nCand++] = { dev, ctx, flags, driverType, how };
    ++g_s.learned;
    if (g_s.log) g_s.log("GPU path: remembered D3D11 device %p / immediate context %p (%s; create flags 0x%X, driver type %d, now %d candidate(s))\n",
                         (void*)dev, (void*)ctx, how, flags, driverType, g_s.nCand);
}

// Remove (and release) a device again - used for OUR OWN device, which is created through the same hooked function.
inline void ForgetDevice(ID3D11Device* dev)
{
    if (!dev) return;
    Lock l;
    int w = 0;
    for (int i = 0; i < g_s.nCand; ++i) {
        if (g_s.cand[i].dev == dev) {
            if (g_s.preferred == g_s.cand[i].ctx) g_s.preferred = nullptr;
            g_s.cand[i].ctx->Release(); g_s.cand[i].dev->Release();
        } else g_s.cand[w++] = g_s.cand[i];
    }
    g_s.nCand = w;
}

inline bool HaveGame() { Lock l; return g_s.nCand > 0; }
inline int CandidateCount() { Lock l; return g_s.nCand; }

// ---------------------------------------------------------------- vtable hook (second chance, as in phase 7a)
inline void Learn(ID3D11DeviceContext* self)
{
    ++g_s.hits;
    if (self == g_s.ownCtx) return;
    if (self->GetType() != D3D11_DEVICE_CONTEXT_IMMEDIATE) return;
    ID3D11Device* dev = nullptr;
    self->GetDevice(&dev);
    if (!dev) return;
    if (dev == g_s.ownDev) { dev->Release(); return; }
    AddCandidate(dev, self, 0, 0, "seen through the context vtable hook");
    dev->Release();
}

inline void STDMETHODCALLTYPE HookOM(ID3D11DeviceContext* self, UINT n, ID3D11RenderTargetView* const* rtv, ID3D11DepthStencilView* dsv)
{
    Learn(self);
    g_s.origOM(self, n, rtv, dsv);
}

inline void STDMETHODCALLTYPE HookOMUAV(ID3D11DeviceContext* self, UINT n, ID3D11RenderTargetView* const* rtv, ID3D11DepthStencilView* dsv,
                                        UINT uavStart, UINT numUav, ID3D11UnorderedAccessView* const* uav, const UINT* counts)
{
    Learn(self);
    g_s.origOMUAV(self, n, rtv, dsv, uavStart, numUav, uav, counts);
}

// ownCtx is the immediate context of a dummy device WE created (same runtime => normally the same vtable as the game's).
inline bool InstallHooks(ID3D11Device* ownDev, ID3D11DeviceContext* ownCtx, LogFn log, void*** vtableOut = nullptr)
{
    g_s.log = log; g_s.ownDev = ownDev; g_s.ownCtx = ownCtx;
    if (!ownCtx) return false;
    void** vt = *reinterpret_cast<void***>(ownCtx);
    if (vtableOut) *vtableOut = vt;
    if (g_s.installed) return true;
    DWORD old = 0;
    if (!VirtualProtect(&vt[kSlotOMSetRenderTargets], 2 * sizeof(void*), PAGE_READWRITE, &old)) {
        if (log) log("GPU path: VirtualProtect on the D3D11 context vtable failed (%lu)\n", GetLastError());
        return false;
    }
    g_s.origOM = reinterpret_cast<OMSetRT_t>(vt[kSlotOMSetRenderTargets]);
    g_s.origOMUAV = reinterpret_cast<OMSetRTUAV_t>(vt[kSlotOMSetRenderTargetsAndUAV]);
    vt[kSlotOMSetRenderTargets] = reinterpret_cast<void*>(&HookOM);
    vt[kSlotOMSetRenderTargetsAndUAV] = reinterpret_cast<void*>(&HookOMUAV);
    DWORD ignored = 0;
    VirtualProtect(&vt[kSlotOMSetRenderTargets], 2 * sizeof(void*), old, &ignored);
    g_s.installed = true;
    return true;
}

// ---------------------------------------------------------------- creation hooks (phase 7b, the main way)
using CreateDevice_t = HRESULT (WINAPI*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL*, UINT, UINT,
                                         ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
using CreateDeviceSC_t = HRESULT (WINAPI*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, const D3D_FEATURE_LEVEL*, UINT, UINT,
                                           const DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
inline CreateDevice_t g_origCreate = nullptr;
inline CreateDeviceSC_t g_origCreateSC = nullptr;

// 5-byte jmp detour with a trampoline (32-bit x86). Returns false (and logs the bytes) if the prologue is not understood.
inline bool InlineHook(void* target, void* hook, void** outTramp, LogFn log, const char* name)
{
    BYTE* src = reinterpret_cast<BYTE*>(target);
    char got[64]{};
    for (int i = 0; i < 12; ++i) _snprintf_s(got + i * 3, 4, _TRUNCATE, "%02X ", src[i]);
    int relOff = -1;
    const size_t len = hk::PrologueLength(src, &relOff);
    if (log) log("InlineHook(%s): function at %p starts with %s -> relocatable prologue length %u\n", name, target, got, (unsigned)len);
    if (!len) return false;
    BYTE* tramp = reinterpret_cast<BYTE*>(VirtualAlloc(nullptr, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!tramp) return false;
    hk::BuildTrampoline(src, (uint32_t)(uintptr_t)src, tramp, (uint32_t)(uintptr_t)tramp, len, relOff);
    DWORD old = 0;
    if (!VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &old)) return false;
    BYTE patch[16]; memset(patch, 0x90, sizeof(patch));
    patch[0] = 0xE9;
    const int32_t rel = (int32_t)((uintptr_t)hook - ((uintptr_t)src + 5));
    memcpy(patch + 1, &rel, 4);
    *outTramp = tramp;                                        // publish the trampoline before the jump goes live
    memcpy(src, patch, len);
    FlushInstructionCache(GetCurrentProcess(), src, len);
    DWORD ignored = 0; VirtualProtect(src, len, old, &ignored);
    return true;
}

inline void Captured(ID3D11Device** ppDev, ID3D11DeviceContext** ppCtx, UINT flags, D3D_DRIVER_TYPE dt, const char* api)
{
    ++g_s.createCalls;
    if (!ppDev || !*ppDev) return;                            // a feature-level query only, nothing was created
    ++g_s.createCaptured;
    AddCandidate(*ppDev, ppCtx ? *ppCtx : nullptr, flags, (int)dt, api);
}

inline HRESULT WINAPI HookD3D11CreateDevice(IDXGIAdapter* a, D3D_DRIVER_TYPE dt, HMODULE sw, UINT flags, const D3D_FEATURE_LEVEL* fl, UINT nfl,
                                            UINT sdk, ID3D11Device** ppDev, D3D_FEATURE_LEVEL* pfl, ID3D11DeviceContext** ppCtx)
{
    const HRESULT hr = g_origCreate(a, dt, sw, flags, fl, nfl, sdk, ppDev, pfl, ppCtx);
    if (SUCCEEDED(hr)) Captured(ppDev, ppCtx, flags, dt, "D3D11CreateDevice");
    return hr;
}

inline HRESULT WINAPI HookD3D11CreateDeviceAndSwapChain(IDXGIAdapter* a, D3D_DRIVER_TYPE dt, HMODULE sw, UINT flags, const D3D_FEATURE_LEVEL* fl, UINT nfl,
                                                        UINT sdk, const DXGI_SWAP_CHAIN_DESC* sd, IDXGISwapChain** ppSC, ID3D11Device** ppDev,
                                                        D3D_FEATURE_LEVEL* pfl, ID3D11DeviceContext** ppCtx)
{
    const HRESULT hr = g_origCreateSC(a, dt, sw, flags, fl, nfl, sdk, sd, ppSC, ppDev, pfl, ppCtx);
    if (SUCCEEDED(hr)) Captured(ppDev, ppCtx, flags, dt, "D3D11CreateDeviceAndSwapChain");
    return hr;
}

// Call once, as early as possible (before dgVoodoo creates its device).
inline bool InstallCreateHooks(LogFn log)
{
    g_s.log = log;
    if (g_s.createHooked) return true;
    HMODULE d3d11 = LoadLibraryA("d3d11.dll");
    if (!d3d11) { if (log) log("GPU path: cannot load d3d11.dll for the creation hooks\n"); return false; }
    void* f1 = reinterpret_cast<void*>(GetProcAddress(d3d11, "D3D11CreateDevice"));
    void* f2 = reinterpret_cast<void*>(GetProcAddress(d3d11, "D3D11CreateDeviceAndSwapChain"));
    bool ok1 = f1 && InlineHook(f1, reinterpret_cast<void*>(&HookD3D11CreateDevice), reinterpret_cast<void**>(&g_origCreate), log, "D3D11CreateDevice");
    bool ok2 = f2 && InlineHook(f2, reinterpret_cast<void*>(&HookD3D11CreateDeviceAndSwapChain), reinterpret_cast<void**>(&g_origCreateSC), log, "D3D11CreateDeviceAndSwapChain");
    g_s.createHooked = ok1 || ok2;
    if (log) log("GPU path: D3D11 creation hooks: CreateDevice=%s CreateDeviceAndSwapChain=%s (d3d11.dll at %p)\n", ok1 ? "ok" : "NOT hooked", ok2 ? "ok" : "NOT hooked", (void*)d3d11);
    return g_s.createHooked;
}

// ---------------------------------------------------------------- eye textures + copy
inline void ReleaseEyes()
{
    for (auto& set : g_s.eye) for (auto& t : set) if (t) { t->Release(); t = nullptr; }
    if (g_s.query) { g_s.query->Release(); g_s.query = nullptr; }
    if (g_s.eyeCtx) { g_s.eyeCtx->Release(); g_s.eyeCtx = nullptr; }
    if (g_s.eyeDev) { g_s.eyeDev->Release(); g_s.eyeDev = nullptr; }
    g_s.queryPending = false;
    g_s.eyeW = g_s.eyeH = 0; g_s.eyeFmt = DXGI_FORMAT_UNKNOWN;
}

// Forget everything about the game's device (it may be recreated, e.g. after an alt-tab).
inline void Invalidate()
{
    ReleaseEyes();
    Lock l;
    for (int i = 0; i < g_s.nCand; ++i) { g_s.cand[i].ctx->Release(); g_s.cand[i].dev->Release(); g_s.cand[i] = Candidate{}; }
    g_s.nCand = 0; g_s.preferred = nullptr;
}

inline bool SupportedFormat(DXGI_FORMAT f, DXGI_FORMAT* eyeFmt)
{
    switch (f) {
    case DXGI_FORMAT_B8G8R8A8_UNORM: case DXGI_FORMAT_B8G8R8A8_TYPELESS: case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: *eyeFmt = DXGI_FORMAT_B8G8R8A8_UNORM; return true;
    case DXGI_FORMAT_R8G8B8A8_UNORM: case DXGI_FORMAT_R8G8B8A8_TYPELESS: case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: *eyeFmt = DXGI_FORMAT_R8G8B8A8_UNORM; return true;
    case DXGI_FORMAT_B8G8R8X8_UNORM: case DXGI_FORMAT_B8G8R8X8_TYPELESS: case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: *eyeFmt = DXGI_FORMAT_B8G8R8X8_UNORM; return true;
    default: return false;
    }
}

inline bool EnsureEyes(ID3D11Device* dev, ID3D11DeviceContext* ctx, UINT w, UINT h, DXGI_FORMAT fmt)
{
    if (g_s.eye[0][0] && g_s.eyeDev == dev && g_s.eyeW == w && g_s.eyeH == h && g_s.eyeFmt == fmt) return true;
    ReleaseEyes();
    D3D11_TEXTURE2D_DESC td{};
    td.Width = w; td.Height = h; td.MipLevels = td.ArraySize = 1; td.Format = fmt; td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT; td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    for (auto& set : g_s.eye) for (auto& t : set) {
        HRESULT hr = dev->CreateTexture2D(&td, nullptr, &t);
        if (FAILED(hr)) {                                           // some formats cannot be render targets
            D3D11_TEXTURE2D_DESC t2 = td; t2.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            hr = dev->CreateTexture2D(&t2, nullptr, &t);
        }
        if (FAILED(hr)) { if (g_s.log) g_s.log("GPU path: CreateTexture2D(%ux%u fmt %d) on the game's device failed hr=0x%08lX\n", w, h, (int)fmt, (unsigned long)hr); ReleaseEyes(); return false; }
    }
    D3D11_QUERY_DESC qd{}; qd.Query = D3D11_QUERY_EVENT;
    dev->CreateQuery(&qd, &g_s.query);                              // optional: bounds how far the CPU may run ahead
    dev->AddRef(); ctx->AddRef(); g_s.eyeDev = dev; g_s.eyeCtx = ctx;
    g_s.eyeW = w; g_s.eyeH = h; g_s.eyeFmt = fmt;
    if (g_s.log) g_s.log("GPU path: eye textures %ux%u (format %d) x2 eyes x2 buffers created on the game's device %p (%.0f MB)\n", w, h, (int)fmt, (void*)dev, 4.0 * w * h * 4.0 / 1048576.0);
    return true;
}

struct CopyResult { bool ok = false; UINT srcW = 0, srcH = 0; DXGI_FORMAT srcFmt = DXGI_FORMAT_UNKNOWN; const char* why = ""; };

// One candidate: copy the left and right halves of ITS currently bound render target into the eye textures of buffer `set`.
inline CopyResult TryCopy(const Candidate& c, int set, UINT expectW, UINT expectH)
{
    CopyResult r;
    ID3D11DeviceContext* ctx = c.ctx;
    ID3D11RenderTargetView* rtv = nullptr; ID3D11DepthStencilView* dsv = nullptr;
    ctx->OMGetRenderTargets(1, &rtv, &dsv);
    if (dsv) dsv->Release();
    if (!rtv) { r.why = "no render target bound on this context"; return r; }
    ID3D11Resource* res = nullptr;
    rtv->GetResource(&res); rtv->Release();
    ID3D11Texture2D* tex = nullptr;
    if (res) { res->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&tex)); res->Release(); }
    if (!tex) { r.why = "bound target is not a 2D texture"; return r; }
    D3D11_TEXTURE2D_DESC d{}; tex->GetDesc(&d);
    r.srcW = d.Width; r.srcH = d.Height; r.srcFmt = d.Format;
    DXGI_FORMAT eyeFmt = DXGI_FORMAT_UNKNOWN;
    // The D3D9 side believes the main target is expectW x expectH. Compare the ASPECT so that a target dgVoodoo scales internally still matches.
    if (d.SampleDesc.Count != 1) r.why = "bound target is multisampled";
    else if ((uint64_t)d.Width * expectH != (uint64_t)expectW * d.Height) r.why = "bound target has a different shape than the game's main target";
    else if (!SupportedFormat(d.Format, &eyeFmt)) r.why = "unsupported render target format";
    else if (!EnsureEyes(c.dev, ctx, d.Width / 2, d.Height, eyeFmt)) r.why = "could not create eye textures";
    else {
        if (g_s.query && g_s.queryPending) {                        // the previous frame's copy must be finished before its buffer is reused
            LARGE_INTEGER w0, w1; QueryPerformanceCounter(&w0);
            for (int i = 0; i < 4000 && ctx->GetData(g_s.query, nullptr, 0, 0) == S_FALSE; ++i) Sleep(0);
            QueryPerformanceCounter(&w1); g_s.waitTicks += w1.QuadPart - w0.QuadPart;
            g_s.queryPending = false;
        }
        const UINT hw = d.Width / 2;
        for (int e = 0; e < 2; ++e) {
            D3D11_BOX box{ (UINT)e * hw, 0, 0, (UINT)e * hw + hw, d.Height, 1 };
            ctx->CopySubresourceRegion(g_s.eye[set & 1][e], 0, 0, 0, 0, tex, 0, &box);
        }
        if (g_s.query) { ctx->End(g_s.query); g_s.queryPending = true; }
        ctx->Flush();
        r.ok = true;
    }
    tex->Release();
    return r;
}

// Try the remembered good candidate first, then the others (newest first). The first one that has the game's main
// target bound wins and is remembered.
inline CopyResult CopyEyes(int set, UINT expectW, UINT expectH)
{
    Candidate list[kMaxCandidates]; int n = 0;
    {
        Lock l;
        for (int i = g_s.nCand - 1; i >= 0; --i) {
            Candidate c = g_s.cand[i];
            c.ctx->AddRef(); c.dev->AddRef();
            if (c.ctx == g_s.preferred) { for (int j = n; j > 0; --j) list[j] = list[j - 1]; list[0] = c; }
            else list[n] = c;
            ++n;
        }
    }
    CopyResult best; best.why = "game's D3D11 context not learned yet";
    for (int i = 0; i < n; ++i) {
        CopyResult r = TryCopy(list[i], set, expectW, expectH);
        if (r.ok) { Lock l; g_s.preferred = list[i].ctx; best = r; break; }
        if (i == 0 || r.srcW) best = r;                             // keep the most informative failure
    }
    for (int i = 0; i < n; ++i) { list[i].ctx->Release(); list[i].dev->Release(); }
    return best;
}

inline void* Eye(int set, int eye) { return g_s.eye[set & 1][eye & 1]; }

} // namespace gpu
