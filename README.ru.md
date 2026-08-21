# **SAMPVOICE** [Оригинальный репозиторий](https://github.com/CyberMor/sampvoice)
[English](https://github.com/dockfries/sampvoice/blob/master/README.md) | Русский | [简体中文](https://github.com/dockfries/sampvoice/blob/master/README.zh-CN.md)

## Описание
---------------------------------
**SAMPVOICE** - это набор разработчика для реализации систем голосового общения на языке *Pawn* под сервера *open.mp*.

#### Поддержка версий
----------------------------------
* Клиент: SA:MP 0.3.7 (R1, R3-1, R5-1, DL-1)
* Сервер: Последняя версия open.mp

## Основные возможности
---------------------------------
* Контролируемая передача голосового потока
* Управление микрофоном игрока
* Привязка голосового потока к игровым объектам
* И ещё множество мелочей...

## Установка
---------------------------------
Для работы плагина, его необходимо установить игрокам и на сервер. Для этого есть клиентская и серверная части плагина.

#### Для игроков
---------------------------------
Игрокам доступны 2 вариант установки: автоматический (через установщик) и ручной (через архив).

##### Автоматический вариант
---------------------------------
1. Для того, чтобы скачать установщик перейдите по [ссылке](https://github.com/dockfries/sampvoice/releases) и выберите подходящую версию плагина.
2. После скачивания запустите установщик, выберите язык установки, затем установщик автоматически определит директорию игры.
3. Если директория верна, нажмите "ОК" и дождитесь конца установки. После окончания установки установщик завершит свою работу.

##### Ручной вариант
---------------------------------
1. Перейдите по [ссылке](https://github.com/dockfries/sampvoice/releases) и скачайте архив с необходимой версией клиента.
2. Распакуйте содержимое архива в корневую директорию игры с заменой файлов.

#### Для разработчиков
---------------------------------
1. Загрузите архив с необходимой версией плагина для вашей платформы по [ссылке](https://github.com/dockfries/sampvoice/releases).
2. Распакуйте архив в корневую директорию сервера.
3. Добавьте в файл конфигурации сервера *server.cfg* строку *"plugins sampvoice"* для *Win32* и *"plugins sampvoice.so"* для *Linux x86*. **(Если у вас стоит плагин Pawn.RakNet обязательно разместите SampVoice после него)**

## Использование
---------------------------------
Для того, чтобы начать использовать плагин, прочтите документацию, которая входит в комплект с серверной частью. Для этого откройте файл *sampvoice.chm* с помощью справочника Windows. **(Если документация не открывается, нажмите на файл документации правой кнопкой мыши, затем "Свойства" -> "Разблокировать" -> "ОК")**

Чтобы начать использовать функционал плагина, подключите заголовочный файл:
```php
#include <sampvoice>
```

#### Краткая справка
---------------------------------
Вам необходимо знать, что в плагине используется своя система типов и констант. Не смотря на то, что это всего лишь оболочка над базовыми типами Pawn, она помогает ориентироваться в типах самого плагина и не путать указатели.

Для того, чтобы перенаправить аудиотрафик от игрока A к игроку B вам необходимо создать звуковой поток (например глобальный, с помощью **SvCreateGStream**), затем подключить к потоку игрока A как спикера (с помощью **SvAttachSpeakerToStream**), после чего подключить к потоку игрока B как слушателя (с помощью **SvAttachListenerToStream**). Готово. Теперь при активации микрофона у игрока A (например, функцией **SvStartRecord**), его аудиотрафик будет передан, а затем услышан игроком B.

Звуковые потоки довольно удобная вещь. Наглядно их можно представить на примере Discord'а:
* Поток - это аналог комнаты (или канала).
* Спикеры - это участники комнаты с отключенным звуком, но включенным микрофоном.
* Слушатели - это участники комнаты с отключенным микрофоном, но включенным звуком.

Игроки могут быть и спикерами и слушателями одновременно. При этом аудиотрафик игрока не будет ему переотправлен.

#### Пример
---------------------------------
Давайте рассмотрим некоторые возможности плагина на практическом примере. Ниже мы создадим сервер, который будет привязывать всех подключившихся игроков к глобальному потоку, а также создавать под каждого игрока локальный поток. Таким образом, игроки смогут общаться через глобальный (слышен одинаково в любой точке карты) и локальный (слышен только рядом с игроком) чаты.
```php
#include <sampvoice>

new SV_GSTREAM:gstream = SV_NULL;
new SV_LSTREAM:lstream[MAX_PLAYERS] = { SV_NULL, ... };

/*
    Паблики OnPlayerActivationKeyPress и OnPlayerActivationKeyRelease
    нужны для того, чтобы при нажатии соответствующих клавиш перенаправлять
    аудиотрафик игрока в соответствующие потоки.
*/

public SV_VOID:OnPlayerActivationKeyPress(SV_UINT:playerid, SV_UINT:keyid) 
{
    // Подключаем игрока к локальному потоку как спикера, если нажата клавиша 'B'
    if (keyid == 0x42 && lstream[playerid]) SvAttachSpeakerToStream(lstream[playerid], playerid);
    // Подключаем игрока к глобальному потоку как спикера, если нажата клавиша 'Z'
    if (keyid == 0x5A && gstream) SvAttachSpeakerToStream(gstream, playerid);
}

public SV_VOID:OnPlayerActivationKeyRelease(SV_UINT:playerid, SV_UINT:keyid)
{
    // Отключаем игрока от локального потока, если отпущена клавиша 'B'
    if (keyid == 0x42 && lstream[playerid]) SvDetachSpeakerFromStream(lstream[playerid], playerid);
    // Отключаем игрока от глобального потока, если отпущена клавиша 'Z'
    if (keyid == 0x5A && gstream) SvDetachSpeakerFromStream(gstream, playerid);
}

public OnPlayerConnect(playerid)
{
    // Проверяем наличие плагина
    if (SvGetVersion(playerid) == SV_NULL)
    {
        SendClientMessage(playerid, -1, "Не удалось обнаружить плагин sampvoice.");
    }
    // Проверяем наличие микрофона
    else if (SvHasMicro(playerid) == SV_FALSE)
    {
        SendClientMessage(playerid, -1, "Не удалось обнаружить микрофон.");
    }
    // Создаём локальный поток с дистанцией слышимости 40.0, неограниченным количеством слушателей
    // и именем 'Local' (имя 'Local' будет отображено красным цветом в speakerlist'е у игроков)
    else if ((lstream[playerid] = SvCreateDLStreamAtPlayer(40.0, SV_INFINITY, playerid, 0xff0000ff, "Local")))
    {
        SendClientMessage(playerid, -1, "Нажмите Z, чтобы говорить в глобальный чат и B, чтобы говорить в локальный чат.");

        // Подключаем игрока к глобальному потоку как слушателя
        if (gstream) SvAttachListenerToStream(gstream, playerid);

        // Назначаем игроку клавиши активации микрофона
        SvAddKey(playerid, 0x42);
        SvAddKey(playerid, 0x5A);
    }
}

public OnPlayerDisconnect(playerid, reason)
{
    // Удаляем локальный поток игрока после отключения
    if (lstream[playerid])
    {
        SvDeleteStream(lstream[playerid]);
        lstream[playerid] = SV_NULL;
    }
}

public OnGameModeInit()
{
    // Раскомментируйте строку, чтобы включить режим отладки
    // SvDebug(SV_TRUE);

    gstream = SvCreateGStream(0xffff0000, "Global");
}

public OnGameModeExit()
{
    if (gstream) SvDeleteStream(gstream);
}
```

## Конфигурация
---------------------------------
#### Канальная маршрутизация (Channel Bitmask)
---------------------------------
Плагин поддерживает **32 независимых аудиоканала** (биты 0–31) для гибкой маршрутизации голоса. Канал добавляет дополнительное измерение: нажатие кнопки, привязка спикера и стрим — каждый несут свою битовую маску каналов. Голос передаётся только если все три маски пересекаются.

| Термин | Значение |
|--------|----------|
| `channelmask` | `uint32_t` битовая маска. Бит 0 = канал 0 (`0x01`), бит 1 = канал 1 (`0x02`), … бит 31 = канал 31 (`0x80000000`) |
| `SvSetKeyWithChannels(player, key, mask)` | При нажатии `key` каналы из `mask` становятся **активными** |
| `SvAttachSpeakerToStreamWithChannels(stream, player, mask)` | Голос только на каналах из `mask` попадёт в `stream` |
| `SvEnableSpeaker(player, mask)` / `SvDisableSpeaker(player, mask)` | Разрешает/запрещает игроку указанные каналы (по умолчанию все). `SvCheckSpeaker(player, mask)` проверяет |
| **Правило** | `(activeCh & enabledCh & chMask) != 0` — все три маски должны пересекаться |

```pawn
#define CH_GLOBAL   0b0001  // канал 0
#define CH_TEAM     0b0010  // канал 1
#define CH_SQUAD    0b0100  // канал 2

new gStream = SvCreateStream(0.0);
new tStream = SvCreateStream(30.0);
new sStream = SvCreateStream(10.0);

SvSetKeyWithChannels(playerid, 0x42, CH_GLOBAL | CH_TEAM);
SvSetKeyWithChannels(playerid, 0x5A, CH_SQUAD);

SvAttachSpeakerToStreamWithChannels(gStream, playerid, CH_GLOBAL);
SvAttachSpeakerToStreamWithChannels(tStream, playerid, CH_TEAM);
SvAttachSpeakerToStreamWithChannels(sStream, playerid, CH_SQUAD);

SvEnableSpeaker(playerid, CH_GLOBAL | CH_TEAM);  // squad запрещён
```

> **Примечание:** `SvAttachSpeakerToStream` (без `WithChannels`) использует маску `0xFFFFFFFF`. `SvAddKey` (без `WithChannels`) тоже. Скрипты без каналов работают без изменений.

## Настройка
---------------------------------
Плагин настраивается через `config.json` open.mp:

```json
{
    "sampvoice": {
        "port": 2020,
        "threads": 4,
        "updaterate": 200
    }
}
```

| Ключ | По умолчанию | Описание |
|------|-------------|----------|
| `sampvoice.port` | Случайный | UDP порт для голосового трафика. Укажите, если нужен доступ через firewall |
| `sampvoice.threads` | 8 | Количество рабочих потоков для обработки голоса. Обычно количество ядер CPU минус 1 |
| `sampvoice.updaterate` | 200 | Интервал проверки позиции игроков в миллисекундах. Меньше = быстрее реакция, но выше нагрузка на CPU |

## Компиляция
---------------------------------
Плагин компилируется под платформы *Win32/x64* и *Linux x86/x86_64*.

Серверный плагин можно собрать как **32-битным**, так и **64-битным**. Клиентский плагин (`.asi` для SA:MP) — только **32-битный**.

> **Примечание:** Размер PAWN-ячейки всегда **32-битный** независимо от архитектуры плагина.

Ниже прилагаются подробные инструкции:

Склонируйте репозиторий себе на компьютер и перейдите в директорию плагина:
```sh
git clone https://github.com/dockfries/sampvoice.git
git submodule update --init --recursive
cd sampvoice
```

### Windows (Сервер)
---------------------------------
#### 32-битный сервер
```sh
mkdir build32
cd build32
cmake .. -A Win32
cmake --build .
```

#### 64-битный сервер
```sh
mkdir build64
cd build64
cmake .. -A x64
cmake --build .
```

### Windows (Клиент)
--------------------------------
Клиентский плагин (`.asi` для SA:MP) собирается только как **32-битный**.

Все четыре версии SA:MP (R1, R3-1, R5-1, DL-1) собираются одновременно:

```sh
mkdir build_client
cd build_client
cmake .. -DBUILD_CLIENT=ON -A Win32
cmake --build . --target sampvoice-client
```

Результаты: `sampvoice_r1.asi`, `sampvoice_r3.asi`, `sampvoice_r5.asi`, `sampvoice_dl.asi`

> **Примечание:** Для упаковки файлов времени выполнения (BASS DLL, файлы языков, ресурсы) вместе с `.asi` обратитесь к CI workflow в `.github/workflows/build.yml`.

> **Примечание:** Клиенту требуются заголовки `d3dx9.h`. Если установлен DirectX SDK, укажите `DXSDK_DIR`. В противном случае сборка найдёт их через NuGet (`Microsoft.DXSDK.D3DX`) или использует локальные заголовки в `client/include/dxsdk/`. Отдельная установка DXSDK не требуется.

### Linux (Server)
---------------------------------
#### 32-битная сборка
```sh
mkdir build32
cd build32
cmake .. -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

#### 64-битная сборка
```sh
mkdir build64
cd build64
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```
