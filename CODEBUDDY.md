# CODEBUDDY.md

This file provides guidance to CodeBuddy Code when working with code in this repository.

## 项目概览

EasyAV 是面向"小白"用户的极简音视频处理工具（C++17 + Dear ImGui + GLFW + OpenGL3）。
当前基线是**仅有 UI 与交互**：不包含 FFmpeg、编码、录屏等真实业务逻辑，点击"开始处理"只是
`TaskController` 在 UI 线程按时间推进进度。完整需求见 `docs/EasyAV_SRS.md`，进度、待办、
验收标准与开发约定见 `docs/EasyAV_Progress_TODO.md`（完成待办后需同步更新该文档与 README.txt）。

## 构建与运行

ImGui/GLFW 源码通过相对路径从同级目录复用，构建前必须保证存在：

- `../imgui-docking`（imgui 源码 + backends）
- `../imgui-docking/examples/libs/glfw/include` 与 `lib-vc2010-32/64`
- 预编译 `glfw3.lib` 由 VS2010 编译，因此 `main.cpp` 中需要 `legacy_stdio_definitions.lib`

MSVC（推荐，4 个配置齐全：Debug/Release × Win32/x64）：

    msbuild EasyAV.slnx /p:Configuration=Debug /p:Platform=x64
    # 产物：Debug/EasyAV.exe 或 Release/EasyAV.exe

或使用 `build_win32.bat`（需先运行 `vcvars32.bat` / `vcvarsall.bat`，固定 32 位输出到 `Debug/`）。

MSYS2 / MinGW / Linux / macOS：

    make          # ImGui 源码从 ../imgui-docking 编译进来
    make clean

无自动化测试；改动后常用的最低验证手段是：

    g++ -std=c++17 -Wall -Wextra -fsyntax-only -I./src -I../imgui-docking -I../imgui-docking/backends \
        -I../imgui-docking/examples/libs/glfw/include main.cpp $(find src -name '*.cpp')

工程/编译关键设置：C++17、`/utf-8`、字符集 MultiByte、`SubSystem=Windows` + `mainCRTStartup`
（不弹控制台）、链接 `opengl32 glfw3 comdlg32 shell32 ole32`。

## 架构

严格分层，依赖方向单向（core ← model ← ui/panels，platform 只被 App 调用）：

| 位置 | 职责 | 依赖 |
| --- | --- | --- |
| `main.cpp` | GLFW 窗口、ImGui 后端、拖拽回调转发（唯一含 GLFW 的文件） | App、ui/Theme |
| `src/App.h/.cpp` | 应用外壳：装配模型与面板、执行平台请求、固定布局（顶栏/队列/场景区/操作栏/状态栏） | model、ui、platform |
| `src/core` | 纯数据与工具：`Types.h`（枚举/MediaItem/Toast）、`Format`（字节/时间/百分比格式化）、`Text`（UTF-8↔wchar_t 路径转换） | 仅标准库，**禁止 include ImGui** |
| `src/model` | `AppModel`（聚合根）、`MediaQueue`（文件队列与扩展名过滤）、`TaskSettings`（各场景参数纯数据）、`TaskController`（任务状态机） | core |
| `src/platform` | 原生文件/文件夹对话框、打开资源管理器（Win32） | core/Text |
| `src/ui` | `Theme`（颜色/DPI/字体）、`Widgets`（通用控件）、`Labels`（中文文案映射） | core、ImGui |
| `src/ui/panels` | 12 个面板/弹窗，每个是 `void DrawXxx(AppModel&)` 无状态函数 | model、ui |

### 关键约定（违反会破坏现有解耦）

- **面板互不引用**：每个面板只读写 `AppModel`，状态全部放在 `AppModel`（`Queue/Compress/Clip/
  Filter/Record/Output/Ui/Task`）。
- **UI 不直接调用系统 API**：面板只置位 `UiState` 中的 `request*` 标志（如 `requestAddFiles`、
  `requestPickOutputDir`），由 `App::ProcessRequests()` 统一调用 `platform::` 并回写结果。
- **core/model/platform 不引用 ImGui**；颜色只取 `ui::Colors()`，尺寸常量一律用 `ui::S()` 缩放
  （与 `style.FontScaleDpi` 同步，保证 125%/150% DPI 一致）。
- **任务执行唯一接入点**是 `TaskController`（Start/Cancel/Tick/Progress/StatusText/
  ConsumeCompleted）。接 FFmpeg 时替换其内部实现为消费后台事件，面板无需改动。
- **UI 文案**：只出现口语化中文，不出现命令行参数、编码器名、CRF 等术语；文案映射集中放在
  `src/ui/Labels.h`。
- **任何阻塞操作不得放在 UI 线程**（SRS 4.1）；`App::Frame()` 每帧只做非阻塞的 `Tick` + 绘制。
- **新增源文件**需同步更新 `EasyAV.vcxproj`、`EasyAV.vcxproj.filters`、`Makefile`、
  `build_win32.bat`（后两者使用通配符的部分除外）。

### 数据流

拖拽/文件对话框 → `AppModel::AddPaths` → `MediaQueue`（校验扩展名、去重、上限 500、递归扫描目录、
UTF-8 绝对路径）→ 面板渲染 → `ActionBar` 点开始 → `TaskController::Start(文件名列表)` →
每帧 `AppModel::Tick` 推进 → `ConsumeCompleted()` 一次性事件触发 Toast / 打开输出目录。

`MediaItem::durationSec == 0` 表示"待分析"（尚无媒体探测），剪辑时间轴因此按百分比显示；
`AppModel::EstimateOutputBytes()` 是经验公式估算，不是真实编码结果。

### 后续接入真实处理链路

计划新增 `src/services/`（`IMediaProbe`/`ITaskService`/`FfmpegCommandBuilder`/`FfmpegProcess`/
`ProgressParser` 等），命令必须用**参数数组**构造，禁止拼接裸 shell 字符串，路径需转义；
子进程用 `CreateProcess` + `CREATE_NO_WINDOW` + 匿名管道 + Job Object 回收。细节与任务拆解见
`docs/EasyAV_Progress_TODO.md` 第 4 节。

## 其他注意点

- 字体优先加载 `assets/fonts/SourceHanSansCN-Regular.ttf`，不存在则回退 `msyh.ttc`/`simhei.ttf`/
  `Deng.ttf`，并使用 `GetGlyphRangesChineseFull()`（该字体目录当前未纳入仓库）。
- 界面为**固定式单窗口布局**（虽使用 imgui docking 分支，但未启用多视口/停靠），窗口最小 1024×680
  （按 DPI 缩放），常量在 `main.cpp` 与 `App::Frame()` 中。
- `imgui.ini` 是运行时产物，已在 `.gitignore` 中；`Debug/`、`Release/` 同为构建产物。
