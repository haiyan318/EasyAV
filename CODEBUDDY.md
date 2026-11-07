# CODEBUDDY.md

This file provides guidance to CodeBuddy Code when working with code in this repository.

## 项目概览

EasyAV 是面向“小白”用户的极简音视频处理工具（C++17 + Dear ImGui + GLFW + OpenGL3）。
当前基线是 **V0.2 编码转换**：通过外部 `ffmpeg.exe` / `ffprobe.exe` 子进程，把
HEVC / AV1 / FLV 等常见编码转成 H.264（最兼容）或 H.265（更小），输出 MP4 / MKV，
也可提取 MP3；“视频瘦身”是转换面板里的可选开关。另有队列时长/编码探测、真实进度/取消、
防覆盖落盘与中文错误提示。剪辑拼接、滤镜水印、录屏截图仍是界面演示；GIF、设置持久化、
真实预览未实现。

完整需求见 `docs/EasyAV_SRS.md`；进度、剩余待办、验收标准与决策记录见
`docs/EasyAV_Progress_TODO.md`（完成待办后需同步更新该文档与 `README.md`）。

## 构建与运行

ImGui/GLFW 源码通过相对路径从同级目录复用，构建前必须保证存在：

- `../imgui-docking`（imgui 源码 + backends）
- `../imgui-docking/examples/libs/glfw/include` 与 `lib-vc2010-32/64`
- 预编译 `glfw3.lib` 由 VS2010 编译，因此 `main.cpp` 中需要 `legacy_stdio_definitions.lib`

构建不依赖 FFmpeg；FFmpeg 只在运行时查找。

MSVC（推荐，4 个配置：Debug/Release × Win32/x64）：

    msbuild EasyAV.slnx /p:Configuration=Debug /p:Platform=x64
    # 产物：Debug/EasyAV.exe 或 Release/EasyAV.exe

或使用 `build_win32.bat`（需先运行 `vcvars32.bat` / `vcvarsall.bat`，固定 32 位输出到 `Debug/`）。

MSYS2 / MinGW / Linux / macOS：

    make
    make clean

> 注意：services 目前只实现了 Windows 版本（CreateProcessW / Job Object / Win32 路径），
> 非 Windows 目标目前无法完整编译；Makefile 保留但需要补平台实现后才能使用。

单元测试（需 vcvars，使用 cl + shell32，不链接 ImGui）：

    tests\run_tests.bat
    # 当前 68 个用例：CommandLine / ProbeOutputParser / OutputNamer /
    # FfmpegCommandBuilder / ErrorFormatter / ProgressParser / TaskController /
    # ConvertMapping

最低限度的语法检查（无 MSVC 时常用）：

    g++ -std=c++17 -Wall -Wextra -fsyntax-only -I./src -I./tests \
        -I../imgui-docking -I../imgui-docking/backends \
        -I../imgui-docking/examples/libs/glfw/include \
        main.cpp $(find src tests -name '*.cpp')

工程/编译关键设置：C++17、`/utf-8`、字符集 MultiByte、`SubSystem=Windows` + `mainCRTStartup`、
链接 `opengl32 glfw3 comdlg32 shell32 ole32`。Win32 与 x64 共用 `Debug/`、`Release/` 输出目录。
已用 VS2026 18.9（v145）验证 Debug|x64 与 Release|x64 构建。

## 架构

严格分层，依赖方向单向（core ← model ← ui/panels；services 只依赖 core，App 负责装配）：

| 位置 | 职责 | 依赖 |
| --- | --- | --- |
| `main.cpp` | GLFW 窗口、ImGui 后端、拖拽回调转发（唯一含 GLFW 的文件） | App、ui/Theme |
| `src/App.h/.cpp` | 应用外壳：装配模型/服务/面板、执行平台请求、固定布局、退出清理 | model、services、ui、platform |
| `src/core` | 纯数据与工具：`Types`、`Format`、`Text`、`TaskContracts`、`ITaskRunner` | 仅标准库，**禁止 ImGui / windows.h** |
| `src/model` | `AppModel`（聚合根）、`MediaQueue`、`TaskSettings`、`ConvertMapping`（意图→参数）、`TaskController`（事件状态机） | core |
| `src/platform` | `AppPaths`（exe/工作目录）、原生文件/文件夹对话框、打开资源管理器 | core/Text |
| `src/services` | 真实处理能力：工具定位、参数构造、子进程、探测、进度、命名、错误、日志、任务服务 | core、platform 路径工具 |
| `src/ui` | `Theme`（颜色/DPI/字体）、`Widgets`（通用控件）、`Labels`（中文文案映射） | core、ImGui |
| `src/ui/panels` | 12 个面板/弹窗，每个是 `void DrawXxx(AppModel&)` 无状态函数 | model、ui |
| `tests` | 自带断言的轻量测试，只链接纯逻辑源码 | services 纯逻辑、model |

### 关键约定（违反会破坏现有解耦）

- **面板互不引用**：每个面板只读写 `AppModel`，状态全部放在 `AppModel`（`Queue/Convert/Clip/
  Filter/Record/Output/Ui/Task`）。
- **UI 不直接调用系统 API**：面板只置位 `UiState` 中的 `request*` 标志，由 `App::ProcessRequests()`
  统一调用 `platform::` 并回写结果。
- **TaskController 只依赖 `core::ITaskRunner`**：`services::TaskService` 实现该端口，`App::Init`
  调用 `AttachRunner(&m_services)`。面板看不到 services。
