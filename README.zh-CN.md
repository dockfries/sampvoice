# **SAMPVOICE** [原始仓库](https://github.com/CyberMor/sampvoice)
简体中文 | [English](https://github.com/AmyrAhmady/sampvoice/blob/master/README.md) | [Русский](https://github.com/AmyrAhmady/sampvoice/blob/master/README.ru.md)

## 本分支的改动
---------------------------------
本分支在 open.mp 移植版基础上，增加了一系列客户端改进：

* **外部 UI 资源** —— 客户端从 `.asi` 同级的 `resources/` 和 `languages/` 文件夹加载字体、图标、模糊着色器和语言包，不再编译进二进制。没有 UI 资源被打进 `.asi`。
  * 用户可替换 `resources/font.ttf`（或 `font.otf`）来更换 UI 字体。烘焙进图集（atlas）的字形覆盖：拉丁、西里尔、希腊、希伯来、阿拉伯（仅有字形，无 RTL 排版）、全量 CJK、日文假名、韩文谚文和泰文。
* **运行时语言包** —— 菜单文案从 `languages/<名称>.json`（UTF-8，键值对）加载，并带英文兜底。在"常规"页有语言选择下拉框，可运行时切换并持久化。内置语言：English、Русский、Srpski、Bahasa Indonesia、Português (Brasil)、简体中文。
* **多字节输入与显示** —— `WM_CHAR`/`WM_IME_CHAR` 直接送入 ImGui（支持输入法合成的中文输入）；玩家昵称从系统 ANSI 代码页转换为 UTF-8 显示；黑名单筛选会把输入归一化到系统代码页后再匹配。
* **麦克风可用性修复** —— 录音设备在进游戏时本地枚举，而不是只在服务端握手时枚举，因此服务端没有插件时菜单不再误报"未检测到麦克风"。
* **按键说话（PTT）延迟优化** —— 移除每次按键的调试日志；麦克风启用时保持 BASS 录音通道常开，改为排空积压数据，而不是每次按键都暂停/播放。
* **omp-cef 兼容** —— 重构了渲染生命周期：窗口消息改用 `SetWindowSubclass` 而非 `SetWindowLong`；`Direct3DCreate9` 调用点以字节校验方式挂钩；D3D9 包装类改用内部 COM 引用计数。
* **Dear ImGui 升级到 v1.92.9b**（从 1.68 WIP），并适配新 API（`BeginChild(ImGuiChildFlags_FrameStyle)`、`ImTextureRef`、图集自管理、字体数据在图集生命周期内常驻）。

## 简介
---------------------------------
**SAMPVOICE** —— 一个用于在 Pawn 语言中为 open.mp 服务器实现语音通信系统的软件开发工具包（SDK）。

#### 版本支持
----------------------------------
* 客户端：SA:MP 0.3.7（R1、R3-1、R5-1、DL-1）
* 服务端：最新 open.mp 版本

## 功能
---------------------------------
* 受控的语音传输
* 玩家麦克风控制
* 将语音流绑定到游戏对象
* 以及更多功能……

## 安装
---------------------------------
要让插件正常工作，玩家端和服务端都必须安装。插件分为客户端和服务端两部分。

#### 玩家安装
---------------------------------
玩家有 2 种安装方式：自动（通过安装程序）和手动（通过压缩包）。

##### 自动安装
---------------------------------
1. 前往[发布页面](https://github.com/AmyrAhmady/sampvoice/releases)，选择所需版本的插件下载安装程序。
2. 下载后运行安装程序，选择安装语言，安装程序会自动找到你的 GTA San Andreas 目录。
3. 如果目录正确，点击"确定"并等待安装完成。安装完成后，安装程序会退出。

##### 手动安装
---------------------------------
1. 前往[发布页面](https://github.com/AmyrAhmady/sampvoice/releases)，下载所需客户端版本的压缩包。
2. 将压缩包解压到你的 GTA San Andreas 目录。

#### 开发者安装
---------------------------------
1. 从[发布页面](https://github.com/AmyrAhmady/sampvoice/releases)下载适用于你平台的插件版本。
2. 将压缩包解压到服务器的根目录。
3. 在 *server.cfg* 服务器配置文件中添加一行 *"plugins sampvoice"*（*Win32*）或 *"plugins sampvoice.so"*（*Linux x86*）。**（如果你有 Pawn.RakNet 插件，请确保 SampVoice 排在它之后）**

## 使用
---------------------------------
要开始使用插件，请阅读随服务端附带的文档。用 Windows 帮助查看器打开 *sampvoice.chm* 文件。**（如果文档打不开，右键点击文档文件，然后 属性 -> 解除锁定 -> 确定）**

要使用插件功能，需包含头文件：
```php
#include <sampvoice>
```

#### 快速参考
---------------------------------
需要知道，插件使用自己的类型和常量系统。尽管这只是一个对 Pawn 基础类型的包装，但它有助于理解插件自身的类型，避免混淆指针。

要把玩家 A 的语音流量重定向到玩家 B，你需要创建一个音频流（例如用 **SvCreateGStream** 创建全局流），然后用 **SvAttachSpeakerToStream** 把它作为说话者附加到玩家 A 的流，再用 **SvAttachListenerToStream** 把玩家 B 作为收听者附加到该流。完成。现在，当玩家 A 的麦克风激活时（例如用 **SvStartRecord** 函数），他的语音流量就会被传输，玩家 B 就能听到。

语音流非常实用，可以用 Discord 来类比理解：
* 流相当于一个房间（或频道）。
* 说话者相当于房间里麦克风已开但处于静音（自己听不到别人？）的参与者。
* 收听者相当于房间里麦克风静音（只接收）的参与者。

玩家可以同时是说话者和收听者。在这种情况下，该玩家的语音流量不会转发给他自己。

#### 示例
---------------------------------
下面用一个实际例子来看看插件的一些功能。我们将创建一个服务器，把所有连接玩家绑定到全局流，同时为每个玩家创建一个本地流。这样，玩家就可以通过全局（地图上任何位置听声音量相同）和本地（只有玩家附近能听到）两种聊天进行交流。
```cpp
#include <sampvoice>

new SV_GSTREAM:gstream = SV_NULL;
new SV_LSTREAM:lstream[MAX_PLAYERS] = { SV_NULL, ... };

/*
    OnPlayerActivationKeyPress 和 OnPlayerActivationKeyRelease 这两个公共函数
    的作用是：在按下对应按键时，把玩家的语音流量重定向到对应的流。
*/

public SV_VOID:OnPlayerActivationKeyPress(SV_UINT:playerid, SV_UINT:keyid) 
{
    // 按下 'B' 键时，把玩家作为说话者附加到本地流
    if (keyid == 0x42 && lstream[playerid]) SvAttachSpeakerToStream(lstream[playerid], playerid);
    // 按下 'Z' 键时，把玩家作为说话者附加到全局流
    if (keyid == 0x5A && gstream) SvAttachSpeakerToStream(gstream, playerid);
}

public SV_VOID:OnPlayerActivationKeyRelease(SV_UINT:playerid, SV_UINT:keyid)
{
    // 松开 'B' 键时，把玩家从本地流分离
    if (keyid == 0x42 && lstream[playerid]) SvDetachSpeakerFromStream(lstream[playerid], playerid);
    // 松开 'Z' 键时，把玩家从全局流分离
    if (keyid == 0x5A && gstream) SvDetachSpeakerFromStream(gstream, playerid);
}

public OnPlayerConnect(playerid)
{
    // 检查插件是否可用
    if (SvGetVersion(playerid) == SV_NULL)
    {
        SendClientMessage(playerid, -1, "找不到 sampvoice 插件。");
    }
    // 检查是否有麦克风
    else if (SvHasMicro(playerid) == SV_FALSE)
    {
        SendClientMessage(playerid, -1, "找不到麦克风。");
    }
    // 创建一个可听距离 40.0、收听者数量不限、名称为 'Local' 的本地流
    // （'Local' 名称会以红色显示在玩家的说话者列表中）
    else if ((lstream[playerid] = SvCreateDLStreamAtPlayer(40.0, SV_INFINITY, playerid, 0xff0000ff, "Local")))
    {
        SendClientMessage(playerid, -1, "按 Z 键对全局频道说话，按 B 键对本地频道说话。");

        // 把玩家作为收听者附加到全局流
        if (gstream) SvAttachListenerToStream(gstream, playerid);

        // 给玩家分配麦克风激活按键
        SvAddKey(playerid, 0x42);
        SvAddKey(playerid, 0x5A);
    }
}

public OnPlayerDisconnect(playerid, reason)
{
    // 玩家断开后移除他的本地流
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
    // 取消注释下面一行可开启调试模式
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

#### 频道位掩码路由
---------------------------------
插件支持 **32 个独立的音频频道**（位 0–31），用于精细的语音路由控制。这比"流"多了一个维度：一次按键、一个说话者挂载和一个流各自携带一个频道掩码，只有三者都重叠时语音才会被转发。

**核心概念：**

| 术语 | 含义 |
|------|------|
| `channelmask` | 一个 `uint32_t` 位掩码。第 0 位 = 频道 0（`0x01`），第 1 位 = 频道 1（`0x02`），…… 第 31 位 = 频道 31（`0x80000000`） |
| `SvSetKeyWithChannels(player, key, mask)` | 玩家按下 `key` 时，`mask` 中的频道变为**激活** |
| `SvAttachSpeakerToStreamWithChannels(stream, player, mask)` | 只有 `mask` 中频道的语音才会进入 `stream` |
| `SvEnableSpeaker(player, mask)` / `SvDisableSpeaker(player, mask)` | Pawn 侧的门控——限制玩家被允许使用的频道（默认全部）。`SvCheckSpeaker(player, mask)` 查询它 |
| **转发规则** | `(activeCh & enabledCh & chMask) != 0` —— 三个掩码必须相交 |

**示例 —— 3 频道配置：**

```pawn
#define CH_GLOBAL   0b0001  // 频道 0
#define CH_TEAM     0b0010  // 频道 1
#define CH_SQUAD    0b0100  // 频道 2

new gStream = SvCreateStream(0.0);           // 全局音乐
new tStream = SvCreateStream(30.0);          // 队伍电台
new sStream = SvCreateStream(10.0);          // 小队电台

SvSetKeyWithChannels(playerid, 0x42, CH_GLOBAL | CH_TEAM);  // B → 全局 + 队伍
SvSetKeyWithChannels(playerid, 0x5A, CH_SQUAD);             // Z → 仅小队

SvAttachSpeakerToStreamWithChannels(gStream, playerid, CH_GLOBAL);
SvAttachSpeakerToStreamWithChannels(tStream, playerid, CH_TEAM);
SvAttachSpeakerToStreamWithChannels(sStream, playerid, CH_SQUAD);

SvEnableSpeaker(playerid, CH_GLOBAL | CH_TEAM);  // 禁止该玩家使用小队频道
// 现在 Z（CH_SQUAD）不生效 —— 小队频道已被 Pawn 禁用
```

> **注意：** `SvAttachSpeakerToStream`（不带 `WithChannels`）默认掩码为 `0xFFFFFFFF` —— 不限频道。`SvAddKey`（不带 `WithChannels`）同样默认 `0xFFFFFFFF`。不使用频道的现有脚本无需改动即可正常工作。

## 配置
---------------------------------
插件可以通过 open.mp 的 `config.json` 配置：

```json
{
    "sampvoice": {
        "port": 2020,
        "threads": 4,
        "updaterate": 200
    }
}
```

| 键 | 默认值 | 说明 |
|-----|--------|------|
| `sampvoice.port` | 随机 | 语音流量的 UDP 端口。需要开放防火墙规则时设置它 |
| `sampvoice.threads` | 8 | 语音处理的工作线程数。一般设为 CPU 核心数减 1 |
| `sampvoice.updaterate` | 200 | 玩家位置检查间隔（毫秒）。越小响应越快，但 CPU 占用越高 |

## 编译
---------------------------------
插件支持 *Win32/x64* 和 *Linux x86/x86_64* 平台。

服务端插件可编译为 **32 位**或 **64 位**。客户端插件（SA:MP `.asi`）**仅限 32 位**。

> **注意：** 无论插件架构如何，PAWN 单元大小始终是 **32 位**。

下面是进一步说明：

把仓库克隆到你的电脑并进入插件目录：
```sh
git clone https://github.com/AmyrAhmady/sampvoice.git
git submodule update --init --recursive
cd sampvoice
```

### Windows（服务端）
---------------------------------
#### 32 位服务端
```sh
mkdir build32
cd build32
cmake .. -A Win32
cmake --build .
```

#### 64 位服务端
```sh
mkdir build64
cd build64
cmake .. -A x64
cmake --build .
```

### Windows（客户端）
--------------------------------
客户端插件（SA:MP `.asi`）**仅限 32 位**。

四个 SA:MP 版本（R1、R3-1、R5-1、DL-1）会同时编译：

```sh
mkdir build_client
cd build_client
cmake .. -DBUILD_CLIENT=ON -A Win32
cmake --build . --target sampvoice-client
```

输出：`sampvoice_r1.asi`、`sampvoice_r3.asi`、`sampvoice_r5.asi`、`sampvoice_dl.asi`

> **注意：** 打包运行时文件（BASS DLL、语言文件、资源）到 `.asi` 旁，请参考 CI 工作流 `.github/workflows/build.yml`。

> **注意：** 客户端从 `.asi` 同级目录的 `resources/` 文件夹加载全部 UI 资源（字体、图标、模糊着色器），从 `languages/` 加载语言包。这两个文件夹**运行时必需**。
>
> **自定义 UI 字体：** 把任意 TrueType（`.ttf`）或 OpenType（`.otf`）字体放到 `resources/font.ttf` / `resources/font.otf` 即可替换 UI 字体（加载器优先 `font.ttf`，其次 `font.otf`）。烘焙进图集的字形范围覆盖：拉丁、西里尔、希腊、希伯来、阿拉伯（无 RTL 排版）、全量 CJK、日文假名、韩文谚文和泰文；超出这些范围的字符会显示为占位方框。注意，更大的字体文件只有在所需字形位于这些范围内时才有意义。

> **注意：** 客户端需要 `d3dx9.h` 头文件。如果安装了 DirectX SDK，请设置 `DXSDK_DIR`。否则构建会通过 NuGet（`Microsoft.DXSDK.D3DX`）找到它们，或使用 `client/include/dxsdk/` 中的本地头文件。无需单独安装 DXSDK。

### Linux（服务端）
---------------------------------
#### 32 位构建
```sh
mkdir build32
cd build32
cmake .. -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

#### 64 位构建
```sh
mkdir build64
cd build64
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```
