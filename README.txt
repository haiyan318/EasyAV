EasyAV - 极简音视频处理工具
========================================

【本次交付范围】
根据 docs/EasyAV_SRS.md 完成 UI 界面搭建，包含完整的中文界面、场景化任务面板、
文件队列、进度反馈、设置弹窗与平台文件选择。
开发进度与下一阶段待办：docs/EasyAV_Progress_TODO.md
本版本不包含任何 FFmpeg / 编码 / 转码业务逻辑，点击“开始处理”只会演示进度。

【技术栈】
C++17 + Dear ImGui (docking 分支) + GLFW + OpenGL3。
工具集 v145，Windows SDK 10.0.26100.0，字符集 MultiByte，编译选项 /utf-8。

【打开与编译】
双击 EasyAV.slnx（或 EasyAV.sln），选择 Debug|x64 / Release|x64 直接生成。
Win32 与 x64、Debug 与 Release 共 4 个配置均已配置。
生成的 exe 位于 Debug\ 或 Release\ 目录，运行时不弹出控制台窗口。
新增依赖：comdlg32.lib、shell32.lib、ole32.lib（文件对话框与打开输出目录）。

【目录与依赖】
本项目与 imgui-docking 同级，ImGui/GLFW 通过相对路径复用：
  头文件包含：src、..\imgui-docking、..\imgui-docking\backends、
              ..\imgui-docking\examples\libs\glfw\include
  库目录：    ..\imgui-docking\examples\libs\glfw\lib-vc2010-32  (Win32)
              ..\imgui-docking\examples\libs\glfw\lib-vc2010-64  (x64)
请勿单独移动 imgui-docking 目录。

【源代码结构（高内聚 / 低耦合）】
  main.cpp                仅窗口、OpenGL/ImGui 后端、事件转发（拖拽回调）
  src/App.h/.cpp          应用外壳：装配模型、面板、平台能力；不依赖 GLFW
  src/core/               纯数据与工具（Types / Format / Text），不引用 ImGui
  src/model/              队列、场景参数、任务状态（MediaQueue / TaskSettings /
                          TaskController / AppModel），不绘制界面
  src/platform/           平台能力（打开文件/文件夹对话框、打开资源管理器）
  src/ui/                 主题、字号、通用控件（Theme / Widgets / Labels）
  src/ui/panels/          各个独立面板：顶栏、队列、场景标签、五个场景面板、
                          底部操作栏、状态栏、设置弹窗、Toast
面板之间不互相引用；只读写 AppModel，UI 通过 request 标志请求平台操作，
由 App 统一执行，便于后续替换实现或增加新的处理引擎。

【已实现的界面功能（对应 SRS）】
1. 顶栏：品牌、添加文件、添加文件夹、设置。
2. 文件队列：拖拽文件/文件夹入队；显示文件名、格式、大小、时长状态；
   支持单个移除、清空；空队列时显示拖拽引导与选择文件按钮。
3. 视频瘦身：体积优先 <-> 画质优先滑块、目标格式（MP4/MKV/GIF/MP3）、
   显卡加速开关、折叠的高级设置、预计输出体积。
4. 剪辑拼接：双滑块时间轴（未探测到时长时按百分比）、整段/去头/去尾快捷操作、
   多文件合并、提取音频、静音视频。
5. 滤镜水印：旋转、裁剪比例（原始/1:1/16:9/9:16）、文字/图片水印、
   透明度与大小调节、预览区可直接拖动水印位置。
6. 录屏截图：屏幕/摄像头大按钮、录制计时、画质选择、定时截图与立即截图。
7. 批量处理：规则汇总、队列预览、“应用到全部文件”。
8. 进度与反馈：全局进度条、当前文件、已用/预计剩余时间、取消按钮；
   所有提示为口语化中文，不出现命令行参数。
9. 防呆：队列为空时“开始处理”禁用；输出默认不覆盖原文件（自动加后缀）；
   设置中可修改输出目录、后缀、完成后打开文件夹、深色界面。

【字体与 DPI】
优先加载 assets/fonts/SourceHanSansCN-Regular.ttf；
若不存在则回退到 Windows 自带字体（msyh.ttc / simhei.ttf / Deng.ttf）。
通过 io.Fonts->GetGlyphRangesChineseFull() 覆盖中文。
布局尺寸通过 ui::S() 跟随 style.FontScaleDpi 缩放，125% / 150% DPI 下比例一致。

【演示模式说明】
TaskController 当前在 UI 线程按时间推进进度（每个文件约 2.2 秒），
用于验证进度条、取消、完成提示等交互；它不做任何文件读写或编码。
MediaQueue 只读取文件系统信息（名称、扩展名、大小），不解析媒体元数据，
因此队列中的“时长”显示为“待分析”。

【后续接入 FFmpeg 的建议位置】
- 媒体探测：填充 MediaItem::durationSec（可在 model/MediaQueue 增加探测回调）。
- 任务执行：替换 src/model/TaskController.cpp 的内部实现，改为后台线程/子进程，
  对外接口（Start/Cancel/Tick/Progress/StatusText）保持不变。
- 命令生成：新增独立模块，禁止在 UI 面板中拼接命令；路径参数必须转义。
- 录制/截图：在 src/platform 下新增实现，通过 Work 线程回传状态。
- 错误汉化：把失败原因写入 TaskController::StatusText 与 Toast。

【文件清单（新增部分）】
  src/App.h/.cpp
  src/core/Types.h、Format.h/.cpp、Text.h/.cpp
  src/model/MediaQueue.h/.cpp、TaskSettings.h、TaskController.h/.cpp、AppModel.h/.cpp
  src/platform/FileDialogs.h/.cpp
  src/ui/Theme.h/.cpp、Widgets.h/.cpp、Labels.h
  src/ui/panels/*.h/.cpp（12 个面板/弹窗）
  EasyAV.vcxproj / .filters 已加入以上文件与依赖库。