- **进程/管道/Job Object 只允许在 `services` 与 `platform`**；core/model 不包含 `windows.h`。
- **命令只通过参数数组构造**（`services::BuildFfmpegArgs`），引号/反斜杠仅由
  `services::CommandLine` 处理，禁止拼接裸 shell 字符串。
- **目标编码 → 编码器名只在 builder 出现**（H.264→libx264/h264_*，H.265→libx265/hevc_*）；
  面板只使用“H.264 / H.265 / 只要声音”这类用户语言，通过 `model::BuildTaskOptions` 映射。
- **任何阻塞操作不得在 UI 线程**：探测、CreateProcess、写文件、等待都在 `TaskService` 的 worker；
  UI 每帧只 `TaskController::Tick`（drain 事件）并绘制。
- **输出安全**：先写同目录 `.easyav-tmp-<pid>-<serial>.<ext>`，成功后 `MoveFileExW` 不覆盖重命名；
  取消/失败必须删除临时文件。
- **技术细节与界面分离**：stderr/退出码只进 `logs/easyav.log`；界面用
  `services::ErrorFormatter::UserMessage` 生成的中文文案。
- **颜色只取 `ui::Colors()`，尺寸一律用 `ui::S()`**（与 `style.FontScaleDpi` 同步）。
- **新增源文件**需同步 `EasyAV.vcxproj`、`.filters`、`build_win32.bat`；Makefile 用通配符
  收集 `src/*/*.cpp`，但 object 名按 basename 展平，新增文件名必须全局唯一。
- **新增纯逻辑模块**同步补 `tests/Test*.cpp` 并登记到 `tests/run_tests.bat`。

### 数据流

```
拖拽/文件对话框 -> AppModel::AddPaths -> MediaQueue（扩展名/去重/上限 500/UTF-8 绝对路径）
  -> TaskService::EnqueueProbe -> worker 调 ffprobe -> ProbeResult -> App::Frame 回写 MediaItem

ActionBar 点开始 -> AppModel::MakeTaskRequest（队列+设置快照）
  -> TaskController::Start -> ITaskRunner::Start（仅入队）
  -> worker 逐文件：ProbeForTask -> OutputNamer -> 目录试写 -> ffmpeg -> ProgressParser
  -> TaskEvent {FileStarted/FileProgress/FileSucceeded/FileFailed/TaskFinished/TaskCanceled}
  -> TaskController::Tick 消费 -> ActionBar/StatusBar 展示
  -> App 消费 ConsumeCompleted/ConsumeFailures -> Toast / 打开输出目录
```

取消：`TaskController::Cancel` → `ITaskRunner::Cancel` 置原子标志 → worker 内
`FfmpegProcess::Wait` 每 100ms 检查 → `TerminateProcess` + Job Object + 删除临时文件。

### 运行时依赖与日志

FFmpeg 查找顺序（命中后执行 `-version` 校验，损坏安装会跳过）：
`%EASYAV_FFMPEG_DIR%` → 系统 PATH → `<exeDir>/assets/ffmpeg{,/bin}` → `<exeDir>{,/bin}` → 当前工作目录。
`ffprobe` 优先与选中的 `ffmpeg` 同目录。仓库不含二进制，`assets/ffmpeg/README.txt` 说明放置方式；
构建时若该目录存在会拷贝到输出目录。

日志：`<exeDir>/logs/easyav.log`，线程安全、逐行 flush、超过 2MB 轮转为 `.1`；
记录命令参数、退出码、stderr 摘要（截断 500 字）与任务开始/结束统计。日志初始化失败静默降级。

### 已知限制

- 只有“编码转换”场景执行真实任务；剪辑/滤镜/录屏点开始会提示开发中。
- 输出编码只支持 H.264 / H.265（AV1 可作为输入，暂不作为输出）；GIF 未开放。
- 依赖 FFmpeg 构建包含 libx264 / libx265；缺少 libx265 时 H.265 会提示改用 H.264。
- 10-bit / HDR 源统一转为 yuv420p（与 V0.1 一致）；HEVC MP4 强制 `hvc1` 标记。
- 设置不持久化；处理中队列锁定（启动快照）。
- 探测在 worker 内最长 20 秒，取消需等探测返回；本地文件通常 <100ms。
- 显卡加速为“先试 NVENC 再回退”，没有启动预检测；QSV/AMF 未实机验证。
- 未知错误仍为通用中文文案；Win32/x64 共用输出目录。
- 仅实现 Windows 版本；Linux/macOS 需要补 services 的非 Windows 实现与构建配置。

## 其他注意点

- 字体优先加载 `assets/fonts/SourceHanSansCN-Regular.ttf`，不存在则回退 `msyh.ttc`/`simhei.ttf`/
  `Deng.ttf`，并使用 `GetGlyphRangesChineseFull()`（字体目录未纳入仓库）。
- 界面为**固定式单窗口布局**（使用 imgui docking 分支，但未启用多视口/停靠），窗口默认与最小均为
  1024×680（按 DPI 缩放），常量在 `main.cpp` 与 `App::Frame()` 中。
- `imgui.ini` 是运行时产物，已在 `.gitignore` 中；`Debug/`、`Release/` 同为构建产物。
- `mediaProbe/命名/进度/命令构造` 的单元测试不依赖 FFmpeg，可随时运行；真实处理验证需本机安装
  FFmpeg 或放入内置目录。
