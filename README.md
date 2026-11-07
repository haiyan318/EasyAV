# EasyAV · 极简音视频处理工具

面向“不懂视频参数”的普通用户：拖入文件、选一个直观目标、点开始，剩下的交给程序。
技术栈 C++17 + Dear ImGui（docking 分支）+ GLFW + OpenGL3，Windows x64。

当前基线：**V0.2 编码转换与界面焕新**（外部 FFmpeg 子进程）。

## 已实现功能

### 编码转换（真实处理）

- **解决播放器打不开**：把 HEVC、AV1、FLV、MPEG-4 等常见输入编码转成 **H.264**（最兼容）
  或 **H.265**（体积更小）。
- **输出容器**：MP4（推荐）或 MKV；H.265 + MP4 自动写入 `hvc1` 标记，苹果设备/Windows 照片可播。
- **只要声音**：任意带音轨的文件输出 MP3。
- **顺便瘦身**：转换面板内可选“顺便压缩体积”，用质量滑块控制大小；关闭时保持画质、只换编码。
- **显卡加速**：优先 NVENC，失败自动回退 CPU；QSV/AMF 参数已支持但未实机验证。
- **奇数分辨率兜底**：自动补偶数缩放，避免 yuv420p “not divisible by 2” 失败。

### 处理链路与体验

- **真实探测**：入队后读取时长、宽高、音视频编码，队列显示“格式 · 源编码 · 大小 · 时长”，悬停看分辨率。
- **真实进度**：解析 `-progress pipe:1`，显示当前文件、百分比、已用 / 预计剩余；处理中高亮当前文件并显示文件级进度。
- **取消**：立即终止子进程并回收进程树，删除临时文件（通常 <0.2 秒）。
- **批量执行**：队列逐文件串行，单个失败不中断整批；结束提示成功 / 失败数量。
- **防覆盖输出**：默认按结果自动命名（`_H264` / `_H265` / `_音频`），同名追加 `(1)(2)`；
  先写同目录临时文件，成功后原子重命名。
- **中文错误**：文件缺失 / 损坏 / 权限 / 磁盘满 / 编码器缺失 / 路径过长等可读提示；
  H.265 组件缺失时会引导改用 H.264。
- **日志**：`logs/easyav.log` 记录命令参数、退出码、stderr 摘要，超过 2MB 轮转。

### 界面

- 单窗口固定布局，浅色 / 深色主题；主面板 8px、控件 6px 圆角，1px 细边框与低饱和选中态，
  不使用重阴影；处理中当前文件高亮，进度条带轻量过渡。
- 首场景“编码转换”三步完成：选结果 → 是否瘦身 → 输出容器；高级设置默认折叠。
- “剪辑拼接 / 滤镜水印 / 录屏截图 / 批量处理”仍是界面演示，点开始会提示开发中。
- 处理中锁定队列（使用启动时快照），避免结果与界面不一致。

## 运行要求（FFmpeg）

程序不内嵌编解码库，通过子进程调用 `ffmpeg.exe` / `ffprobe.exe`。
启动时按以下顺序查找（命中后执行 `-version` 校验，损坏的安装会跳过）：

1. 环境变量 `EASYAV_FFMPEG_DIR`（目录，或直接指向 `ffmpeg.exe`）
2. 系统 `PATH`（优先使用用户已安装的 FFmpeg）
3. 程序目录 `assets/ffmpeg` 或 `assets/ffmpeg/bin`
4. 程序同目录或其 `bin`
5. 当前工作目录（从 IDE / 仓库根目录运行）

H.264 需要 `libx264`，H.265 需要 `libx265`；AV1/HEVC/FLV 输入需要对应解码器。
建议使用支持 Unicode 路径的 Windows 完整构建（MSVC 官方或 BtbN 构建）。
仓库不提交 FFmpeg 二进制，放置说明见 [assets/ffmpeg/README.txt](assets/ffmpeg/README.txt)。

## 快速开始

已打包的 Windows x64 版本（含 VC++ 运行库，不含 FFmpeg）：

    dist/EasyAV_V0.2_win64/EasyAV.exe

