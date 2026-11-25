# WAR VR

Experimental VR mod for Warhammer Online: Age of Reckoning. It renders the game in full 3D on any OpenVR / SteamVR headset; you play with the usual mouse and keyboard.

**Do not use it on the Return of Reckoning server.** You may get banned.

## Requirements

- Windows.
- A Warhammer Online: Age of Reckoning client and a private server to connect to. If you don't have a server, Ross's unsupported setup notes are in [extras/server emulator instructions.txt](extras/server%20emulator%20instructions.txt).
- A headset that runs SteamVR games.
- A graphics card and driver with Vulkan 1.3 (for DXVK, which is included).

## Install

1. Download the zip from [Releases](https://github.com/accursed-farms/warhammer-online-vr/releases/latest).
2. Unzip it into your Warhammer folder (the one with WAR.exe). Overwrite if asked.

That's it. To uninstall, delete `dinput8.dll` from that folder.

## Play

1. Start SteamVR.
2. Start the game and log in.
3. Put the headset on. VR switches on by itself a few seconds after your character is in the world.

The first minutes on a map can stutter. That's DXVK preparing the game's shaders the first time it sees them; it settles down and is usually better on later runs.

## Hotkeys

Hold Ctrl+Shift and tap:

| Key | Does |
|---|---|
| Numpad * | VR on / off |
| Numpad / | Recenter the view |
| H | HUD style: flat, curved, low |
| End / Delete | HUD closer / farther |
| PageUp / PageDown | World bigger / smaller |
| Numpad - | Lock the view level (the mouse stops tilting it; the headset does) |
| M | Mouse cursor on / off |
| L | Laser pointer on / off |

No numpad? Every key can be changed in `WARVR.ini`. See `WARVR.ini.example`.

## Settings

Run `WARVRConfig.exe` for the main options: VR on by itself, HUD style, laser pointer, draw distance, texture filtering, the log. It only changes its own lines in `WARVR.ini`, so anything you edited by hand stays.

Everything else is in `WARVR.ini.example`; copy it to `WARVR.ini` and uncomment what you want.

## Known issues

- No motion controllers; mouse and keyboard only.
- The HUD is a floating panel and isn't designed for VR yet.
- Some effects may still look wrong in VR. It hasn't been tested thoroughly.
- The game can crash while loading a character or a zone with VR on. If that happens, turn VR off (Ctrl+Shift+Numpad *) before changing zones.

## Problems

Tick "Write WARVR.log" in `WARVRConfig.exe` (or put `log=on` in `WARVR.ini`), play until it goes wrong, then [open an issue](https://github.com/accursed-farms/warhammer-online-vr/issues) and attach `%TEMP%\WARVR.log`.

## What's new

### 2026.11.24
- **One-step install:** unzip into the game folder and play. The game loads the mod by itself, so the injector is gone.
- **VR switches on by itself** once you're in the world. Turn it off with `auto_vr=off` or in `WARVRConfig.exe`.
- **DXVK is included** (32-bit `d3d9.dll`), so there's nothing extra to download.
- The mod file is now `dinput8.dll` instead of `WARVR.dll`. If you're updating, delete `WARVR.dll` and `WARVRInjector.exe`.

### 2026.11.23
- **DXVK instead of dgVoodoo:** fixes the CPU bottleneck. Ross measured 90 fps across the board on it.
- **Settings window:** `WARVRConfig.exe`.
- **Laser pointer:** Ctrl+Shift+L.
- **HUD styles:** Ctrl+Shift+H for flat, curved or low.
- `draw_distance` works without VR too; `lazy_state` for dgVoodoo; time of day on your own server ([docs/SERVER_TIME.md](docs/SERVER_TIME.md)).

## About

Ross Scott made the original version by vibe-coding it with Claude, as a starting point for making the mod better. Since November 2026 Lucas Traba maintains it, with Ross's permission, so the changes since then are on Lucas. They were also written with AI (Claude) and are checked with unit tests, under Wine on Linux with a fake headset, and by Ross on Windows with a real headset where noted.

Contributions are welcome. To build it yourself: `build.bat` with Visual Studio, or `tools/build-cross.sh` on Linux. `tools/fakevr` is a fake headset for testing without one.

## Old method: dgVoodoo

Before DXVK, the mod ran on dgVoodoo, which is slower in VR (it shares the game's one busy CPU thread). It still works if you need it:

1. Install the mod as above, then delete the `d3d9.dll` that came with it.
2. Download dgVoodoo2, version 2.78.2 or older (the mod was made with 2.78.1): [releases](https://github.com/dege-diosg/dgVoodoo2/releases), or [2.78.1 directly](https://www.t2e.pl/t2e-download/dgvoodoo2.1046).
3. Copy everything from dgVoodoo's `MS\x86` folder into your Warhammer folder.
4. Run `dgVoodooCpl.exe`, click "Add" and pick your Warhammer folder. On the DirectX tab, set VRAM to 4096 and (optional) untick the watermark. Press Apply.

Ross's original build (`WARVR.dll` with `WARVRInjector.exe`) is attached to the [original release](https://github.com/accursed-farms/warhammer-online-vr/releases/tag/original).

## Third-party software

- **OpenVR** (`openvr_api.dll`): Copyright (c) 2015 Valve Corporation, BSD 3-Clause. https://github.com/ValveSoftware/openvr
- **DXVK** (`d3d9.dll`): Copyright (c) 2017 Philip Rebohle and contributors, zlib licence. https://github.com/doitsujin/dxvk

Their licence texts are in `WARVR-docs/licenses` in the zip.

This is an unofficial fan modification, not affiliated with or endorsed by Valve, EA/Mythic, or the authors of DXVK or dgVoodoo2. SteamVR and OpenVR are trademarks of Valve.
