Experimental VR mod for Warhammer Online: Age of Reckoning

## Maintained version
This mode was originally vibe-coded using Claude Sonnet 5.5 by Ross Scott.  It knowingly had shortcomings and was intended as a foundation for potentially making the mod better.  Since November 2026 Lucas Traba (lucastraba) maintains it here, with Ross's permission, so the updates and improvements since the original are Lucas's contribution.

-AI disclosure: the original version was completely vibe-coded (see the description below). The changes since then were also written with AI (Claude), and checked with unit tests, under Wine on Linux with a fake headset, and, where noted, by Ross on Windows with a real headset.

-Download: the Releases page of this repository (WARVR.dll, WARVRInjector.exe, WARVRConfig.exe, openvr_api.dll, an example WARVR.ini). The files in bin\ are Ross's original build.

-Stutter when a map first loads: with DXVK (below) the game can stutter for a while when you first load a map or see something new. That is DXVK preparing the game's shaders the first time they are used, not a crash; it settles down, and is usually less on later runs. More in docs\DXVK.md.

-DXVK instead of dgVoodoo: the fix for the CPU bottleneck. Ross measured 90 fps across the board on it, including where dgVoodoo dropped frames. See docs\DXVK.md. With your usual dgVoodoo setup the mod still works as before.

-Settings window: WARVRConfig.exe (next to the injector) sets the main options without editing WARVR.ini by hand: VR on when you enter the world, HUD style, laser, draw distance, lazy_state, the log, DXVK texture filtering and DXVK's own log. It only changes the lines it owns, so a hand-edited WARVR.ini keeps everything else. It can also set the time of day on your own ProjectWAR server, after a two-line server change (docs\SERVER_TIME.md).

-HUD shape: Ctrl+Shift+H switches the HUD between flat (as before), curved (bent around you, so the minimap and chat at the edges face you) and low (curved, lower and leaned back, a glance down). hud_style=flat|curved|low in WARVR.ini picks the one it starts with.

-Laser pointer: Ctrl+Shift+L (or laser=on in WARVR.ini) draws a red beam along the line the game itself uses for the mouse, ending in a dot on whatever the cursor is really pointing at. laser_origin=hand|eyes picks where the beam starts, laser_width=3 sets its width in pixels. VR only.

-lazy_state=on in WARVR.ini (with dgVoodoo): skips some of the extra work the mod does for every object.

-draw_distance now also works without VR (flat screen).

-Problems: set log=on in WARVR.ini (or tick it in WARVRConfig.exe) and open an issue on this repository with %TEMP%\WARVR.log.

-Building it yourself: build.bat (Visual Studio), or tools\build-cross.sh on Linux. tools\fakevr is a fake headset for testing without one.

## Description:
This mod will activate full 3D VR rendering when inside Warhammer Online and should scale to any resolution.

## Warning:
Running this mod on the "Return of Reckoning" server is NOT recommended!  You may get banned if you do.

## Requirements:
-A functional client of Warhammer Online: Age of Reckoning and a private or local server to connect to.

-VR headset capable of running OpenVR / SteamVR games. (tested on PCVR)

-Windows (it might be possible for Linux, but is completely untested)

## Known issues:
-No motion controls have been added, just default game controls. 

-The HUD operates as a floating plane and is not optimized for VR use, but is functional. 

-It is possible some effects may still be bugged for VR, it has not been thoroughly tested.

-Occasional client crashes when loading characters (or possible zones), it's recommended not to activate it until you're fully in the game and disable it before loading new zones. 

## Installation:
This mod assumes you have a Warhammer Online - Age of Reckoning server already running.  If not, unsupported instructions are included in extras\server emulator instructions.txt

1. Copy the following to your Warhammer client directory:
third_party\openvr\bin\win32\openvr_api.dll
bin\WarVRInjector.exe
bin\WARVR.dll
2. Download DXVK from here:
https://github.com/doitsujin/dxvk/releases
3. Extract d3d9.dll from the tar.gz release and place it in your Warhammer client directory.
4. Run WARVRInjector.exe prior to launching the game
5. Fully load a character in the game
6. Put on your VR headset.
7. Press Ctrl+Shift+Numpad* to enable / disable VR.  You may need to alt-tab back into the client.  Press again to disable.

## Additional features:
Creating WARVR.ini in the client directory has some additional options by adding these lines when the injector is launched:

draw_distance=1.0
(This extends the game draw distance beyond the default maximum.  1.3 looks good, but will reduce performance.

log=on
This outputs WARVR.log in your Windows temp directory, useful for diagnosing potential problems 

## Hotkeys:
Ctrl+Shift+Numpad* = activate / deactivate VR

Ctrl+Shift+M = show / hide mouse cursor

Ctrl+Shift+PageUp / Pagedown = change world scale

Ctrl+Shift+Del / End = move HUD forward or backwards in 3D space

Ctrl+Shift+ Numpad / = recenter headtracking

Ctrl+Shift+Numpad - = lock vertical camera motion so that your view stays level in VR, press again to disable

## Third-party software
- OpenVR SDK (openvr_capi.h, openvr_api.dll) - Copyright (c) 2015, Valve Corporation,
  BSD 3-Clause licence. https://github.com/ValveSoftware/openvr
  Full licence text: third_party/openvr/LICENSE

This project is an unofficial fan modification. It is not affiliated with or endorsed by
Valve, EA/Mythic, or the author of dgVoodoo2. SteamVR and OpenVR are trademarks of Valve.