源码构建产物：

    Debug/EasyAV.exe
    Release/EasyAV.exe

## 构建

推荐 Visual Studio 2026（v145 工具集）：

    msbuild EasyAV.slnx /p:Configuration=Release /p:Platform=x64

或双击 `EasyAV.slnx`；4 个配置（Debug/Release × Win32/x64）均已配置。
也可使用 `build_win32.bat`（需先运行 vcvars，固定 32 位输出到 `Debug/`）。

ImGui/GLFW 从同级目录复用，请勿单独移动：

    ../imgui-docking
    ../imgui-docking/examples/libs/glfw/{include,lib-vc2010-64}

> 当前 services 仅实现 Windows（CreateProcessW / Job Object / Win32 路径），
> Linux/macOS 的 Makefile 目标暂不可用。

## 测试

先运行 vcvars32/64，使 `cl` 在 PATH 中：

    tests\run_tests.bat

当前 **68 个单元测试**，覆盖参数转义、ffprobe 解析、输出命名、命令构造（含 H.265/硬件/奇数尺寸）、
错误文案、进度解析、TaskController 事件消费、意图→参数映射；测试不依赖 ImGui，也不启动 FFmpeg。

## 目录结构

    main.cpp                窗口、ImGui/OpenGL 后端、拖拽回调
    src/App.*               应用外壳：装配模型 / 服务 / 面板
    src/core/               纯数据与工具（Types / Format / Text / TaskContracts / ITaskRunner）
    src/model/              AppModel / MediaQueue / TaskSettings / ConvertMapping / TaskController
    src/platform/           AppPaths / 原生文件对话框 / 打开资源管理器
    src/services/           FFmpeg 定位、参数构造、子进程、探测、进度、命名、错误、日志、任务服务
    src/ui/                 Theme / Widgets / Labels
    src/ui/panels/          12 个面板与弹窗（无状态 DrawXxx，含 ConvertPanel）
    tests/                  轻量断言测试 + run_tests.bat
    assets/ffmpeg/          内置 FFmpeg 放置目录（二进制不提交）
    dist/                   本地打包产物（不纳入 Git）

分层约束：core ← model ← ui；services 只依赖 core；App 负责装配。
面板只读写 AppModel，不互相引用，也不直接调用系统 API。

## 处理链路

    ActionBar 点击开始
      → AppModel::MakeTaskRequest（队列 + ConvertSettings 经 ConvertMapping 映射）
      → TaskController::Start → ITaskRunner::Start（仅入队）
      → worker：ffprobe → OutputNamer → 目录试写 → ffmpeg（H.264/H.265/MP3）→ ProgressParser
      → TaskEvent（进度 / 成功 / 失败 / 结束）→ TaskController::Tick
      → 界面更新 / 失败 Toast / 完成后打开输出目录

## 已知限制

- 输出编码只支持 H.264 / H.265；AV1 可作为输入，暂不作为输出；GIF 未开放。
- 依赖本机 FFmpeg 含 libx265；缺少时会提示改用 H.264。
- 10-bit / HDR 源统一转为 yuv420p（与 V0.1 一致）。
- 仅“编码转换”场景执行真实任务；剪辑 / 滤镜 / 录屏为界面演示。
- 设置持久化、真实视频预览、崩溃恢复未实现；处理中不能增删队列。
- 探测阶段取消需等 ffprobe 返回（本地文件通常 <100ms）。
- 显卡加速无启动预检测，无 NVIDIA 时会先失败一次再回退。
- 未知错误显示通用中文提示，详细原因见 `logs/easyav.log`。
- 仅 Windows 实现；Win32 与 x64 共用 `Debug/`、`Release/` 输出目录。
- 字体未随包分发，依赖系统字体。

## 文档

- [需求说明书](docs/EasyAV_SRS.md)
- [进度、待办与决策记录](docs/EasyAV_Progress_TODO.md)
- [开发约定（CODEBUDDY.md）](CODEBUDDY.md)
