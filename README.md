# mywaylyric

一个基于 Qt 6 的 Linux 桌面歌词程序，通过 MPRIS 读取播放器元数据，并从本地 `.lrc` 文件中匹配歌词后显示在桌面上。

当前工程里的可执行文件名还是 `MprisDemo`，但功能上已经是一个本地歌词桌面悬浮窗项目。

## 功能

- 监听支持 MPRIS 的播放器元数据
- 按歌曲名和歌手匹配本地歌词文件
- 匹配到歌词时显示，未匹配到时自动隐藏
- 支持托盘菜单
- 支持手动浏览歌词文件
- 支持图形界面编辑歌词目录
- 歌词目录与样式设置会保存到配置文件
- 支持桌面歌词样式设置
- 支持锁定/解锁歌词窗口

## 依赖

- CMake 3.16+
- 支持 C++17 的编译器
- Qt 6
  - `Core`
  - `Gui`
  - `Widgets`
  - `DBus`
  - `WaylandCompositor`
- `LayerShellQt`
- `X11`

如果你使用 Debian / Ubuntu 系发行版，可以先按自己的系统包名安装对应开发包。不同发行版的 `LayerShellQt` 包名可能不同。

## 构建

```bash
cmake -S . -B build
cmake --build build
```

构建完成后，可执行文件默认位于：

```bash
./build/MprisDemo
```

## 运行

```bash
./build/MprisDemo
```

程序启动后会驻留在系统托盘中。只有在成功匹配并加载歌词后，桌面歌词窗口才会显示。

## 使用方式

1. 启动播放器，并确保播放器支持 MPRIS。
2. 启动本程序。
3. 右键托盘图标，可以进行以下操作：
   - `歌词目录设置...`
   - `浏览歌词`
   - `重新载入歌词`
   - `歌词样式设置...`
   - `锁定歌词 / 解锁歌词`
4. 把本地 `.lrc` 文件放到你配置的歌词目录中。

## 歌词匹配规则

程序会优先使用“歌名 + 歌手”的组合去匹配歌词文件名，典型形式包括：

- `Song Title - Artist.lrc`
- `Artist - Song Title.lrc`
- `Song Title Artist.lrc`

如果存在多个候选文件，当前逻辑会弹出选择框让你手动确认。

## 配置文件

程序使用 `QSettings` 保存配置。

在 Linux 下，配置通常位于：

```text
~/.config/LyricPhase/DesktopLyric.conf
```

目前主要会保存：

- 歌词搜索目录
- 窗口位置和大小
- 字体
- 描边颜色与宽度
- 已播放/未播放歌词渐变颜色

## 项目结构

```text
main.cpp                    程序入口
MprisPlayer.*               MPRIS 播放器信息读取
LyricParser.*               LRC 歌词解析
LyricFinder.*               本地歌词搜索与匹配
DesktopLyricWindow.*        桌面歌词窗口
LyricTray.*                 系统托盘菜单
LyricsDirectoryDialog.*     歌词目录设置对话框
LyricsStyleDialog.*         歌词样式设置对话框
LyricSelectionDialog.*      多候选歌词选择对话框
```

## 当前限制

- 目前主要面向 Linux 桌面环境
- 歌词依赖本地 `.lrc` 文件，不会联网下载
- 工程名和生成的可执行文件名暂时仍是 `MprisDemo`
- 多候选歌词选择框目前还比较基础

## 后续可以继续做的方向

- 递归搜索子目录中的歌词文件
- 更好的多候选歌词表格式选择框
- 自动记住每首歌上次选择的歌词
- 根据播放/暂停状态自动隐藏或淡出歌词
- 重命名工程与可执行文件，统一为 `mywaylyric`
