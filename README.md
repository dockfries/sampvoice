# **SAMPVOICE** [Original repository](https://github.com/CyberMor/sampvoice)
English | [Русский](https://github.com/AmyrAhmady/sampvoice/blob/master/README.ru.md)

## Changes in this fork
---------------------------------
This fork adds several client-side improvements on top of the open.mp port:

* **External UI resources** — the client loads fonts, icons, the blur shader and
  language packs from `resources/` and `languages/` folders placed next to the
  `.asi`, instead of embedding them. No UI resource is compiled into the binary.
  * `resources/font.ttf` (or `font.otf`) can be replaced by the user to change
    the UI font. The baked glyph atlas covers Latin, Cyrillic, Greek, Hebrew,
    Arabic (glyphs only, no RTL shaping), full CJK, Japanese kana, Korean and
    Thai.
* **Runtime language packs** — menu strings are loaded from
  `languages/<name>.json` (UTF-8, key/value) with an English fallback. A
  Language selector in the General tab switches packs at runtime and persists
  the choice. Packaged languages: English, Русский, Srpski, Bahasa Indonesia,
  Português (Brasil), 简体中文.
* **Multi-byte input & display** — `WM_CHAR`/`WM_IME_CHAR` are fed straight
  into ImGui (IME-composed Chinese input works), player nicknames are converted
  from the system ANSI code page to UTF-8 for display, and the blacklist filter
  normalizes the input to the system code page before matching.
* **Mic availability fix** — recording devices are enumerated locally at game
  load instead of only during the server handshake, so the menu no longer
  reports "No microphones available" when the server lacks the plugin.
* **Push-to-talk latency** — per-keypress debug logging removed and the BASS
  recording channel is kept running while the mic is enabled, draining stale
  data instead of pausing/playing on every key press.
* **omp-cef compatibility** — the render lifecycle was refactored: window
  messages use `SetWindowSubclass` instead of `SetWindowLong`, the
  `Direct3DCreate9` call site is hooked with byte validation, and the D3D9
  wrappers use internal COM reference counting.
* **Dear ImGui upgraded to v1.92.9b** (from 1.68 WIP) and adapted to the new
  API (`BeginChild(ImGuiChildFlags_FrameStyle)`, `ImTextureRef`, atlas
  self-management, font data kept alive for the atlas lifetime).

## Description
---------------------------------
**SAMPVOICE** - is a Software Development Kit (SDK) for implementing voice communication systems in the Pawn language for open.mp servers.

#### Version support
----------------------------------
* Client: SA:MP 0.3.7 (R1, R3-1, R5-1, DL-1)
* Server: Latest open.mp version

## Features
---------------------------------
* Controlled voice transmission
* Player microphone control
* Binding a voice stream to game objects
* And many more features...

## Installation
---------------------------------
For the plugin to work, it must be installed by the players and on the server. There is a client and server part of the plugin for this.

#### For players
---------------------------------
Players have access to 2 installation options: automatic (via the installer) and manual (via the archive).

##### Automatically
---------------------------------
1. In order to download the installer, head over to [the `releases` page](https://github.com/AmyrAhmady/sampvoice/releases) and choose the desired version of the plugin.
2. After downloading, launch the installer and choose the desired language for your installation, afterwards the installer will automatically find your GTA San Andreas folder.
3. If the directory is correct, click "OK" and wait for the installation to complete. After the installation is complete, the installer will exit.

##### Manually
---------------------------------
1. Head over [the `releases` page](https://github.com/AmyrAhmady/sampvoice/releases) and download the archive with the desired client version.
2. Extract the archive to your GTA San Andreas folder.

#### For developers
---------------------------------
1. Download from [the `releases` page](https://github.com/AmyrAhmady/sampvoice/releases) the desired version of the plugin for your platform.
2. Unpack the archive to the root directory of the server.
3. Add to the *server.cfg* server configuration file the line *"plugins sampvoice"* for *Win32* and *"plugins sampvoice.so"* for *Linux x86*. **(If you have a Pawn.RakNet plugin be sure to place SampVoice after it)**

## Usage
---------------------------------
To get started using the plugin, read the documentation that comes with the server side. To do this, open the *sampvoice.chm* file using the Windows reference. **(If the documentation does not open, right-click on the documentation file, then Properties -> Unblock -> OK)**

To start using the plugin functionality, include the header file:
```php
#include <sampvoice>
```

#### Quick reference
---------------------------------
You need to know that the plugin uses its own type and constant system. Despite the fact that this is just a wrapper over the basic types of Pawn, it helps to navigate the types of the plugin itself and not to confuse pointers.

In order to redirect audio traffic from player A to player B, you need to create an audio stream (for example, a global one, using **SvCreateGStream**), then attach it to the stream of player A as a speaker (using **SvAttachSpeakerToStream**), after which attach to player B's stream as a listener (using **SvAttachListenerToStream**). Done. Now, when player A's microphone is activated (for example, with the **SvStartRecord** function), his audio traffic will be transmitted and then heard by player B.

Sound streams are pretty handy. They can be visualized using the example of Discord:
* A stream is an analogue of a room (or channel).
* Speakers are participants in the room with mute but microphone on.
* Listeners are participants in the room with their microphone mute but mute.

Players can be both speakers and listeners at the same time. In this case, the player's audio traffic will not be forwarded to him.

#### Example
---------------------------------
Let's take a look at some of the plugin's features with a practical example. Below we will create a server that will bind all connected players to the global stream, and also create a local stream for each player. Thus, players will be able to communicate through the global (heard equally at any point on the map) and local (heard only near the player) chats.
```cpp
#include <sampvoice>

new SV_GSTREAM:gstream = SV_NULL;
new SV_LSTREAM:lstream[MAX_PLAYERS] = { SV_NULL, ... };

/*
    The public OnPlayerActivationKeyPress and OnPlayerActivationKeyRelease
    are needed in order to redirect the player's audio traffic to the
    corresponding streams when the corresponding keys are pressed.
*/

public SV_VOID:OnPlayerActivationKeyPress(SV_UINT:playerid, SV_UINT:keyid) 
{
    // Attach player to local stream as speaker if 'B' key is pressed
    if (keyid == 0x42 && lstream[playerid]) SvAttachSpeakerToStream(lstream[playerid], playerid);
    // Attach the player to the global stream as a speaker if the 'Z' key is pressed
    if (keyid == 0x5A && gstream) SvAttachSpeakerToStream(gstream, playerid);
}

public SV_VOID:OnPlayerActivationKeyRelease(SV_UINT:playerid, SV_UINT:keyid)
{
    // Detach the player from the local stream if the 'B' key is released
    if (keyid == 0x42 && lstream[playerid]) SvDetachSpeakerFromStream(lstream[playerid], playerid);
    // Detach the player from the global stream if the 'Z' key is released
    if (keyid == 0x5A && gstream) SvDetachSpeakerFromStream(gstream, playerid);
}

public OnPlayerConnect(playerid)
{
    // Checking for plugin availability
    if (SvGetVersion(playerid) == SV_NULL)
    {
        SendClientMessage(playerid, -1, "Could not find plugin sampvoice.");
    }
    // Checking for a microphone
    else if (SvHasMicro(playerid) == SV_FALSE)
    {
        SendClientMessage(playerid, -1, "The microphone could not be found.");
    }
    // Create a local stream with an audibility distance of 40.0, an unlimited number of listeners
    // and the name 'Local' (the name 'Local' will be displayed in red in the players' speakerlist)
    else if ((lstream[playerid] = SvCreateDLStreamAtPlayer(40.0, SV_INFINITY, playerid, 0xff0000ff, "Local")))
    {
        SendClientMessage(playerid, -1, "Press Z to talk to global chat and B to talk to local chat.");

        // Attach the player to the global stream as a listener
        if (gstream) SvAttachListenerToStream(gstream, playerid);

        // Assign microphone activation keys to the player
        SvAddKey(playerid, 0x42);
        SvAddKey(playerid, 0x5A);
    }
}

public OnPlayerDisconnect(playerid, reason)
{
    // Removing the player's local stream after disconnecting
    if (lstream[playerid])
    {
        SvDetachListenerFromStream(lstream[playerid], playerid);
        SvDetachSpeakerFromStream(lstream[playerid], playerid);
        SvDeleteStream(lstream[playerid]);
        lstream[playerid] = SV_NULL;
    }
}

public OnGameModeInit()
{
    // Uncomment the line to enable debug mode
    // SvDebug(SV_TRUE);

    gstream = SvCreateGStream(0xffff0000, "Global");
}

public OnGameModeExit()
{
    if (gstream)
    {
        for (new i = 0; i < MAX_PLAYERS; i ++)
        {
            if (!IsPlayerConnected(i)) continue;
            SvDetachListenerFromStream(gstream, i);
            SvDetachSpeakerFromStream(gstream, i);
        }
        SvDeleteStream(gstream);
    }
}
```

#### Channel Bitmask Routing
---------------------------------
The plugin supports **32 independent audio channels** (bits 0–31) for fine-grained voice routing control. This adds an extra dimension beyond streams: a key press, a speaker attachment, and a stream each carry a channel mask, and voice is forwarded only if all three overlap.

**Key concepts:**

| Term | Meaning |
|------|---------|
| `channelmask` | A `uint32_t` bitmask. Bit 0 = channel 0 (`0x01`), bit 1 = channel 1 (`0x02`), … bit 31 = channel 31 (`0x80000000`) |
| `SvSetKeyWithChannels(player, key, mask)` | When the player presses `key`, the channels in `mask` become **active** |
| `SvAttachSpeakerToStreamWithChannels(stream, player, mask)` | Only voice on channels from `mask` will pass into `stream` |
| `SvEnableSpeaker(player, mask)` / `SvDisableSpeaker(player, mask)` | Pawn‑side gate — restricts which channels the player is allowed to use (default: all). `SvCheckSpeaker(player, mask)` queries it |
| **Forward rule** | `(activeCh & enabledCh & chMask) != 0` — all three masks must intersect |

**Example — 3‑channel setup:**

```pawn
#define CH_GLOBAL   0b0001  // channel 0
#define CH_TEAM     0b0010  // channel 1
#define CH_SQUAD    0b0100  // channel 2

new gStream = SvCreateStream(0.0);           // global music
new tStream = SvCreateStream(30.0);          // team radio
new sStream = SvCreateStream(10.0);          // squad radio

SvSetKeyWithChannels(playerid, 0x42, CH_GLOBAL | CH_TEAM);  // B → global + team
SvSetKeyWithChannels(playerid, 0x5A, CH_SQUAD);             // Z → squad only

SvAttachSpeakerToStreamWithChannels(gStream, playerid, CH_GLOBAL);
SvAttachSpeakerToStreamWithChannels(tStream, playerid, CH_TEAM);
SvAttachSpeakerToStreamWithChannels(sStream, playerid, CH_SQUAD);

SvEnableSpeaker(playerid, CH_GLOBAL | CH_TEAM);  // deny squad for this player
// Now Z (CH_SQUAD) has no effect — squad channel is disabled by Pawn
```

> **Note:** `SvAttachSpeakerToStream` (without `WithChannels`) defaults to mask `0xFFFFFFFF` — no channel restriction. `SvAddKey` (without `WithChannels`) also defaults to `0xFFFFFFFF`. Existing scripts that don't use channels work unchanged.

## Configuration
---------------------------------
The plugin can be configured via open.mp's `config.json`:

```json
{
    "sampvoice": {
        "port": 2020,
        "threads": 4,
        "updaterate": 200
    }
}
```

| Key | Default | Description |
|-----|---------|-------------|
| `sampvoice.port` | Random | UDP port for voice traffic. Set this if you need to open a firewall rule |
| `sampvoice.threads` | 8 | Number of worker threads for voice processing. Typically set to CPU cores minus 1 |
| `sampvoice.updaterate` | 200 | Player position check interval in milliseconds. Lower = more responsive but higher CPU |

## Compiling
---------------------------------
Plugin compiles for *Win32/x64* and *Linux x86/x86_64* platforms.

The server plugin can be built as **32-bit** or **64-bit**. The client plugin (SA:MP `.asi`) is **32-bit only**.

> **Note:** PAWN cell size is always **32-bit** regardless of plugin architecture.

Below are further instructions:

Clone the repository to your computer and go to the plugin directory:
```sh
git clone https://github.com/AmyrAhmady/sampvoice.git
git submodule update --init --recursive
cd sampvoice
```

### Windows (Server)
---------------------------------
#### 32-bit server
```sh
mkdir build32
cd build32
cmake .. -A Win32
cmake --build .
```

#### 64-bit server
```sh
mkdir build64
cd build64
cmake .. -A x64
cmake --build .
```

### Windows (Client)
--------------------------------
The client plugin (`.asi` for SA:MP) can only be built as **32-bit**.

Four SA:MP versions (R1, R3-1, R5-1, DL-1) are built simultaneously:

```sh
mkdir build_client
cd build_client
cmake .. -DBUILD_CLIENT=ON -A Win32
cmake --build . --target sampvoice-client
```

Outputs: `sampvoice_r1.asi`, `sampvoice_r3.asi`, `sampvoice_r5.asi`, `sampvoice_dl.asi`

> **Note:** For packaging the runtime files (BASS DLLs, language files, resources) alongside the `.asi`, refer to the CI workflow in `.github/workflows/build.yml`.

> **Note:** The client loads all UI resources from the `resources/` folder placed next to the `.asi` (fonts, icons, blur shader) and the language packs from `languages/`. These folders are **required** at runtime.
>
> **Customizing the UI font:** drop any TrueType (`.ttf`) or OpenType (`.otf`) font into `resources/font.ttf` / `resources/font.otf` to replace the UI font (the loader prefers `font.ttf`, then `font.otf`). The glyph ranges baked into the atlas cover Latin, Cyrillic, Greek, Hebrew, Arabic (no RTL shaping), full CJK, Japanese kana, Korean and Thai; characters outside those ranges render as fallback boxes. Note that a larger font file only matters if the requested glyphs are within those ranges.

> **Note:** The client requires `d3dx9.h` headers. If you have the DirectX SDK installed, set `DXSDK_DIR`. Otherwise, the build will find them via NuGet (`Microsoft.DXSDK.D3DX`) or use the local headers in `client/include/dxsdk/`. No separate DXSDK installation is required.

### Linux (Server)
---------------------------------
#### 32-bit build
```sh
mkdir build32
cd build32
cmake .. -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

#### 64-bit build
```sh
mkdir build64
cd build64
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```
